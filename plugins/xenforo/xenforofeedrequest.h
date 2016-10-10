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

#ifndef XENFOROFEEDREQUEST_H
#define XENFOROFEEDREQUEST_H

#include "feedrequest.h"
#include <qhtmlparser/qhtmlparser.h>
#include <QBuffer>
#include <QVariantMap>
#include <QXmlStreamWriter>

class QNetworkAccessManager;
class QNetworkReply;

class XenforoFeedRequest : public FeedRequest
{
    Q_OBJECT

public:
    explicit XenforoFeedRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual QByteArray result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getFeed(const QVariantMap &settings);

private Q_SLOTS:
    void checkLogin();
    void checkPage();

private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);

    void login(const QUrl &url, const QString &username, const QString &password);

    void getPage(const QUrl &url);

    void followRedirect(const QUrl &url, const char *slot);

    static void fixRelativeUrls(QString &page, const QString &baseUrl);

    static QString getRedirect(const QNetworkReply *reply);

    static QString getLatestPageUrl(const QHtmlElement &element);
    static QString getNextPageUrl(const QHtmlElement &element);

    static QStringList getCategories(const QHtmlElement &element);
    
    static QHtmlElementList getItems(const QHtmlElement &element);
    
    void writeStartFeed();
    void writeEndFeed();
    void writeFeedTitle(const QString &title);
    void writeFeedUrl(const QString &url);

    void writeStartItem();
    void writeEndItem();
    void writeItemAuthor(const QHtmlElement &element);
    void writeItemBody(const QHtmlElement &element);
    void writeItemCategories(const QStringList &categories);
    void writeItemDate(const QHtmlElement &element);
    void writeItemTitle(const QString &title);
    void writeItemUrl(const QHtmlElement &element);
    
    QNetworkAccessManager* networkAccessManager();
    
    static const int MAX_REDIRECTS;

    static const QByteArray USER_AGENT;

    QNetworkAccessManager *m_nam;

    QBuffer m_buffer;
    QXmlStreamWriter m_writer;
    
    QString m_errorString;
    
    Status m_status;

    int m_results;
    int m_redirects;

    QVariantMap m_settings;
};

#endif // XENFOROFEEDREQUEST_H
