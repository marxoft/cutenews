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

#ifndef POLITICOFEEDREQUEST_H
#define POLITICOFEEDREQUEST_H

#include "feedrequest.h"
#include "feedparser.h"
#include <qhtmlparser/qhtmlparser.h>
#include <QBuffer>
#include <QVariantMap>
#include <QXmlStreamWriter>

class ArticleRequest;
class PoliticoArticleRequest;
class QNetworkAccessManager;
class QNetworkReply;

class PoliticoFeedRequest : public FeedRequest
{
    Q_OBJECT

public:
    explicit PoliticoFeedRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual QByteArray result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getFeed(const QVariantMap &settings);

private Q_SLOTS:
    void checkFeed();
    void checkArticle(ArticleRequest *request);

private:
    void setErrorString(const QString &e);

    void setResult(const QByteArray &r);
    
    void setStatus(Status s);

    void getArticle(const QString &url);

    void followRedirect(const QString &url, const char *slot);

    static QString getRedirect(const QNetworkReply *reply);
    
    void writeStartFeed(const QString &url, const QString &title, const QString &description);
    void writeEndFeed();

    void writeStartItem();
    void writeEndItem();
    void writeItemAuthor(const QString &author);
    void writeItemBody(const QString &body);
    void writeItemCategories(const QStringList &categories);
    void writeItemDate(const QDateTime &date);
    void writeItemEnclosures(const QVariantList &enclosures);
    void writeItemTitle(const QString &title);
    void writeItemUrl(const QString &url);
    
    PoliticoArticleRequest* articleRequest();
    QNetworkAccessManager* networkAccessManager();

    static const int MAX_REDIRECTS;

    static const QString DEFAULT_FEED_URL;
    static const QString ICON_URL;

    static const QByteArray USER_AGENT;

    PoliticoArticleRequest *m_request;
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

#endif // POLITICOFEEDREQUEST_H
