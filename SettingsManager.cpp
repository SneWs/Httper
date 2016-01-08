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
    Settings settings;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(readJsonFile().toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        return settings;

    QJsonObject httperObj = doc.object()["httper"].toObject();
    if (httperObj.isEmpty())
        return settings;

    if (httperObj["verbs"].isArray())
    {
        for (const auto& elem : httperObj["verbs"].toArray())
            if (elem.isString())
                settings.m_verbs.emplace_back(elem.toString().toStdString());
    }

    if (httperObj["contentTypes"].isArray())
    {
        for (const auto& elem : httperObj["contentTypes"].toArray())
            if (elem.isString())
                settings.m_contentTypes.emplace_back(elem.toString().toStdString());
    }

    if (httperObj["followRedirects"].isBool())
        settings.m_followRedirects = httperObj["followRedirects"].toBool();

    if (httperObj["lastUsedUrl"].isString())
        settings.m_lastUsedUrl = httperObj["lastUsedUrl"].toString().toStdString();

    if (httperObj["window"].isObject())
    {
        QJsonObject window = httperObj["window"].toObject();
        if (window["geometry"].isArray())
        {
            QJsonArray geometry = window["geometry"].toArray();
            settings.m_x = geometry[0].toInt();
            settings.m_y = geometry[1].toInt();
            settings.m_w = geometry[2].toInt();
            settings.m_h = geometry[3].toInt();
        }
    }

    return std::move(settings);
}

void SettingsManager::writeSettings(const Settings& settings)
{
    QJsonArray verbs;
    for (const auto& str : settings.m_verbs)
        verbs.append(str.c_str());

    QJsonArray contentTypes;
    for (const auto& str : settings.m_contentTypes)
        contentTypes.append(str.c_str());

    // Window settings
    QJsonArray geometry;
    geometry.append(settings.m_x);
    geometry.append(settings.m_y);
    geometry.append(settings.m_w);
    geometry.append(settings.m_h);

    QJsonObject window;
    window.insert("geometry", geometry);

    QJsonObject httper;
    httper.insert("followRedirects", settings.m_followRedirects);
    httper.insert("lastUsedUrl", settings.m_lastUsedUrl.c_str());
    httper.insert("verbs", verbs);
    httper.insert("contentTypes", contentTypes);
    httper.insert("window", window);

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
