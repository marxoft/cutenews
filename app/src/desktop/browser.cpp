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

#include "browser.h"
#include "cachingnetworkaccessmanager.h"
#include "pluginmanager.h"
#include <QApplication>
#include <QClipboard>
#include <QLineEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>
#include <QWebHitTestResult>
#include <QMenu>
#include <QAction>

const QByteArray Browser::STYLE_SHEET = QByteArray("data:text/css;charset=utf-8;base64,")
    + QByteArray("img { max-width: 100%; } iframe { max-width: 100%; }").toBase64();

Browser::Browser(QWidget *parent) :
    QWidget(parent),
    m_nam(new CachingNetworkAccessManager(this)),
    m_urlEdit(new QLineEdit(this)),
    m_toolBar(new QToolBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_webView(new QWebView(this)),
    m_menu(new QMenu(this)),
    m_copyAction(new QAction(tr("&Copy URL"), this)),
    m_tabAction(new QAction(tr("Open in &tab"), this)),
    m_browserAction(new QAction(tr("Open in &browser"), this)),
    m_externalAction(new QAction(tr("Open &externally"), this)),
    m_pluginAction(new QAction(tr("Open externally using &plugin"), this)),
    m_downloadAction(new QAction(tr("&Download"), this)),
    m_downloadPluginAction(new QAction(tr("Download using &plugin"), this))
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

    m_menu->addAction(m_webView->pageAction(QWebPage::Reload));
    m_menu->addAction(m_copyAction);
    m_menu->addAction(m_tabAction);
    m_menu->addAction(m_browserAction);
    m_menu->addAction(m_externalAction);
    m_menu->addAction(m_pluginAction);
    m_menu->addAction(m_downloadAction);
    m_menu->addAction(m_downloadPluginAction);
    
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);    

    connect(m_webView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(m_webView, SIGNAL(linkClicked(QUrl)), this, SLOT(onLinkClicked(QUrl)));
    connect(m_webView, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(onUrlChanged(QUrl)));
    connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copyUrl()));
    connect(m_tabAction, SIGNAL(triggered()), this, SLOT(openUrlInTab()));
    connect(m_browserAction, SIGNAL(triggered()), this, SLOT(openUrlInBrowser()));
    connect(m_externalAction, SIGNAL(triggered()), this, SLOT(openUrlExternally()));
    connect(m_pluginAction, SIGNAL(triggered()), this, SLOT(openUrlWithPlugin()));
    connect(m_downloadAction, SIGNAL(triggered()), this, SLOT(downloadUrl()));
    connect(m_downloadPluginAction, SIGNAL(triggered()), this, SLOT(downloadUrlWithPlugin()));
}

Browser::Browser(const QUrl &url, QWidget *parent) :
    QWidget(parent),
    m_nam(new CachingNetworkAccessManager(this)),
    m_urlEdit(new QLineEdit(this)),
    m_toolBar(new QToolBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_webView(new QWebView(this)),
    m_menu(new QMenu(this)),
    m_copyAction(new QAction(tr("Copy URL"), this)),
    m_tabAction(new QAction(tr("Open in tab"), this)),
    m_browserAction(new QAction(tr("Open in browser"), this)),
    m_externalAction(new QAction(tr("Open externally"), this)),
    m_pluginAction(new QAction(tr("Open externally using &plugin"), this)),
    m_downloadAction(new QAction(tr("&Download"), this)),
    m_downloadPluginAction(new QAction(tr("Download using &plugin"), this))
{
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Back));
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Forward));
    m_toolBar->addWidget(m_urlEdit);
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Reload));
    m_toolBar->setMovable(false);

    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_webView->page()->setNetworkAccessManager(m_nam);
    m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    m_menu->addAction(m_webView->pageAction(QWebPage::Reload));
    m_menu->addAction(m_copyAction);
    m_menu->addAction(m_tabAction);
    m_menu->addAction(m_browserAction);
    m_menu->addAction(m_externalAction);
    m_menu->addAction(m_pluginAction);
    m_menu->addAction(m_downloadAction);
    m_menu->addAction(m_downloadPluginAction);
    
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);    

    connect(m_webView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(m_webView, SIGNAL(linkClicked(QUrl)), this, SLOT(onLinkClicked(QUrl)));
    connect(m_webView, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(onUrlChanged(QUrl)));
    connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copyUrl()));
    connect(m_tabAction, SIGNAL(triggered()), this, SLOT(openUrlInTab()));
    connect(m_browserAction, SIGNAL(triggered()), this, SLOT(openUrlInBrowser()));
    connect(m_externalAction, SIGNAL(triggered()), this, SLOT(openUrlExternally()));
    connect(m_pluginAction, SIGNAL(triggered()), this, SLOT(openUrlWithPlugin()));
    connect(m_downloadAction, SIGNAL(triggered()), this, SLOT(downloadUrl()));
    connect(m_downloadPluginAction, SIGNAL(triggered()), this, SLOT(downloadUrlWithPlugin()));
    
    setUrl(url);
}

QString Browser::toHtml() const {
    return m_webView->page()->mainFrame()->toHtml();
}

void Browser::setHtml(const QString &h, const QUrl &baseUrl) {
    m_webView->setHtml(h, baseUrl);
    m_toolBar->hide();
}

QString Browser::title() const {
    return m_webView->title();
}

QUrl Browser::url() const {
    return m_webView->url();
}

void Browser::setUrl(const QUrl &u) {
    m_webView->setUrl(u);
    m_urlEdit->setText(u.toString());
    m_toolBar->show();
}

void Browser::onLinkClicked(const QUrl &u) {
    setUrl(u);
}

void Browser::onUrlChanged(const QUrl &u) {
    m_urlEdit->setText(u.toString());
    emit urlChanged(u);
}

void Browser::showContextMenu(const QPoint &pos) {
    const QWebHitTestResult res = m_webView->page()->mainFrame()->hitTestContent(pos);
    m_url = res.linkUrl();
    
    if (m_url.isValid()) {
        m_copyAction->setEnabled(true);
        m_tabAction->setEnabled(true);
        m_browserAction->setEnabled(true);
        m_externalAction->setEnabled(true);
        m_pluginAction->setEnabled(PluginManager::instance()->enclosureIsSupported(m_url.toString()));
        m_downloadAction->setEnabled(true);
        m_downloadPluginAction->setEnabled(m_pluginAction->isEnabled());
    }
    else {
        m_copyAction->setEnabled(false);
        m_tabAction->setEnabled(false);
        m_browserAction->setEnabled(false);
        m_externalAction->setEnabled(false);
        m_pluginAction->setEnabled(false);
        m_downloadAction->setEnabled(false);
        m_downloadPluginAction->setEnabled(false);
    }

    m_menu->popup(m_webView->mapToGlobal(pos));
}

void Browser::copyUrl() {
    QApplication::clipboard()->setText(m_url.toString());
}

void Browser::openUrlInBrowser() {
    setUrl(m_url);
}

void Browser::openUrlInTab() {
    emit openUrlInTab(m_url.toString());
}

void Browser::openUrlExternally() {
    emit openUrlExternally(m_url.toString());
}

void Browser::openUrlWithPlugin() {
    emit openUrlWithPlugin(m_url.toString());
}

void Browser::downloadUrl() {
    emit downloadUrl(m_url.toString());
}

void Browser::downloadUrlWithPlugin() {
    emit downloadUrlWithPlugin(m_url.toString());
}
