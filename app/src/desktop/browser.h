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

#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QUrl>

class QVBoxLayout;
class QLineEdit;
class QToolBar;
class QWebView;

class Browser : public QWidget
{
    Q_OBJECT
    
    Q_PROPERTY(QString html READ toHtml WRITE setHtml)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

public:
    explicit Browser(QWidget *parent = 0);
    explicit Browser(const QUrl &url, QWidget *parent = 0);
    
    QString toHtml() const;
    void setHtml(const QString &h);
    
    QUrl url() const;
    void setUrl(const QUrl &u);

private Q_SLOTS:
    void onUrlChanged(const QUrl &u);

private:    
    QLineEdit *m_urlEdit;
    
    QToolBar *m_toolBar;
    
    QVBoxLayout *m_layout;
    
    QWebView *m_webView;
};

#endif // BROWSER_H
