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

#ifndef BBCFEEDREQUEST_H
#define BBCFEEDREQUEST_H

#include "feedrequest.h"
#include "feedparser.h"
#include <qhtmlparser/qhtmlparser.h>
#include <QBuffer>
#include <QVariantMap>
#include <QXmlStreamWriter>

class QNetworkAccessManager;
class QNetworkReply;

class BbcFeedRequest : public FeedRequest
{
    Q_OBJECT

public:
    explicit BbcFeedRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual QByteArray result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getFeed(const QVariantMap &settings);

private Q_SLOTS:
    void checkFeed();
    void checkPage();

private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);

    void getPage(const QString &url);

    void followRedirect(const QString &url, const char *slot);

    static void fixRelativeUrls(QString &page, const QString &baseUrl);

    static QString getRedirect(const QNetworkReply *reply);
    
    static QString unescape(const QString &text);
    
    void writeStartFeed();
    void writeEndFeed();
    void writeFeedTitle(const QString &title);
    void writeFeedUrl(const QString &url);

    void writeStartItem();
    void writeEndItem();
    void writeItemBody(const QHtmlElement &element);
    void writeItemDate(const QDateTime &date);
    void writeItemTitle(const QString &title);
    void writeItemUrl(const QString &url);
    
    QNetworkAccessManager* networkAccessManager();

    static const int MAX_REDIRECTS;

    static const QString BASE_URL;
    static const QString ICON_URL;

    static const QByteArray USER_AGENT;

    QNetworkAccessManager *m_nam;

    FeedParser m_parser;
    QBuffer m_buffer;
    QXmlStreamWriter m_writer;
    
    QString m_errorString;
    
    Status m_status;

    int m_results;
    int m_redirects;
    
    QVariantMap m_settings;
};

#endif // BBCFEEDREQUEST_H
