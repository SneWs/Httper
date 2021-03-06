#include "MainWnd.h"
#include "ui_MainWnd.h"
#include "CookieJar.h"
#include "MWidget.h"
#include "ComboBoxFocusManager.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"
#include "Base64Dlg.h"

#include <QSettings>
#include <QMenu>
#include <QMessageBox>
#include <QWebView>
#include <QComboBox>

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
#include <QFileDialog>

#include <memory>

namespace
{
    std::map<std::string, std::string> g_supportedTextContentTypes;

    static bool isContentTypeText(const char* contentType)
    {
        std::string actualType(contentType);
        auto contentPos = actualType.find_first_of(';');
        if (contentPos !=  std::string::npos)
            actualType = std::string(actualType.begin(), actualType.begin() + contentPos);

        if (actualType.find("image/") == std::string::npos)
        {
            for (const auto& str : g_supportedTextContentTypes)
            {
                if (str.second == actualType)
                    return true;
            }
        }

        return false;
    }

    static bool isContentTypeImage(const char* contentType)
    {
        std::string actualType(contentType);
        auto contentPos = actualType.find_first_of(';');
        if (contentPos !=  std::string::npos)
            actualType = std::string(actualType.begin(), actualType.begin() + contentPos);

        if (actualType.find("image/") != std::string::npos)
        {
            for (const auto& str : g_supportedTextContentTypes)
            {
                if (str.second == actualType)
                    return true;
            }
        }

        return false;
    }
}

MainWnd::MainWnd(Settings settings)
    : QMainWindow(nullptr)
    , ui(new Ui::MainWnd)
    , m_networkManager(nullptr)
    , m_cookieJar(std::unique_ptr<CookieJar>(new CookieJar))
    , m_activeRequest(nullptr)
    , m_settings(settings)
    , m_openWebViews()
    , m_openBase64Dialogs()
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/AppIcon.png"));

    g_supportedTextContentTypes["Plain Text"] = "text/plain";
    g_supportedTextContentTypes["Html"] = "text/html";
    g_supportedTextContentTypes["Json"] = "application/json";
    g_supportedTextContentTypes["Xml"] = "application/xml";
    g_supportedTextContentTypes["Png"] = "image/png";
    g_supportedTextContentTypes["Jpg"] = "image/jpg";
    g_supportedTextContentTypes["Jpeg"] = "image/jpeg";
    g_supportedTextContentTypes["Gif"] = "image/gif";

    setupTabViews();
    setupViewAsActions();
    connectSignals();
}

MainWnd::~MainWnd()
{
    removeAdditionalWindows();
}

void MainWnd::applySettings()
{
    // Window settings
    if (m_settings.hasWindowSettings())
    {
        move(m_settings.getWindowX(), m_settings.getWindowY());
        resize(m_settings.getWindowWidth(), m_settings.getWindowHeight());
    }

    ui->miEditAutoRedirect->setChecked(m_settings.followRedirects());

    std::string& lastUsedUrl = m_settings.lastUsedUrl();
    if (lastUsedUrl.length() > 0)
        ui->txtUrl->setText(lastUsedUrl.c_str());

    setupVerbs();
    setupContentTypes();

    afterSettingsApplied();
}

void MainWnd::saveSettings()
{
    m_settings.setLastUsedUrl(getEnteredUrl().toStdString());
    m_settings.setWindowGeometry(pos().x(), pos().y(), width(), height());

    // Write our app logic specific settings.
    SettingsManager::writeSettings(m_settings);
}

void MainWnd::closeEvent(QCloseEvent* e)
{
    for (auto& view : m_openWebViews)
        view->close();

    QMainWindow::closeEvent(e);
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
    menu->addAction("Plain Text");
    menu->addSeparator();
    menu->addAction("Html");
    menu->addAction("Json");
    menu->addAction("Xml");
    menu->addSeparator();
    menu->addAction("Image");

    ui->btnViewContentAs->setMenu(menu);
    ui->btnViewContentAs->setPopupMode(QToolButton::InstantPopup);

    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(onViewContentAsActionClicked(QAction*)));
}

