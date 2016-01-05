#ifndef MAINWND_H
#define MAINWND_H

#include <QMainWindow>
#include <QAction>
#include <QNetworkReply>
#include <QBuffer>

#include <vector>

namespace Ui
{
class MainWnd;
}

class MWidget;
class QWebView;
class QTableWidgetItem;
class QNetworkAccessManager;
class QNetworkCookieJar;
class QBuffer;

class RequestInfo
{
public:
    RequestInfo(QUrl url_, QString verb_, QString contentType_, QString content_)
        : request(nullptr)
        , sendBuffer(nullptr)
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
    QUrl url;
    QString verb;
    QString contentType;
    QString content;
};

class MainWnd : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWnd();
    ~MainWnd();

protected:
    virtual void closeEvent(QCloseEvent*) override;

protected slots:
    void onSendRequestButtonClicked();
    void onAddHeadersKeyValuePairButtonClicked();
    void onViewContentAsActionClicked(QAction*);
    void onWebViewClosed(MWidget* widget);
    void onCurrentHeaderItemSelectionChanged(QTableWidgetItem*, QTableWidgetItem*);
    void onHeadersRemoveSelectedButtonClicked();

    void onHttpRequestFinished(QNetworkReply*);
    void onHttpRequestError(QNetworkReply::NetworkError);

private:
    void setupTabViews();
    void setupViewAsActions();
    void setupVerbs();
    void connectSignals();
    QString getEnteredUrl() const;
    QString getEnteredVerb() const;
    QString getEnteredContentType() const;

    void doHttpRequest(QUrl url, QString verb, QString contentType, QString content);

private:
    Ui::MainWnd* ui;

    QNetworkAccessManager* m_networkManager;
    QNetworkCookieJar* m_cookieJar;
    RequestInfo* m_activeRequest;

    std::vector<MWidget*> m_openWebViews;
};

#endif // MAINWND_H
