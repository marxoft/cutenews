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

#include "subscriptionserver.h"
#include "database.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "subscriptions.h"
#include "utils.h"
#include <QDateTime>
#include <QSqlError>

static QVariantMap subscriptionQueryToMap(const QSqlQuery &query) {
    QVariantMap subscription;
    subscription["id"] = Database::subscriptionId(query);
    subscription["cacheSize"] = Database::subscriptionCacheSize(query);
    subscription["description"] = Database::subscriptionDescription(query);
    subscription["downloadEnclosures"] = Database::subscriptionDownloadEnclosures(query);
    subscription["iconPath"] = Database::subscriptionIconPath(query);
    subscription["lastUpdated"] = Database::subscriptionLastUpdated(query);
    subscription["source"] = Database::subscriptionSource(query);
    subscription["sourceType"] = Database::subscriptionSourceType(query);
    subscription["title"] = Database::subscriptionTitle(query);
    subscription["updateInterval"] = Database::subscriptionUpdateInterval(query);
    subscription["url"] = Database::subscriptionUrl(query);
    subscription["unreadArticles"] = Database::subscriptionUnreadArticles(query);
    return subscription;
}

static void writeResponse(QHttpResponse *response, int responseCode, const QByteArray &data = QByteArray()) {
    response->setHeader("Content-Type", "application/json");
    response->setHeader("Content-Length", QByteArray::number(data.size()));
    response->writeHead(responseCode);
    response->end(data);
}

SubscriptionServer::SubscriptionServer(QObject *parent) :
    QObject(parent)
{
}

bool SubscriptionServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first() != "subscriptions")) {
        return false;
    }
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const int requestId = Utils::createId();
            const QString queryString = Utils::urlQueryToSqlQuery(request->url());
            m_requests.insert(requestId, response);
            
            if (queryString.isEmpty()) {
                Database::fetchSubscriptions(requestId);
            }
            else {
                Database::fetchSubscriptions(queryString, requestId);
            }
            
            connect(Database::instance(), SIGNAL(subscriptionsFetched(QSqlQuery, int)),
                    this, SLOT(onSubscriptionsFetched(QSqlQuery, int)), Qt::UniqueConnection);
            return true;
        }
        
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QString source = properties.value("source").toString();
            
            if (!source.isEmpty()) {
                const int sourceType = properties.value("sourceType").toInt();
                const bool downloadEnclosures = properties.value("downloadEnclosures").toBool();
                
                if (Subscriptions::instance()->create(source, sourceType, downloadEnclosures)) {
                    writeResponse(response, QHttpResponse::STATUS_CREATED);
                    return true;
                }
            }
            
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "update") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const int updateId = Utils::urlQueryItemValue(request->url(), "id", "-1").toInt();
            
            if (updateId > 0) {
                Subscriptions::instance()->update(updateId);
                writeResponse(response, QHttpResponse::STATUS_OK);
                return true;
            }
            
            Subscriptions::instance()->updateAll();
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }
        
        response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        response->end();
        return true;
    }
    
    if (parts.at(1) == "status") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            QVariantMap status;
            status["activeSubscription"] = Subscriptions::instance()->activeSubscription();
            status["progress"] = Subscriptions::instance()->progress();
            status["status"] = Subscriptions::instance()->status();
            status["statusText"] = Subscriptions::instance()->statusText();
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(status));
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "cancel") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            Subscriptions::instance()->cancel();
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    const int subscriptionId = parts.at(1).toInt();
    
    if (subscriptionId == -1) {
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        const int requestId = Utils::createId();
        m_requests.insert(requestId, response);
        Database::fetchSubscription(subscriptionId, requestId);
        connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                this, SLOT(onSubscriptionFetched(QSqlQuery, int)), Qt::UniqueConnection);
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        
        if (properties.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            Database::updateSubscription(subscriptionId, properties);
            writeResponse(response, QHttpResponse::STATUS_OK);
        }
        
        return true;
    }
        
    if (request->method() == QHttpRequest::HTTP_DELETE) {
        Database::deleteSubscription(subscriptionId);
        writeResponse(response, QHttpResponse::STATUS_OK);
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void SubscriptionServer::onSubscriptionFetched(const QSqlQuery &query, int requestId) {
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
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(subscriptionQueryToMap(query)));
    }
    
    m_requests.remove(requestId);
    
    if (m_requests.isEmpty()) {
        disconnect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
    }
}

void SubscriptionServer::onSubscriptionsFetched(QSqlQuery query, int requestId) {
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
        QVariantList subscriptions;
        
        while (query.next()) {
            subscriptions << subscriptionQueryToMap(query);
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(subscriptions));
    }
    
    m_requests.remove(requestId);
    
    if (m_requests.isEmpty()) {
        disconnect(Database::instance(), SIGNAL(subscriptionsFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionsFetched(QSqlQuery, int)));
    }
}
