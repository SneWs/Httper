#include "MainWnd.h"
#include "ui_MainWnd.h"
#include "MWidget.h"

#include <string>

#include <QMenu>
#include <QMessageBox>
#include <QWebView>

#include <QString>
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>
#include <QNetworkCookieJar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MainWnd::MainWnd()
    : QMainWindow(nullptr)
    , ui(new Ui::MainWnd)
    , m_networkManager(nullptr)
    , m_cookieJar(new QNetworkCookieJar(this))
    , m_activeRequest(nullptr)
    , m_openWebViews()
{
    ui->setupUi(this);

    setupTabViews();
    setupViewAsActions();
    setupVerbs();
    connectSignals();

    ui->statusBar->showMessage(tr("Welcome to Httper"), 5000);
}

MainWnd::~MainWnd()
{
    for (auto* view : m_openWebViews)
        delete view;
    m_openWebViews.clear();

    delete m_cookieJar;
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
    connect(ui->btnHeaderAddKeyValue, SIGNAL(pressed()), this, SLOT(onAddHeadersKeyValuePairButtonClicked()));

    // Headers table view
    connect(ui->tblHeaders, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
        this, SLOT(onCurrentHeaderItemSelectionChanged(QTableWidgetItem*, QTableWidgetItem*)));

    // Headers remove item button
    connect(ui->btnHeadersRemoveSelected, SIGNAL(pressed()), this, SLOT(onHeadersRemoveSelectedButtonClicked()));
}

void MainWnd::onSendRequestButtonClicked()
{
    QUrl url = QUrl::fromUserInput(getEnteredUrl());
    if (!url.isValid() || url.host().isEmpty())
    {
        QMessageBox msg;
        msg.setText("Invalid URL");
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }

    ui->statusBar->showMessage(tr("Sending Request..."), 0);
    ui->btnSendRequest->setEnabled(false);
    QString verb = getEnteredVerb();
    QString contentType = getEnteredContentType();
    QString contentToSend = ui->txtContentToSend->toPlainText().trimmed();

    ui->lblResponseUrl->setText(verb + " on " + url.toString());
    doHttpRequest(url, verb, contentType, contentToSend);
}

void MainWnd::onAddHeadersKeyValuePairButtonClicked()
{
    QString key = ui->txtHeaderKey->text().trimmed();
    QString value = ui->txtHeaderValue->text().trimmed();

    if (key.isEmpty())
        return;

    int newRowIndex = ui->tblHeaders->rowCount();

    // Do we have a row with this key already?
    auto hits = ui->tblHeaders->findItems(key, Qt::MatchFixedString);
    if (hits.count() > 0)
        newRowIndex = hits[0]->row();
    else
        ui->tblHeaders->insertRow(newRowIndex);

    QTableWidgetItem* newKeyItem = new QTableWidgetItem(key);
    QTableWidgetItem* newValueItem = new QTableWidgetItem(value);
    ui->tblHeaders->setItem(newRowIndex, 0, newKeyItem);
    ui->tblHeaders->setItem(newRowIndex, 1, newValueItem);
}

void MainWnd::onViewContentAsActionClicked(QAction* ac)
{
    MWidget* view = new MWidget();

    QWebView* webView = new QWebView(view);

    connect(view, SIGNAL(onWidgetClosed(MWidget*)), this, SLOT(onWebViewClosed(MWidget*)));
    connect(webView, SIGNAL(titleChanged(QString)), view, SLOT(setWindowTitle(QString)));

    m_openWebViews.emplace_back(view);

    if (ac->text().compare("Html", Qt::CaseInsensitive) == 0)
        webView->setHtml(m_activeRequest->content.toUtf8(), m_activeRequest->url);
    else if (ac->text().compare("Json", Qt::CaseInsensitive) == 0)
    {
        QJsonDocument doc = QJsonDocument::fromJson(m_activeRequest->content.toUtf8());

        QString str = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
        qDebug() << "JSON: " << str;
        webView->settings()->setDefaultTextEncoding("utf-8");
        webView->setContent(str.toUtf8(), QString("application/json"), m_activeRequest->url);
    }
    else if (ac->text().compare("Xml", Qt::CaseInsensitive) == 0)
        webView->setContent(m_activeRequest->content.toUtf8(), QString("application/xml"), m_activeRequest->url);

    view->setFocusPolicy(Qt::StrongFocus);
    view->setLayout(new QGridLayout(view));
    view->layout()->setMargin(0);
    view->layout()->addWidget(webView);
    view->setGeometry(100, 100, 800, 600);
    view->showNormal();
    view->setFocus();
}

