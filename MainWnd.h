#ifndef MAINWND_H
#define MAINWND_H

#include "Settings.h"
#include "SettingsManager.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <QObject>
#include <QEvent>
#include <QMainWindow>
#include <QAction>
#include <QNetworkReply>
#include <QBuffer>

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

    std::unique_ptr<QNetworkReply> request;
    std::unique_ptr<QBuffer> sendBuffer;
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

    void loadSettings();
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
    void onShowAboutDialog();

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
    std::unique_ptr<Ui::MainWnd> ui;
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    std::unique_ptr<CookieJar> m_cookieJar;
    std::unique_ptr<RequestInfo> m_activeRequest;

    Settings m_settings;
    std::vector<std::unique_ptr<MWidget>> m_openWebViews;
};

#endif // MAINWND_H
