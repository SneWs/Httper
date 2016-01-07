#include "SettingsManager.h"
#include "Settings.h"

#include <QDebug>
#include <QString>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

namespace
{
    static QString SettingsFilename = "se.grenangen.httper.json";
}

Settings SettingsManager::loadSettings()
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(readJsonFile().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        return Settings();

    QJsonObject httperObj = doc.object()["httper"].toObject();
    if (httperObj.isEmpty())
        return Settings();

    bool followRedirects = true;
    std::vector<std::string> verbs;
    std::vector<std::string> contentTypes;

    if (httperObj["followRedirects"].isBool())
        followRedirects = httperObj["followRedirects"].toBool();

    if (httperObj["verbs"].isArray())
    {
        for (const auto& elem : httperObj["verbs"].toArray())
            if (elem.isString())
                verbs.emplace_back(elem.toString().toStdString());
    }

    if (httperObj["contentTypes"].isArray())
    {
        for (const auto& elem : httperObj["contentTypes"].toArray())
            if (elem.isString())
                contentTypes.emplace_back(elem.toString().toStdString());
    }

    return std::move(Settings(followRedirects, std::move(verbs), std::move(contentTypes)));
}

void SettingsManager::writeSettings(const Settings& settings)
{
    QJsonArray verbs;
    for (const auto& str : settings.m_verbs)
        verbs.append(str.c_str());

    QJsonArray contentTypes;
    for (const auto& str : settings.m_contentTypes)
        contentTypes.append(str.c_str());

    QJsonObject httper;
    httper.insert("followRedirects", settings.m_autoFollowRedirects);
    httper.insert("verbs", verbs);
    httper.insert("contentTypes", contentTypes);

    QJsonObject jObj;
    jObj.insert("httper", httper);

    QJsonDocument doc(jObj);

    QString configLocation(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Httper");
    QDir configDir(configLocation);
    if (!configDir.exists())
    {
        QDir dir;
        dir.mkpath(configDir.path());
    }

    auto path = configDir.filePath(SettingsFilename);
    QFile settingsFile(path);
    if (!settingsFile.open(QFile::WriteOnly | QFile::Text))
        return;

    settingsFile.write(doc.toJson());
}

QString SettingsManager::readJsonFile()
{
    QString configLocation(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/Httper");
    qDebug() << "Going to try to read settings from path " << configLocation;
    QDir configDir(configLocation);

    if (!configDir.exists())
    {
        QDir dir;
        dir.mkpath(configDir.path());
    }

    QString jsonSettingsString = "{}";

    auto path = configDir.filePath(SettingsFilename);
    QFile settingsFile(path);
    if (settingsFile.exists() && settingsFile.open(QFile::ReadOnly | QFile::Text))
        jsonSettingsString = settingsFile.readAll();
    else
    {
        jsonSettingsString = readDefaultsJsonFile();

        // Write default settings to disk.
        if (settingsFile.open(QFile::WriteOnly | QFile::Text))
            settingsFile.write(jsonSettingsString.toUtf8());
    }

    return jsonSettingsString;
}

QString SettingsManager::readDefaultsJsonFile()
{
    QFile file(":/settings/defaults.json");
    if (!file.exists())
        return "";

    if (!file.open(QFile::ReadOnly | QFile::Text))
        return "";

    QString jsonString = file.readAll();
    file.close();

    return jsonString;
}
