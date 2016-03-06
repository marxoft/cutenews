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
#include "database.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "utils.h"
#include <QDateTime>
#include <QSqlError>

static QVariantMap articleQueryToMap(const QSqlQuery &query) {
    QVariantMap article;
    article["id"] = Database::articleId(query);
    article["author"] = Database::articleAuthor(query);
    article["body"] = Database::articleBody(query);
    article["categories"] = Database::articleCategories(query);
    article["date"] = Database::articleDate(query);
    article["enclosures"] = Database::articleEnclosures(query);
    article["favourite"] = Database::articleIsFavourite(query);
    article["read"] = Database::articleIsRead(query);
    article["subscriptionId"] = Database::articleSubscriptionId(query);
    article["title"] = Database::articleTitle(query);
    article["url"] = Database::articleUrl(query);
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
            const int requestId = Utils::createId();
            const QString queryString = Utils::urlQueryToSqlQuery(request->url());
            m_requests.insert(requestId, response);
            
            if (queryString.isEmpty()) {
                Database::fetchArticles(requestId);
            }
            else {
                Database::fetchArticles(queryString, requestId);
            }
            
            connect(Database::instance(), SIGNAL(articlesFetched(QSqlQuery, int)),
                    this, SLOT(onArticlesFetched(QSqlQuery, int)), Qt::UniqueConnection);
            return true;
        }
               
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    const int articleId = parts.at(1).toInt();
    
    if (articleId == -1) {
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        const int requestId = Utils::createId();
        m_requests.insert(requestId, response);
            Database::fetchArticle(articleId, requestId);
        connect(Database::instance(), SIGNAL(articleFetched(QSqlQuery, int)),
                this, SLOT(onArticleFetched(QSqlQuery, int)), Qt::UniqueConnection);
        return true;
    }

    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();

        if ((!properties.contains("read")) && (!properties.contains("favourite"))) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            if (properties.contains("read")) {
                Database::markArticleRead(articleId, properties.value("read").toBool());
            }

            if (properties.contains("favourite")) {
                Database::markArticleFavourite(articleId, properties.value("favourite").toBool());
            }

            writeResponse(response, QHttpResponse::STATUS_OK);
        }
        
        return true;
    }

    if (request->method() == QHttpRequest::HTTP_DELETE) {
        Database::deleteArticle(articleId);
        writeResponse(response, QHttpResponse::STATUS_OK);
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void ArticleServer::onArticleFetched(const QSqlQuery &query, int requestId) {
    if (!m_requests.contains(requestId)) {
        return;
    }
    
    QHttpResponse *response = m_requests.value(requestId);
    
    if (!response) {
        return;
    }
    
    const QSqlError error = query.lastError();
    
    if (error.isValid()) {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articleQueryToMap(query)));
    }
    
    m_requests.remove(requestId);
    
    if (m_requests.isEmpty()) {
        disconnect(Database::instance(), SIGNAL(articleFetched(QSqlQuery, int)),
                   this, SLOT(onArticleFetched(QSqlQuery, int)));
    }
}

void ArticleServer::onArticlesFetched(QSqlQuery query, int requestId) {
    if (!m_requests.contains(requestId)) {
        return;
    }
    
    QHttpResponse *response = m_requests.value(requestId);
    
    if (!response) {
        return;
    }
    
    const QSqlError error = query.lastError();
    
    if (error.isValid()) {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    else {
        QVariantList articles;
        
        while (query.next()) {
            articles << articleQueryToMap(query);
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(articles));
    }
    
    m_requests.remove(requestId);
    
    if (m_requests.isEmpty()) {
        disconnect(Database::instance(), SIGNAL(articlesFetched(QSqlQuery, int)),
                   this, SLOT(onArticlesFetched(QSqlQuery, int)));
    }
}
