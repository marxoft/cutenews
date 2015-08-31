/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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
#include <QLineEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebPage>
#include <QWebView>

Browser::Browser(QWidget *parent) :
    QWidget(parent),
    m_urlEdit(new QLineEdit(this)),
    m_toolBar(new QToolBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_webView(new QWebView(this))
{
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Back));
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Forward));
    m_toolBar->addWidget(m_urlEdit);
    m_toolBar->setMovable(false);
    m_toolBar->hide();
    
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

Browser::Browser(const QUrl &url, QWidget *parent) :
    QWidget(parent),
    m_urlEdit(new QLineEdit(this)),
    m_toolBar(new QToolBar(this)),
    m_layout(new QVBoxLayout(this)),
    m_webView(new QWebView(this))
{
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Back));
    m_toolBar->addAction(m_webView->pageAction(QWebPage::Forward));
    m_toolBar->addWidget(m_urlEdit);
    m_toolBar->setMovable(false);
    
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_webView);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    setUrl(url);
}

QString Browser::toHtml() const {
    return m_webView->page()->mainFrame()->toHtml();
}

void Browser::setHtml(const QString &h) {
    m_webView->setHtml(h);
    m_toolBar->hide();
}

QUrl Browser::url() const {
    return m_webView->url();
}

void Browser::setUrl(const QUrl &u) {
    m_webView->setUrl(u);
    m_urlEdit->setText(u.toString());
    m_toolBar->show();
}

void Browser::onUrlChanged(const QUrl &u) {
    m_urlEdit->setText(u.toString());
}
