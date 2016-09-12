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

#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QUrl>

class CachingNetworkAccessManager;
class QAction;
class QLineEdit;
class QMenu;
class QToolBar;
class QVBoxLayout;
class QWebView;

class Browser : public QWidget
{
    Q_OBJECT
    
    Q_PROPERTY(QString html READ toHtml WRITE setHtml)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

public:
    explicit Browser(QWidget *parent = 0);
    explicit Browser(const QUrl &url, QWidget *parent = 0);
    
    QString toHtml() const;
    void setHtml(const QString &h, const QUrl &baseUrl = QUrl());

    QString title() const;
    
    QUrl url() const;
    void setUrl(const QUrl &u);

private Q_SLOTS:
    void onLinkClicked(const QUrl &u);
    void onUrlChanged(const QUrl &u);

    void showContextMenu(const QPoint &pos);

    void copyUrl();
    void openUrlInTab();
    void openUrlInBrowser();
    void openUrlExternally();

Q_SIGNALS:
    void openUrlExternally(const QString &url);
    void openUrlInTab(const QString &url);
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    
private:
    CachingNetworkAccessManager *m_nam;
    
    QLineEdit *m_urlEdit;
    
    QToolBar *m_toolBar;
    
    QVBoxLayout *m_layout;
    
    QWebView *m_webView;

    QMenu *m_menu;

    QAction *m_copyAction;
    QAction *m_tabAction;
    QAction *m_browserAction;
    QAction *m_externalAction;
    
    QUrl m_url;
};

#endif // BROWSER_H
