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

#ifndef BROWSERPAGE_H
#define BROWSERPAGE_H

#include "page.h"

class CachingNetworkAccessManager;
class QLineEdit;
class QToolBar;
class QVBoxLayout;
class QWebView;

class BrowserPage : public Page
{
    Q_OBJECT
    
    Q_PROPERTY(QString html READ toHtml WRITE setHtml)
    Q_PROPERTY(QString text READ toPlainText WRITE setText)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString url READ url WRITE setUrl)

public:
    explicit BrowserPage(QWidget *parent = 0);
    explicit BrowserPage(const QString &url, QWidget *parent = 0);
    
    QString toHtml() const;
    QString toPlainText() const;

    QString title() const;
    
    QString url() const;

public Q_SLOTS:
    void setHtml(const QString &html, const QString &baseUrl = QString());
    void setText(const QString &text, const QString &baseUrl = QString());

    void setUrl(const QString &u);

private Q_SLOTS:
    void setUrl(const QUrl &u);

    void onUrlChanged(const QUrl &u);

    void showContextMenu(const QPoint &pos);

Q_SIGNALS:
    void openUrlInTab(const QString &title, const QString &url);
    void openUrlExternally(const QString &url);
    void openUrlWithPlugin(const QString &url);
    void downloadUrl(const QString &url);
    void downloadUrlWithPlugin(const QString &url);
    void showHtmlInTab(const QString &title, const QString &html, const QString &baseUrl = QString());
    void showTextInTab(const QString &title, const QString &text, const QString &baseUrl = QString());
    void titleChanged(const QString &title);
    void urlChanged(const QString &url);
    
private:
    static const QByteArray STYLE_SHEET;
    
    CachingNetworkAccessManager *m_nam;
    
    QLineEdit *m_urlEdit;
    
    QToolBar *m_toolBar;
    
    QVBoxLayout *m_layout;
    
    QWebView *m_webView;
};

#endif // BROWSERPAGE_H