void MainWnd::setupVerbs()
{
    auto& verbs = m_settings.verbs();
    if (verbs.size() > 0)
    {
        for (const auto& it : verbs)
            ui->drpVerb->addItem(it.c_str());

        ui->drpVerb->setCurrentIndex(0);
    }

    connect(new ComboBoxFocusManager(ui->drpVerb), SIGNAL(lostFocus()),
        this, SLOT(onVerbDropdownLostFocus()));
}

void MainWnd::setupContentTypes()
{
    auto& contentTypes = m_settings.contentTypes();
    if (contentTypes.size() > 0)
    {
        for (const auto& it : contentTypes)
            ui->drpContentType->addItem(it.c_str());

        ui->drpContentType->setCurrentIndex(0);
    }

    connect(new ComboBoxFocusManager(ui->drpContentType), SIGNAL(lostFocus()),
        this, SLOT(onContentTypeDropdownLostFocus()));
}

void MainWnd::connectSignals()
{
    connect(ui->btnSendRequest, SIGNAL(pressed()), this, SLOT(onSendRequestButtonClicked()));
    connect(ui->btnHeaderAddKeyValue, SIGNAL(pressed()), this, SLOT(onAddHeadersKeyValuePairButtonClicked()));
    connect(ui->btnBrowseForFile, SIGNAL(pressed()), this, SLOT(onOpenFileButtonClicked()));

    // Headers table view
    connect(ui->tblHeaders, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
        this, SLOT(onCurrentHeaderItemSelectionChanged(QTableWidgetItem*, QTableWidgetItem*)));

    // Headers remove item button
    connect(ui->btnHeadersRemoveSelected, SIGNAL(pressed()), this, SLOT(onHeadersRemoveSelectedButtonClicked()));

    // Edit menu items
    connect(ui->miEditSettings, SIGNAL(triggered(bool)), this, SLOT(showSettingsDialog()));
    connect(ui->miEditRemoveAllCookies, SIGNAL(triggered(bool)), this, SLOT(onClearAllCookies()));
    connect(ui->miEditAutoRedirect, SIGNAL(triggered(bool)), this, SLOT(onToggleFollowRedirects(bool)));

    // Tools menu
    connect(ui->miToolsBase64Encode, SIGNAL(triggered(bool)), this, SLOT(onBase64Encode()));
    connect(ui->miToolsBase64Decode, SIGNAL(triggered(bool)), this, SLOT(onBase64Decode()));

    // Window menu items
    connect(ui->miWindowCloseAllButThis, SIGNAL(triggered(bool)), this, SLOT(onCloseAllAdditionalWindowsMenuItemClicked()));

    // Help menu items
    connect(ui->miHelpAbout, SIGNAL(triggered(bool)), this, SLOT(onShowAboutDialog()));
}

void MainWnd::onAdditionalWindowPostAction()
{
    qDebug() << "Open WebViews : " << m_openWebViews.size();
    ui->miWindowCloseAllButThis->setEnabled(m_openWebViews.size() > 0);
}

void MainWnd::removeAdditionalWindows()
{
    m_openWebViews.clear();
    onAdditionalWindowPostAction();
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

    ui->txtResponseData->setPlainText("");

    ui->statusBar->showMessage(tr("Sending Request..."), 0);
    ui->btnSendRequest->setEnabled(false);
    QString verb = getEnteredVerb();
    QString contentType = getEnteredContentType();
    QString contentToSend = ui->txtContentToSend->toPlainText().trimmed();

    ui->lblResponseUrl->setText(verb + " on " + url.toString());
    doHttpRequest(url, verb, contentType, contentToSend, getDefinedHeaders());
}

void MainWnd::onVerbDropdownLostFocus()
{
    auto item = ui->drpVerb->currentText();
    for (auto i = 0; i < ui->drpVerb->count(); i++)
        if (ui->drpVerb->itemText(i).compare(item, Qt::CaseInsensitive) == 0)
            return;

    ui->drpVerb->addItem(item);

    m_settings.verbs().push_back(item.toStdString());
    saveSettings();
}