void MainWnd::onWebViewClosed(MWidget* widget)
{
    m_openWebViews.erase(std::remove(m_openWebViews.begin(), m_openWebViews.end(), widget), m_openWebViews.end());
    delete widget;
}

void MainWnd::onCurrentHeaderItemSelectionChanged(QTableWidgetItem* newItem, QTableWidgetItem* /*prevItem*/)
{
    ui->btnHeadersRemoveSelected->setEnabled(newItem != nullptr);
}

void MainWnd::onHeadersRemoveSelectedButtonClicked()
{
    auto* item = ui->tblHeaders->currentItem();
    if (item != nullptr)
        ui->tblHeaders->removeRow(item->row());
}

void MainWnd::onHttpRequestFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 301 || statusCode == 302)
    {
        QString locationString = reply->rawHeader("Location");
        if (!locationString.isEmpty())
        {
            QUrl newUrl = QUrl::fromUserInput(locationString);
            if (!newUrl.isValid() || newUrl.host().isEmpty())
            {
                // Most likely a relative URL
                newUrl = m_activeRequest->url;
                newUrl.setPath(locationString);

                if (!newUrl.isValid())
                    return;
            }

            QString verb = m_activeRequest->verb;
            QString content = m_activeRequest->content;
            QString contentType = m_activeRequest->contentType;

            qDebug() << "Redirecting to " << newUrl.toString();

            doHttpRequest(newUrl, verb, contentType, content);
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

    m_activeRequest->content = reply->readAll();
    ui->txtResponseData->setPlainText(m_activeRequest->content);

    if (ui->tabContainer->count() < 2)
        ui->tabContainer->insertTab(1, ui->tabResponse, tr("Response"));

    ui->tabContainer->setCurrentIndex(1);
    ui->btnSendRequest->setEnabled(true);
    ui->statusBar->showMessage(tr("Response Received"), 5000);
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

QString MainWnd::getEnteredContentType() const
{
    return ui->drpContentType->currentText().trimmed();
}

void MainWnd::doHttpRequest(QUrl url, QString verb, QString contentType, QString content)
{
    if (m_activeRequest)
    {
        if (!m_activeRequest->request->isFinished())
            m_activeRequest->request->abort();

        delete m_activeRequest;
        m_activeRequest = nullptr;
    }

    m_activeRequest = new RequestInfo(url, verb, contentType, content);

    if (!m_networkManager)
    {
        QNetworkConfigurationManager manager;

        m_networkManager = new QNetworkAccessManager(this);
        m_networkManager->setConfiguration(manager.defaultConfiguration());
        m_networkManager->setCookieJar(m_cookieJar);

        connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onHttpRequestFinished(QNetworkReply*)));
    }

    qDebug() << "Doing HTTP request " << verb << " " << url;
    qDebug() << "With Content-Type: " << contentType << " > Using content: ";
    qDebug() << content;

    QNetworkRequest request(url);
    if (!content.isEmpty() && verb != "GET")
    {
        request.setRawHeader("Content-Type", contentType.toUtf8());

        m_activeRequest->sendBuffer = new QBuffer();
        m_activeRequest->sendBuffer->open(QBuffer::ReadWrite);
        m_activeRequest->sendBuffer->write(content.toUtf8());

        // Ensure that the read will begin from the start of the buffer.
        m_activeRequest->sendBuffer->seek(0);
    }

    m_activeRequest->request = m_networkManager->sendCustomRequest(request, verb.toUtf8(), m_activeRequest->sendBuffer);

    m_activeRequest->request->ignoreSslErrors();
    connect(m_activeRequest->request, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(onHttpRequestError(QNetworkReply::NetworkError)));
}









