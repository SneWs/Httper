#ifndef MAINWND_H
#define MAINWND_H

#include "Settings.h"
#include "SettingsManager.h"

#include <QObject>
#include <QEvent>
#include <QMainWindow>
#include <QAction>
#include <QNetworkReply>
#include <QBuffer>

#include <string>
#include <vector>
#include <map>

namespace Ui
{
class MainWnd;
}

class MWidget;
class QWebView;
class QTableWidgetItem;
class QNetworkAccessManager;
class QBuffer;

class CookieJar;

typedef std::map<std::string, std::string> Headers;

class RequestInfo
{
public:
    RequestInfo(QUrl url_, QString verb_, QString contentType_, QString content_)
        : request(nullptr)
        , sendBuffer(nullptr)
        , headers()
        , url(url_)
        , verb(verb_)
        , contentType(contentType_)
        , content(content_)
    { }

    ~RequestInfo()
    {
        delete sendBuffer;
        sendBuffer = nullptr;

        delete request;
        request = nullptr;
    }

    QNetworkReply* request;
    QBuffer* sendBuffer;
    Headers headers;
    QUrl url;
    QString verb;
    QString contentType;
    QString content;
};

class MainWnd : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWnd(Settings settings);
    ~MainWnd();

    void saveSettings();

protected:
    virtual void closeEvent(QCloseEvent*) override;

protected slots:
    void onSendRequestButtonClicked();
    void onVerbDropdownLostFocus();
    void onContentTypeDropdownLostFocus();
    void onAddHeadersKeyValuePairButtonClicked();
    void onViewContentAsActionClicked(QAction*);
    void onWebViewClosed(MWidget* widget);
    void onCloseAllAdditionalWindowsMenuItemClicked();
    void onCurrentHeaderItemSelectionChanged(QTableWidgetItem*, QTableWidgetItem*);
    void onHeadersRemoveSelectedButtonClicked();

    // Menu item actions
    void onClearAllCookies();
    void onToggleFollowRedirects(bool checked);

    void onHttpRequestFinished(QNetworkReply*);
    void onHttpRequestError(QNetworkReply::NetworkError);

private:
    void setupTabViews();
    void setupViewAsActions();
    void setupVerbs();
    void setupContentTypes();
    void connectSignals();
    void onAdditionalWindowPostAction();
    void removeAdditionalWindows();

    QString getEnteredUrl() const;
    QString getEnteredVerb() const;
    QString getEnteredContentType() const;
    Headers getDefinedHeaders() const;

    void doHttpRequest(QUrl url, QString verb, QString contentType, QString content, Headers headers);

private:
    Ui::MainWnd* ui;

    QNetworkAccessManager* m_networkManager;
    CookieJar* m_cookieJar;
    RequestInfo* m_activeRequest;

    Settings m_settings;
    std::vector<MWidget*> m_openWebViews;
};

#endif // MAINWND_H
