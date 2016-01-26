#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <string>
#include <vector>
#include <map>

#include <QNetworkCookieJar>

typedef std::map<std::string, std::string> CookieValues;

class CookieJar : public QNetworkCookieJar
{
public:
    CookieJar();

    void removeAll();
    CookieValues getAllCookies() const;
};

#endif // COOKIEJAR_H
