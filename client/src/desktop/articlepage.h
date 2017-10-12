/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef ARTICLEPAGE_H
#define ARTICLEPAGE_H

#include "page.h"

class BrowserPage;
class QLabel;
class QModelIndex;
class QStandardItemModel;
class QTreeView;
class QVBoxLayout;

class ArticlePage : public Page
{
    Q_OBJECT

    Q_PROPERTY(QString header READ header WRITE setHeader)
    Q_PROPERTY(QString html READ toHtml WRITE setHtml)
    Q_PROPERTY(QString text READ toPlainText WRITE setText)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)

public:
    explicit ArticlePage(QWidget *parent = 0);

    QString header() const;

    QString toHtml() const;
    QString toPlainText() const;

    QString title() const;

    QString url() const;

public Q_SLOTS:
    void setEnclosures(const QVariantList &enclosures);

    void setHeader(const QString &header);

    void setHtml(const QString &html, const QString &baseUrl = QString());
    void setText(const QString &text, const QString &baseUrl = QString());

    void setUrl(const QString &url);

private Q_SLOTS:
    void openEnclosureUrlInBrowser(const QModelIndex &index);

    void showEnclosureContextMenu(const QPoint &pos);

Q_SIGNALS:
    void openArticleInTab(const QString &title, const QString &url);
    void openUrlInTab(const QString &title, const QString &url);
    void openUrlExternally(const QString &url);
    void openUrlWithPlugin(const QString &url);
    void downloadUrl(const QString &url);
    void showHtmlInTab(const QString &title, const QString &html, const QString &baseUrl = QString());
    void showTextInTab(const QString &title, const QString &text, const QString &baseUrl = QString());
    void titleChanged(const QString &title);
    void urlChanged(const QString &url);

private:
    QStandardItemModel *m_enclosuresModel;

    QLabel *m_headerLabel;
    QLabel *m_enclosuresLabel;

    BrowserPage *m_browser;

    QTreeView *m_enclosuresView;

    QVBoxLayout *m_layout;
};

#endif // ARTICLEPAGE_H
