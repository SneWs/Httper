#include "CookieJar.h"

#include <QNetworkCookie>

CookieJar::CookieJar()
{

}

void CookieJar::removeAll()
{
    auto cookies = allCookies();
    for (const auto& cookie : cookies)
        deleteCookie(cookie);
}

CookieValues CookieJar::getAllCookies() const
{
    CookieValues retVal;

    auto cookies = allCookies();
    for (const auto& cookie : cookies)
        retVal[std::string(cookie.name().constData(), cookie.name().length())] = std::string(cookie.value().constData(), cookie.value().length());

    return retVal;
}
