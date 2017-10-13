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

#ifndef TWITTERFEEDREQUEST_H
#define TWITTERFEEDREQUEST_H

#include "feedrequest.h"
#include <qhtmlparser/qhtmlparser.h>
#include <QBuffer>
#include <QVariantMap>
#include <QXmlStreamWriter>

class QNetworkAccessManager;
class QNetworkReply;

class TwitterFeedRequest : public FeedRequest
{
    Q_OBJECT

public:
    explicit TwitterFeedRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual QByteArray result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getFeed(const QVariantMap &settings);

private Q_SLOTS:
    void checkPage();

private:
    void setErrorString(const QString &e);

    void setResult(const QByteArray &r);
    
    void setStatus(Status s);

    void getPage(const QString &url);
    
    void followRedirect(const QString &url, const char *slot);

    static void fixRelativeUrls(QString &page, const QString &baseUrl);

    static QString getRedirect(const QNetworkReply *reply);

    static QHtmlElementList getItems(const QHtmlElement &element);
    
    void writeStartFeed(const QHtmlElement &element);
    void writeEndFeed();

    void writeStartItem();
    void writeEndItem();
    void writeItemAuthor(const QHtmlElement &element);
    void writeItemBody(const QHtmlElement &element, bool includeImages);
    void writeItemCategories(const QHtmlElement &element);
    void writeItemDate(const QHtmlElement &element);
    void writeItemEnclosures(const QHtmlElement &element);
    void writeItemTitle(const QHtmlElement &element);
    void writeItemUrl(const QHtmlElement &element);
    
    QNetworkAccessManager* networkAccessManager();    

    static const int MAX_REDIRECTS;

    static const QString BASE_URL;
    static const QString HASHTAG_URL;
    static const QString USER_URL;
    static const QString ICON_URL;

    static const QByteArray USER_AGENT;

    QNetworkAccessManager *m_nam;

    QBuffer m_buffer;
    QXmlStreamWriter m_writer;
    
    QString m_errorString;
    
    Status m_status;

    int m_redirects;
    
    QVariantMap m_settings;
};

#endif // TWITTERFEEDREQUEST_H
