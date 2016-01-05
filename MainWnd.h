#ifndef MAINWND_H
#define MAINWND_H

#include <QMainWindow>
#include <QNetworkReply>

namespace Ui
{
class MainWnd;
}

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

protected slots:
    void onSendRequestButtonClicked();

    void onHttpRequestFinished(QNetworkReply*);
    void onHttpRequestError(QNetworkReply::NetworkError);

private:
    void setupTabViews();
    void setupViewAsActions();
    void setupVerbs();
    void connectSignals();
    QString getEnteredUrl() const;

    void doHttpRequest(QUrl url, QString verb, QString content);

private:
    Ui::MainWnd* ui;

    QNetworkAccessManager* m_networkManager;
    RequestInfo* m_activeRequest;
};

#endif // MAINWND_H
