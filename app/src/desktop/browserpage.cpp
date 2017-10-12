/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "browserpage.h"
#include "cachingnetworkaccessmanager.h"
#include "pluginmanager.h"
#include <QLineEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>
#include <QWebHitTestResult>
#include <QMenu>

const QByteArray BrowserPage::STYLE_SHEET = QByteArray("data:text/css;charset=utf-8;base64,")
    + QByteArray("img { max-width: 100%; } iframe { max-width: 100%; }").toBase64();

BrowserPage::BrowserPage(QWidget *parent) :
    Page(parent),
    m_nam(new CachingNetworkAccessManager(this)),
    m_urlEdit(new QLineEdit(this)),
    m_toolBar(new QToolBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_webView(new QWebView(this))
{
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Back));
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Forward));
    m_toolBar->addWidget(m_urlEdit);
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Reload));
    m_toolBar->setMovable(false);
    m_toolBar->hide();
    
    m_webView->setStyleSheet("background: #fff");
    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_webView->page()->setNetworkAccessManager(m_nam);
    m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_webView->settings()->setUserStyleSheetUrl(QUrl::fromEncoded(STYLE_SHEET));

    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);    

    connect(m_webView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(m_webView, SIGNAL(linkClicked(QUrl)), this, SLOT(setUrl(QUrl)));
    connect(m_webView, SIGNAL(statusBarMessage(QString)), this, SIGNAL(information(QString)));
    connect(m_webView, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(onUrlChanged(QUrl)));
    connect(m_webView->page(), SIGNAL(linkHovered(QString, QString, QString)), this, SIGNAL(information(QString)));
}

BrowserPage::BrowserPage(const QString &url, QWidget *parent) :
    Page(parent),
    m_nam(new CachingNetworkAccessManager(this)),
    m_urlEdit(new QLineEdit(this)),
    m_toolBar(new QToolBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_webView(new QWebView(this))
{
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Back));
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Forward));
    m_toolBar->addWidget(m_urlEdit);
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Reload));
    m_toolBar->setMovable(false);

    m_webView->setStyleSheet("background: #fff");
    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_webView->page()->setNetworkAccessManager(m_nam);
    m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    m_webView->settings()->setUserStyleSheetUrl(QUrl::fromEncoded(STYLE_SHEET));

    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);    

    connect(m_webView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(m_webView, SIGNAL(linkClicked(QUrl)), this, SLOT(setUrl(QUrl)));
    connect(m_webView, SIGNAL(statusBarMessage(QString)), this, SIGNAL(information(QString)));
    connect(m_webView, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(onUrlChanged(QUrl)));
    connect(m_webView->page(), SIGNAL(linkHovered(QString, QString, QString)), this, SIGNAL(information(QString)));
    
    setUrl(url);
}

QString BrowserPage::toHtml() const {
    return m_webView->page()->mainFrame()->toHtml();
}

void BrowserPage::setHtml(const QString &html, const QString &baseUrl) {
    m_webView->setHtml(html, baseUrl);
    m_toolBar->hide();
}

QString BrowserPage::toPlainText() const {
    return m_webView->page()->mainFrame()->toPlainText();
}

void BrowserPage::setText(const QString &text, const QString &baseUrl) {
    m_webView->setContent(text.toUtf8(), "text/plain", baseUrl);
    m_toolBar->hide();
}

QString BrowserPage::title() const {
    return m_webView->title();
}

QString BrowserPage::url() const {
    return m_webView->url().toString();
}

void BrowserPage::setUrl(const QString &u) {
    m_webView->setUrl(u);
    m_urlEdit->setText(u);
    m_toolBar->show();
}

void BrowserPage::setUrl(const QUrl &u) {
    m_webView->setUrl(u);
    m_urlEdit->setText(u.toString());
    m_toolBar->show();
}

void BrowserPage::onUrlChanged(const QUrl &u) {
    const QString s = u.toString();
    m_urlEdit->setText(s);
    emit urlChanged(s);
}

void BrowserPage::showContextMenu(const QPoint &pos) {
    const QWebHitTestResult result = m_webView->page()->mainFrame()->hitTestContent(pos);
    const QString url = result.linkUrl().toString();
    QMenu menu(this);
    
    if (!url.isEmpty()) {
        const FeedPluginConfig *articleConfig = PluginManager::instance()->getConfigForArticle(url);
        const FeedPluginConfig *enclosureConfig = PluginManager::instance()->getConfigForEnclosure(url);
        menu.addAction(m_webView->pageAction(QWebPage::CopyLinkToClipboard));
        QAction *articleAction = articleConfig ? menu.addAction(tr("Open Article in New Tab Via %1")
                .arg(articleConfig->displayName())) : 0;
        QAction *tabAction = menu.addAction(tr("Open Link in New Tab"));
        QAction *externalAction = menu.addAction(tr("Open Link Externally"));
        QAction *pluginAction = enclosureConfig ? menu.addAction(tr("Open Link Externally Via %1")
                .arg(enclosureConfig->displayName())) : 0;
        QAction *downloadAction = menu.addAction(tr("Download Link"));
        QAction *action = menu.exec(m_webView->mapToGlobal(pos));

        if (!action) {
            return;
        }

        if (action == articleAction) {
            emit openArticleInTab(result.linkText(), url);
        }
        else if (action == tabAction) {
            emit openUrlInTab(result.linkText(), url);
        }
        else if (action == externalAction) {
            emit openUrlExternally(url);
        }
        else if (action == pluginAction) {
            emit openUrlWithPlugin(url);
        }
        else if (action == downloadAction) {
            emit downloadUrl(url);
        }
    }
    else {
        menu.addAction(m_webView->pageAction(QWebPage::Copy));
        menu.addAction(m_webView->pageAction(QWebPage::SelectAll));
        QAction *sourceAction = menu.addAction(tr("View Page Source"));
        QAction *action = menu.exec(m_webView->mapToGlobal(pos));

        if (!action) {
            return;
        }

        if (action == sourceAction) {
            emit showTextInTab(tr("View Page Source"), toHtml(), m_webView->page()->mainFrame()->baseUrl().toString());
        }
    }
}
