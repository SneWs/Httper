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

    void setWindowGeometry(int x, int y, int w, int h) { m_x = x; m_y = y; m_w = w; m_h = h; }
    bool hasWindowSettings() const { return m_w > 0 && m_h > 0; }

    int getWindowX() const { return m_x; }
    int getWindowY() const { return m_y; }
    int getWindowHeight() const { return m_h; }
    int getWindowWidth() const { return m_w; }

private:
    bool m_autoFollowRedirects; // Defaults to true
    std::vector<std::string> m_verbs; // Accumulated verbs over time
    std::vector<std::string> m_contentTypes; // Accumulated content types over time

    // Window settings
    int m_x,
        m_y,
        m_w,
        m_h;

    friend class SettingsManager;
};

#endif // SETTINGS_H_
