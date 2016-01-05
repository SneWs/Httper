#ifndef MAINWND_H
#define MAINWND_H

#include <QMainWindow>
#include <QAction>
#include <QNetworkReply>

#include <vector>

namespace Ui
{
class MainWnd;
}

class MWidget;
class QWebView;
class QNetworkAccessManager;

class RequestInfo
{
public:
    RequestInfo(QUrl url_, QString verb_, QString content_)
        : request(nullptr)
        , url(url_)
        , verb(verb_)
        , content(content_)
    { }

    ~RequestInfo()
    {
        delete request;
        request = nullptr;
    }

    QNetworkReply* request;
    QUrl url;
    QString verb;
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
    void onViewContentAsActionClicked(QAction*);
    void onWebViewClosed(MWidget* widget);

    void onHttpRequestFinished(QNetworkReply*);
    void onHttpRequestError(QNetworkReply::NetworkError);

private:
    void setupTabViews();
    void setupViewAsActions();
    void setupVerbs();
    void connectSignals();
    QString getEnteredUrl() const;
    QString getEnteredVerb() const;

    void doHttpRequest(QUrl url, QString verb, QString content);

private:
    Ui::MainWnd* ui;

    QNetworkAccessManager* m_networkManager;
    RequestInfo* m_activeRequest;

    std::vector<MWidget*> m_openWebViews;
};

#endif // MAINWND_H
