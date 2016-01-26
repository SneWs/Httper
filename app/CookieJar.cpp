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
        retVal[cookie.name().toStdString()] = cookie.value().toStdString();

    return retVal;
}
