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
#include "pluginmanager.h"
#include "pluginsettings.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
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

static QVariantMap articleResultToMap(const ArticleResult &result, const QString &authority) {
    QVariantMap article;
    article["author"] = result.author;
    article["body"] = Utils::replaceSrcPaths(result.body, authority + TEMPORARY_CACHE_PATH);
    article["categories"] = result.categories;
    article["date"] = result.date;
    article["enclosures"] = result.enclosures;
    article["title"] = result.title;
    article["url"] = result.url;
    return article;
}

ArticleServer::ArticleServer(QObject *parent) :
    QObject(parent)
{
}

bool ArticleServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first().compare("articles", Qt::CaseInsensitive) != 0)) {
        return false;
    }
    
    QString authority("http://" + request->header("host"));
    
    if (CACHE_AUTHORITY.endsWith("/")) {
        authority.append("/");
    }
    
    response->setProperty("authority", authority);
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString ids = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!ids.isEmpty()) {
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)));
                addResponse(connection, response);
                connection->fetchArticles(ids.split(",", QString::SkipEmptyParts));
                return true;
            }
            
            const int offset = Utils::urlQueryItemValue(request->url(), "offset", "0").toInt();
            const int limit = Utils::urlQueryItemValue(request->url(), "limit", "0").toInt();
            const QString subscriptionId = Utils::urlQueryItemValue(request->url(), "subscriptionId");
            
            if (!subscriptionId.isEmpty()) {
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)));
                addResponse(connection, response);
                connection->fetchArticlesForSubscription(subscriptionId, offset, limit);
                return true;
            }
            
            const QString query = Utils::urlQueryItemValue(request->url(), "search");
                
            if (!query.isEmpty()) {
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)));
                addResponse(connection, response);
                connection->searchArticles(query, offset, limit);
                return true;
            }
            
            DBConnection *connection = DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)));
            addResponse(connection, response);
            connection->fetchArticles(offset, limit);
            return true;
        }
               
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.size() == 2) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            if (parts.at(1).compare("read", Qt::CaseInsensitive) == 0) {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)));
                addResponse(connection, response);
                connection->markArticleRead(id, true, true);
            }
            else if (parts.at(1).compare("unread", Qt::CaseInsensitive) == 0) {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)));
                addResponse(connection, response);
                connection->markArticleRead(id, false, true);
            }
            else if (parts.at(1).compare("favourite", Qt::CaseInsensitive) == 0) {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)));
                addResponse(connection, response);
                connection->markArticleFavourite(id, true, true);
            }
            else if (parts.at(1).compare("unfavourite", Qt::CaseInsensitive) == 0) {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                DBConnection *connection = DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)));
                addResponse(connection, response);
                connection->markArticleFavourite(id, false, true);
            }
            else if (parts.at(1).compare("deleteread", Qt::CaseInsensitive) == 0) {
                const int expiryDate = Utils::urlQueryItemValue(request->url(), "expiry", 0).toInt();
                DBConnection *connection = DBConnection::connection(this, SLOT(onReadArticlesDeleted(DBConnection*)));
                addResponse(connection, response);
                connection->deleteReadArticles(expiryDate);
            }
            else {
                const QString id = QString::fromUtf8(QByteArray::fromBase64(parts.at(1).toUtf8()));
                FeedPluginConfig *config = PluginManager::instance()->getConfigForArticle(id);

                if (config) {
                    ArticleRequest *request = PluginManager::instance()->articleRequest(id, this);

                    if (request) {
                        addResponse(request, response);
                        PluginSettings settings(config->id());
                        request->getArticle(id, settings.values());
                    }
                    else {
                        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
                    }
                }
                else {
                    DBConnection *connection = DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)));
                    addResponse(connection, response);
                    connection->fetchArticle(parts.at(1));
                }
            }

            return true;
        }

        if (request->method() == QHttpRequest::HTTP_DELETE) {
            DBConnection *connection = DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)));
            addResponse(connection, response);
            connection->deleteArticle(parts.at(1));
            return true;
        }
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void ArticleServer::addResponse(DBConnection *connection, QHttpResponse *response) {
    m_responses.insert(connection, response);
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

void ArticleServer::addResponse(ArticleRequest *request, QHttpResponse *response) {
    m_responses.insert(request, response);
    connect(request, SIGNAL(finished(ArticleRequest*)), this, SLOT(onArticleRequestFinished(ArticleRequest*)));
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

QHttpResponse* ArticleServer::getResponse(QObject *obj) {
    return m_responses.value(obj);
}

void ArticleServer::removeResponse(QHttpResponse *response) {
    if (QObject *obj = m_responses.key(response)) {
        m_responses.remove(obj);
        obj->deleteLater();
        disconnect(response, 0, this, 0);
    }
}

void ArticleServer::onArticleFetched(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
        if (connection->status() == DBConnection::Ready) {
            const QString authority = response->property("authority").toString();
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articleQueryToMap(connection,
                                                                                                        authority)));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void ArticleServer::onArticlesFetched(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
        if (connection->status() == DBConnection::Ready) {
            const QString authority = response->property("authority").toString();
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
}

void ArticleServer::onArticleRequestFinished(ArticleRequest *request) {
    if (QHttpResponse *response = getResponse(request)) {
        if (request->status() == ArticleRequest::Ready) {
            const QString authority = response->property("authority").toString();
            writeResponse(response, QHttpResponse::STATUS_OK,
                    QtJson::Json::serialize(articleResultToMap(request->result(), authority)));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void ArticleServer::onReadArticlesDeleted(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
        if (connection->status() == DBConnection::Ready) {
            QVariantMap result;
            result["count"] = connection->numRowsAffected();
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(result));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void ArticleServer::onConnectionFinished(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
        if (connection->status() == DBConnection::Ready) {
            writeResponse(response, QHttpResponse::STATUS_OK);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void ArticleServer::onResponseDone() {
    if (QHttpResponse *response = qobject_cast<QHttpResponse*>(sender())) {
        removeResponse(response);
    }
}