void MainWnd::onContentTypeDropdownLostFocus()
{
    auto item = ui->drpContentType->currentText();
    for (auto i = 0; i < ui->drpContentType->count(); i++)
        if (ui->drpContentType->itemText(i).compare(item, Qt::CaseInsensitive) == 0)
            return;

    ui->drpContentType->addItem(item);

    m_settings.contentTypes().push_back(item.toStdString());
    saveSettings();
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
    view->setFocusPolicy(Qt::StrongFocus);
    view->setLayout(new QGridLayout(view));
    view->layout()->setMargin(0);
    view->setGeometry(100, 100, 800, 600);

    connect(view, SIGNAL(onWidgetClosed(MWidget*)), this, SLOT(onWebViewClosed(MWidget*)));

    // Special case handling for plain text and images
    if (ac->text().compare("Plain Text", Qt::CaseInsensitive) == 0)
    {
        QPlainTextEdit* txt = new QPlainTextEdit(view);
        txt->setReadOnly(true);
        txt->setPlainText(m_activeRequest->content);
        m_openWebViews.emplace_back(std::unique_ptr<MWidget>(view));

        view->layout()->addWidget(txt);
        view->showNormal();
        view->setFocus();

        onAdditionalWindowPostAction();
        return;
    }

    QWebView* webView = new QWebView(view);
    connect(webView, SIGNAL(titleChanged(QString)), view, SLOT(setWindowTitle(QString)));
    m_openWebViews.emplace_back(std::unique_ptr<MWidget>(view));

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
    else if (ac->text().compare("Image", Qt::CaseInsensitive) == 0)
        webView->setUrl(m_activeRequest->url);

    view->layout()->addWidget(webView);
    view->showNormal();
    view->setFocus();

    onAdditionalWindowPostAction();
}

void MainWnd::onWebViewClosed(MWidget* widget)
{
    auto newEnd = std::remove_if(m_openWebViews.begin(), m_openWebViews.end(), [widget](std::unique_ptr<MWidget>& ptr) {
        return ptr.get() == widget;
    });

    m_openWebViews.erase(newEnd, m_openWebViews.end());

    onAdditionalWindowPostAction();
}

void MainWnd::onCloseAllAdditionalWindowsMenuItemClicked()
{
    removeAdditionalWindows();
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

void MainWnd::onOpenFileButtonClicked()
{
    QString filter = "Any File (*.*)";
    QString selectedFile = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), filter);
    if (selectedFile.isEmpty())
        return;

    QFileInfo file(selectedFile);
    if (!file.exists())
    {
        QMessageBox::warning(this, tr("Error"), tr("File does not exist."), QMessageBox::Ok);
        return;
    }

    ui->txtFile->setText(file.absoluteFilePath());
}

void MainWnd::onBase64DialogClosed(QDialog* dlg)
{
    auto newEnd = std::remove_if(m_openBase64Dialogs.begin(), m_openBase64Dialogs.end(), [dlg](std::unique_ptr<Base64Dlg>& ptr) {
        return ptr.get() == dlg;
    });

    m_openBase64Dialogs.erase(newEnd, m_openBase64Dialogs.end());
}

void MainWnd::showSettingsDialog()
{
    SettingsDlg dlg(this);
    dlg.setModal(true);

    auto result = dlg.exec();
    if (result == QDialog::Accepted)
    {

    }
}

void MainWnd::onClearAllCookies()
{
    auto msg = QMessageBox::warning(this, tr("Are you sure"),
        tr("Are you sure you want to clear all cookies?"), QMessageBox::Yes | QMessageBox::No);

    if (msg != QMessageBox::Yes)
        return;

    qDebug() << "Clearing all cookies from local cookiejar";
    m_cookieJar->removeAll();

    // https://youtu.be/g1uL3xlIUGM?list=RDg1uL3xlIUGM&t=23
    ui->statusBar->showMessage(tr("The cookie jar is now empty."), 5000);
}

void MainWnd::onToggleFollowRedirects(bool checked)
{
    m_settings.setFollowRedirects(checked);
    saveSettings();
}

void MainWnd::onBase64Encode()
{
    auto dlg = new Base64Dlg(Base64Dlg::Mode::Encode, this);
    dlg->setWindowTitle(tr("Base64 Encode"));

    connect(dlg, SIGNAL(onDialogClosed(QDialog*)), this, SLOT(onBase64DialogClosed(QDialog*)));
    m_openBase64Dialogs.emplace_back(std::unique_ptr<Base64Dlg>(dlg));

    dlg->show();
}

