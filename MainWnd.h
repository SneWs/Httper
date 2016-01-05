#ifndef MAINWND_H
#define MAINWND_H

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
class QNetworkCookieJar;
class QBuffer;

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

class ComboBoxFocusManager : public QObject
{
    Q_OBJECT
public:
    explicit ComboBoxFocusManager(QObject* parent)
        : QObject(parent)
    {
        if (parent)
            parent->installEventFilter(this);
    }

    virtual bool eventFilter(QObject* /*obj*/, QEvent* ev) override
    {
        if (ev->type() == QEvent::FocusIn)
            emit gainedFocus();
        else if (ev->type() == QEvent::FocusOut)
            emit lostFocus();

        return false;
    }

signals:
    void gainedFocus();
    void lostFocus();
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
    void onVerbDropdownLostFocus();
    void onContentTypeDropdownLostFocus();
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
    void setupContentTypes();
    void connectSignals();
    QString getEnteredUrl() const;
    QString getEnteredVerb() const;
    QString getEnteredContentType() const;
    Headers getDefinedHeaders() const;

    void doHttpRequest(QUrl url, QString verb, QString contentType, QString content, Headers headers);

private:
    Ui::MainWnd* ui;

    QNetworkAccessManager* m_networkManager;
    QNetworkCookieJar* m_cookieJar;
    RequestInfo* m_activeRequest;

    std::vector<MWidget*> m_openWebViews;
};

#endif // MAINWND_H
