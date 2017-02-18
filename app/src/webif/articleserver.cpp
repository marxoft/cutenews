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

#include "articleserver.h"
#include "dbconnection.h"
#include "definitions.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "utils.h"

static QVariantMap articleQueryToMap(const DBConnection *connection, const QString &authority) {
    QVariantMap article;
    article["id"] = connection->value(0);
    article["author"] = connection->value(1);
    article["body"] = connection->value(2).toString().replace(CACHE_AUTHORITY, authority);
    article["categories"] = connection->value(3).toString().split(", ", QString::SkipEmptyParts);
    article["date"] = connection->value(4);
    article["enclosures"] = QtJson::Json::parse(connection->value(5).toString());
    article["favourite"] = connection->value(6).toBool();
    article["read"] = connection->value(7).toBool();
    article["subscriptionId"] = connection->value(8);
    article["title"] = connection->value(9);
    article["url"] = connection->value(10);
    return article;
}

static void writeResponse(QHttpResponse *response, int responseCode, const QByteArray &data = QByteArray()) {
    response->setHeader("Content-Type", "application/json");
    response->setHeader("Content-Length", QByteArray::number(data.size()));
    response->writeHead(responseCode);
    response->end(data);
}

ArticleServer::ArticleServer(QObject *parent) :
    QObject(parent)
{
}

bool ArticleServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first() != "articles")) {
        return false;
    }
    
    QString authority("http://" + request->header("host"));
    
    if (CACHE_AUTHORITY.endsWith("/")) {
        authority.append("/");
    }
    
    response->setProperty("authority", authority);
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            enqueueResponse(response);
            const QString ids = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!ids.isEmpty()) {
                DBConnection::connection(this,
                SLOT(onArticlesFetched(DBConnection*)))->fetchArticles(ids.split(",", QString::SkipEmptyParts));
                return true;
            }
            
            const int offset = Utils::urlQueryItemValue(request->url(), "offset", "0").toInt();
            const int limit = Utils::urlQueryItemValue(request->url(), "limit", "0").toInt();
            const QString subscriptionId = Utils::urlQueryItemValue(request->url(), "subscriptionId");
            
            if (!subscriptionId.isEmpty()) {
                DBConnection::connection(this,
                SLOT(onArticlesFetched(DBConnection*)))->fetchArticlesForSubscription(subscriptionId, offset, limit);
                return true;
            }
            
            const QString query = Utils::urlQueryItemValue(request->url(), "search");
                
            if (!query.isEmpty()) {
                DBConnection::connection(this,
                SLOT(onArticlesFetched(DBConnection*)))->searchArticles(query, offset, limit);
                return true;
            }
            
            DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->fetchArticles(offset, limit);
            return true;
        }
               
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.size() == 2) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            enqueueResponse(response);
            
            if (parts.at(1) == "read") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleRead(id, true, true);
            }
            else if (parts.at(1) == "unread") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleRead(id, false, true);
            }
            else if (parts.at(1) == "favourite") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleFavourite(id, true,
                                                                                                            true);
            }
            else if (parts.at(1) == "unfavourite") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleFavourite(id, false,
                                                                                                            true);
            }
            else if (parts.at(1) == "deleteread") {
                const int expiryDate = Utils::urlQueryItemValue(request->url(), "expiry", 0).toInt();
                DBConnection::connection(this,
                SLOT(onReadArticlesDeleted(DBConnection*)))->deleteReadArticles(expiryDate);
            }
            else {
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->fetchArticle(parts.at(1));
            }

            return true;
        }

        if (request->method() == QHttpRequest::HTTP_DELETE) {
            enqueueResponse(response);
            DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->deleteArticle(parts.at(1));
            return true;
        }
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void ArticleServer::enqueueResponse(QHttpResponse *response) {
    m_responses.enqueue(response);
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

QHttpResponse* ArticleServer::dequeueResponse() {
    if (!m_responses.isEmpty()) {
        QHttpResponse *response = m_responses.dequeue();
        disconnect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
        return response;
    }
    
    return 0;
}

void ArticleServer::onArticleFetched(DBConnection *connection) {
    if (QHttpResponse *response = dequeueResponse()) {
        const QString authority = response->property("authority").toString();
        
        if (connection->status() == DBConnection::Ready) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articleQueryToMap(connection,
                                                                                                        authority)));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    
    connection->deleteLater();
}

void ArticleServer::onArticlesFetched(DBConnection *connection) {
    if (QHttpResponse *response = dequeueResponse()) {
        const QString authority = response->property("authority").toString();
        
        if (connection->status() == DBConnection::Ready) {
            QVariantList articles;
            
            while (connection->nextRecord()) {
                articles << articleQueryToMap(connection, authority);
            }
            
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articles));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    
    connection->deleteLater();
}

void ArticleServer::onReadArticlesDeleted(DBConnection *connection) {
    if (QHttpResponse *response = dequeueResponse()) {
        if (connection->status() == DBConnection::Ready) {
            QVariantMap result;
            result["count"] = connection->numRowsAffected();
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(result));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    
    connection->deleteLater();
}

void ArticleServer::onConnectionFinished(DBConnection *connection) {
    if (QHttpResponse *response = dequeueResponse()) {
        if (connection->status() == DBConnection::Ready) {
            writeResponse(response, QHttpResponse::STATUS_OK);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
    
    connection->deleteLater();
}

void ArticleServer::onResponseDone() {
    if (QHttpResponse *response = qobject_cast<QHttpResponse*>(sender())) {
        m_responses.removeOne(response);
        disconnect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
    }
}
