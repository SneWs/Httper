#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

class QString;

class Settings;

class SettingsManager
{
public:
    static Settings loadSettings();
    static void writeSettings(const Settings& settings);

private:
    SettingsManager();

    static QString readJsonFile();
    static QString readDefaultsJsonFile();

};

#endif // SETTINGSMANAGER_H
