#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string>
#include <vector>

class SettingsManager;

class Settings
{
public:
    Settings();
    Settings(bool followRedirects, std::vector<std::string>&& verbs,
        std::vector<std::string>&& contentTypes);

    void setFollowRedirects(bool follow) { m_autoFollowRedirects = follow; }

    bool followRedirects() const { return m_autoFollowRedirects; }
    std::vector<std::string>& verbs() { return m_verbs; }
    std::vector<std::string>& contentTypes() { return m_contentTypes; }

private:
    bool m_autoFollowRedirects; // Defaults to true
    std::vector<std::string> m_verbs; // Accumulated verbs over time
    std::vector<std::string> m_contentTypes; // Accumulated content types over time

    friend class SettingsManager;
};

#endif // SETTINGS_H_