void MainWnd::onBase64Decode()
{
    auto dlg = new Base64Dlg(Base64Dlg::Mode::Decode, this);
    dlg->setWindowTitle(tr("Base64 Decode"));

    connect(dlg, SIGNAL(onDialogClosed(QDialog*)), this, SLOT(onBase64DialogClosed(QDialog*)));
    m_openBase64Dialogs.emplace_back(std::unique_ptr<Base64Dlg>(dlg));

    dlg->show();
}

void MainWnd::onShowAboutDialog()
{
    AboutDlg dlg(this);
    dlg.setWindowTitle(tr("About Httper"));
    dlg.setModal(true);
    dlg.exec();
}

void MainWnd::onHttpRequestFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (m_settings.followRedirects() && (statusCode == 301 || statusCode == 302))
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

            qDebug() << "Redirecting to " << newUrl.toString();

            doHttpRequest(newUrl, m_activeRequest->verb, m_activeRequest->contentType,
                m_activeRequest->content, m_activeRequest->headers);

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

    bool hasContent = false;
    auto contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (isContentTypeText(contentType.toUtf8()))
    {
        m_activeRequest->content = reply->readAll();

        hasContent = m_activeRequest->content.size();
        ui->txtResponseData->setPlainText(m_activeRequest->content);
    }
    else if (isContentTypeImage(contentType.toUtf8()))
    {
        m_activeRequest->content = "";

        hasContent = true;
        ui->txtResponseData->setPlainText("The response body is of image type, to view it, use the view as button.");
    }

    ui->btnViewContentAs->setEnabled(hasContent);

    if (ui->tabContainer->count() < 2)
        ui->tabContainer->insertTab(1, ui->tabResponse, tr("Response"));

    ui->tabContainer->setCurrentIndex(1);
    ui->btnSendRequest->setEnabled(true);
    ui->statusBar->showMessage(tr("Response Received"), 5000);
}

void MainWnd::onHttpRequestError(QNetworkReply::NetworkError error)
{
    qDebug() << "HTTP Error: " << error;
    emit m_networkManager->finished(m_activeRequest->request.get());
}

void MainWnd::afterSettingsApplied()
{
    ui->statusBar->showMessage(tr("Welcome to Httper"), 3000);
    qApp->processEvents();
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

Headers MainWnd::getDefinedHeaders() const
{
    Headers retVal;

    auto numRows = ui->tblHeaders->rowCount();
    for (auto i = 0; i < numRows; i++)
    {
        auto key = ui->tblHeaders->item(i, 0)->text();
        auto value = ui->tblHeaders->item(i, 1)->text();

        retVal[key.toStdString()] = value.toStdString();
    }

    return retVal;
}

void MainWnd::doHttpRequest(QUrl url, QString verb, QString contentType,
    QString content, Headers headers)
{
    if (m_activeRequest.get())
    {
        if (!m_activeRequest->request->isFinished())
            m_activeRequest->request->abort();
    }

    m_activeRequest = std::unique_ptr<RequestInfo>(new RequestInfo(url, verb, contentType, content));
    m_activeRequest->headers = headers;

    if (!m_networkManager.get())
    {
        m_networkManager = std::unique_ptr<QNetworkAccessManager>(new QNetworkAccessManager(this));

        QNetworkConfigurationManager manager;
        m_networkManager->setConfiguration(manager.defaultConfiguration());
        m_networkManager->setCookieJar(m_cookieJar.get());

        connect(m_networkManager.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(onHttpRequestFinished(QNetworkReply*)));
    }

    QNetworkRequest request(url);
    for (const auto& header : headers)
        request.setRawHeader(header.first.c_str(), header.second.c_str());

    if (!content.isEmpty() && verb != "GET")
    {
        request.setRawHeader("Content-Type", contentType.toUtf8());

        m_activeRequest->sendBuffer = std::unique_ptr<QBuffer>(new QBuffer);
        m_activeRequest->sendBuffer->open(QBuffer::ReadWrite);
        m_activeRequest->sendBuffer->write(content.toUtf8());

        // Ensure that the read will begin from the start of the buffer.
        m_activeRequest->sendBuffer->seek(0);
    }

    m_activeRequest->request.reset(m_networkManager->sendCustomRequest(request, verb.toUtf8(), m_activeRequest->sendBuffer.get()));

    m_activeRequest->request->ignoreSslErrors();
    connect(m_activeRequest->request.get(), SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(onHttpRequestError(QNetworkReply::NetworkError)));
}

