#include "MainWnd.h"
#include "ui_MainWnd.h"

#include <QMenu>

#include <QString>
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

MainWnd::MainWnd()
    : QMainWindow(nullptr)
    , ui(new Ui::MainWnd)
    , m_networkManager(nullptr)
    , m_activeRequest(nullptr)
{
    ui->setupUi(this);

    setupTabViews();
    setupViewAsActions();
    setupVerbs();
    connectSignals();
}

MainWnd::~MainWnd()
{
    delete m_activeRequest;
    delete m_networkManager;

    delete ui;
}

void MainWnd::setupTabViews()
{
    ui->tabContainer->setCurrentIndex(0);
    ui->tabRequestDetailsContainer->setCurrentIndex(0);
    ui->tabContainer->removeTab(1);
}

void MainWnd::setupViewAsActions()
{
    QMenu* menu = new QMenu(ui->btnViewContentAs);
    menu->addAction("Json");
    menu->addAction("Xml");
    menu->addAction("Html");
    ui->btnViewContentAs->setMenu(menu);
    ui->btnViewContentAs->setPopupMode(QToolButton::InstantPopup);
}

void MainWnd::setupVerbs()
{
    ui->drpVerb->addItems(QStringList() << "GET" << "POST" << "PUT" << "DELETE"
                          << "COPY" << "HEAD" << "MKCOL" << "MOVE"
                          << "OPTIONS" << "PROPFIND" << "TRACE");
}

void MainWnd::connectSignals()
{
    connect(ui->btnSendRequest, SIGNAL(pressed()), this, SLOT(onSendRequestButtonClicked()));
}

void MainWnd::onSendRequestButtonClicked()
{
    ui->btnSendRequest->setEnabled(false);

    QUrl url = QUrl::fromUserInput(getEnteredUrl());
    if (!url.isValid())
    {
        // TODO: Add message box etc.
        qDebug() << "WARNING : Invalid URL!";
        return;
    }

    QString verb = "GET";

    ui->lblResponseUrl->setText(verb + " on " + url.toString());
    doHttpRequest(url, verb, "");
}

void MainWnd::onHttpRequestFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 301 || 302)
    {
        QUrl newUrl = QUrl::fromUserInput(reply->header(QNetworkRequest::LocationHeader).toString());
        if (newUrl.isValid())
        {
            QString verb = m_activeRequest->verb;
            QString content = m_activeRequest->content;

            delete m_activeRequest;
            m_activeRequest = nullptr;

            doHttpRequest(newUrl, verb, content);
            return;
        }
    }

    QString msg = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if (msg.isEmpty())
        msg = reply->errorString();

    ui->lblHttpStatus->setText(QString::number(statusCode) + " " + msg);

    ui->txtHeaders->clear();
    for (const auto& h : reply->rawHeaderPairs())
        ui->txtHeaders->append(h.first + ": " + h.second);

    ui->txtResponseData->setPlainText(reply->readAll());

    if (ui->tabContainer->count() < 2)
        ui->tabContainer->insertTab(1, ui->tabResponse, tr("Response"));

    ui->tabContainer->setCurrentIndex(1);
    ui->btnSendRequest->setEnabled(true);

    delete m_activeRequest;
    m_activeRequest = nullptr;
}

void MainWnd::onHttpRequestError(QNetworkReply::NetworkError error)
{
    qDebug() << "HTTP Error: " << error;
    emit m_networkManager->finished(m_activeRequest->request);
}

QString MainWnd::getEnteredUrl() const
{
    return ui->txtUrl->text().trimmed();
    emit m_activeRequest->request->finished();
}

void MainWnd::doHttpRequest(QUrl url, QString verb, QString content)
{
    if (m_activeRequest)
        return;

    m_activeRequest = new RequestInfo(url, verb, content);

    if (!m_networkManager)
    {
        m_networkManager = new QNetworkAccessManager(this);
        connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onHttpRequestFinished(QNetworkReply*)));
    }

    m_activeRequest->request = m_networkManager->sendCustomRequest(QNetworkRequest(url), "GET", nullptr);
    m_activeRequest->request->ignoreSslErrors();
    connect(m_activeRequest->request, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onHttpRequestError(QNetworkReply::NetworkError)));
}








