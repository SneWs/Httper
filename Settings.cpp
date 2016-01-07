#include "Settings.h"
#include "SettingsManager.h"

Settings::Settings()
    : Settings(true, std::move(std::vector<std::string>()), std::move(std::vector<std::string>()))
{ }

Settings::Settings(bool followRedirects, std::vector<std::string>&& verbs, std::vector<std::string>&& contentTypes)
    : m_autoFollowRedirects(followRedirects)
    , m_verbs(verbs)
    , m_contentTypes(contentTypes)
{ }