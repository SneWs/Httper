#include "MainWnd.h"
#include "ui_MainWnd.h"
#include "MWidget.h"

#include <QMenu>
#include <QMessageBox>
#include <QWebView>

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
    , m_openWebViews()
{
    ui->setupUi(this);

    setupTabViews();
    setupViewAsActions();
    setupVerbs();
    connectSignals();
}

MainWnd::~MainWnd()
{
    for (auto* view : m_openWebViews)
        delete view;
    m_openWebViews.clear();

    delete m_activeRequest;
    delete m_networkManager;

    delete ui;
}

void MainWnd::closeEvent(QCloseEvent *)
{
    for (auto* view : m_openWebViews)
        view->close();
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

    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(onViewContentAsActionClicked(QAction*)));
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
    QUrl url = QUrl::fromUserInput(getEnteredUrl());
    if (!url.isValid() || url.host().isEmpty())
    {
        // TODO: Add message box etc.
        qDebug() << "WARNING : Invalid URL!";

        QMessageBox msg;
        msg.setText("Invalid URL");
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }

    ui->btnSendRequest->setEnabled(false);
    QString verb = getEnteredVerb();

    ui->lblResponseUrl->setText(verb + " on " + url.toString());
    doHttpRequest(url, verb, "");
}

void MainWnd::onViewContentAsActionClicked(QAction* ac)
{
    if (ac->text().compare("Html", Qt::CaseInsensitive) == 0)
    {
        MWidget* view = new MWidget();

        QWebView* webView = new QWebView(view);
        webView->setHtml(m_activeRequest->content.toUtf8(), m_activeRequest->url);
        webView->reload();

        connect(view, SIGNAL(onWidgetClosed(MWidget*)), this, SLOT(onWebViewClosed(MWidget*)));
        connect(webView, SIGNAL(titleChanged(QString)), view, SLOT(setWindowTitle(QString)));

        m_openWebViews.emplace_back(view);
        view->setLayout(new QGridLayout(view));
        view->layout()->addWidget(webView);
        view->setGeometry(100, 100, 800, 600);
        view->showNormal();
    }
}

void MainWnd::onWebViewClosed(MWidget* widget)
{
    m_openWebViews.erase(std::remove(m_openWebViews.begin(), m_openWebViews.end(), widget), m_openWebViews.end());
    delete widget;
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
}

void MainWnd::onHttpRequestError(QNetworkReply::NetworkError error)
{
    qDebug() << "HTTP Error: " << error;
    emit m_networkManager->finished(m_activeRequest->request);
}

QString MainWnd::getEnteredUrl() const
{
    return ui->txtUrl->text().trimmed();
}

QString MainWnd::getEnteredVerb() const
{
    return ui->drpVerb->currentText();
}

void MainWnd::doHttpRequest(QUrl url, QString verb, QString content)
{
    if (m_activeRequest)
    {
        if (!m_activeRequest->request->isFinished())
            m_activeRequest->request->abort();

        delete m_activeRequest;
        m_activeRequest = nullptr;
    }

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









