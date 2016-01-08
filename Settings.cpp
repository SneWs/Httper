#include "Settings.h"
#include "SettingsManager.h"

Settings::Settings()
    : m_followRedirects(true)
    , m_verbs()
    , m_contentTypes()
    , m_lastUsedUrl("")
    , m_x(100)
    , m_y(100)
    , m_w(0)
    , m_h(0)
{ }

