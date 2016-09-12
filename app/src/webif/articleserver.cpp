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
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "utils.h"

static QVariantMap articleQueryToMap(const DBConnection *connection) {
    QVariantMap article;
    article["id"] = connection->value(0);
    article["author"] = connection->value(1);
    article["body"] = connection->value(2);
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
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString queryString = Utils::urlQueryToSqlQuery(request->url());
            m_responses.enqueue(response);
            
            if (queryString.isEmpty()) {
                DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->fetchArticles();
            }
            else {
                DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->fetchArticles(queryString);
            }
            
            return true;
        }
               
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }

    if (parts.size() == 2) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            if (parts.at(1) == "read") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                m_responses.enqueue(response);
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleRead(id, true, true);
            }
            else if (parts.at(1) == "unread") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                m_responses.enqueue(response);
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleRead(id, false, true);
            }
            else if (parts.at(1) == "favourite") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                m_responses.enqueue(response);
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleFavourite(id, true, true);
            }
            else if (parts.at(1) == "unfavourite") {
                const QString id = Utils::urlQueryItemValue(request->url(), "id");
                m_responses.enqueue(response);
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->markArticleFavourite(id, false, true);
            }
            else {
                m_responses.enqueue(response);
                DBConnection::connection(this, SLOT(onArticleFetched(DBConnection*)))->fetchArticle(parts.at(1));
            }

            return true;
        }

        if (request->method() == QHttpRequest::HTTP_DELETE) {
            m_responses.enqueue(response);
            DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->deleteArticle(parts.at(1));
            return true;
        }
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void ArticleServer::onArticleFetched(DBConnection *connection) {
    if (m_responses.isEmpty()) {
        connection->deleteLater();
        return;
    }
    
    QHttpResponse *response = m_responses.dequeue();
    
    if (connection->status() == DBConnection::Ready) {
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articleQueryToMap(connection)));
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    
    connection->deleteLater();
}

void ArticleServer::onArticlesFetched(DBConnection *connection) {
    if (m_responses.isEmpty()) {
        connection->deleteLater();
        return;
    }
    
    QHttpResponse *response = m_responses.dequeue();
    
    if (connection->status() == DBConnection::Ready) {
        QVariantList articles;
        
        while (connection->nextRecord()) {
            articles << articleQueryToMap(connection);
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articles));
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    
    connection->deleteLater();
}

void ArticleServer::onConnectionFinished(DBConnection *connection) {
    if (m_responses.isEmpty()) {
        connection->deleteLater();
        return;
    }
    
    QHttpResponse *response = m_responses.dequeue();
    
    if (connection->status() == DBConnection::Ready) {
        writeResponse(response, QHttpResponse::STATUS_OK);
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    
    connection->deleteLater();
}
