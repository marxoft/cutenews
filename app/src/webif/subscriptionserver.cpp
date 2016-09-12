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
#include "dbconnection.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "subscriptions.h"
#include "utils.h"

static QVariantMap subscriptionToMap(const DBConnection *connection) {
    QVariantMap subscription;
    subscription["id"] = connection->value(0);
    subscription["description"] = connection->value(1);
    subscription["downloadEnclosures"] = connection->value(2);
    subscription["iconPath"] = connection->value(3);
    subscription["lastUpdated"] = connection->value(4);
    subscription["source"] = QtJson::Json::parse(connection->value(5).toString());
    subscription["sourceType"] = connection->value(6);
    subscription["title"] = connection->value(7);
    subscription["updateInterval"] = connection->value(8);
    subscription["url"] = connection->value(9);
    subscription["unreadArticles"] = connection->value(10);
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
            const QString queryString = Utils::urlQueryToSqlQuery(request->url());
            m_responses.enqueue(response);
            
            if (queryString.isEmpty()) {
                DBConnection::connection(this, SLOT(onSubscriptionsFetched(DBConnection*)))->fetchSubscriptions();
            }
            else {
                DBConnection::connection(this,
                SLOT(onSubscriptionsFetched(DBConnection*)))->fetchSubscriptions(queryString);
            }
            
            return true;
        }
        
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
            const QString source = properties.value("source").toString();
            
            if (!source.isEmpty()) {
                const int sourceType = properties.value("sourceType").toInt();
                const bool downloadEnclosures = properties.value("downloadEnclosures").toBool();
                
                Subscriptions::instance()->create(source, sourceType, downloadEnclosures);
                writeResponse(response, QHttpResponse::STATUS_CREATED);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "update") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString updateId = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!updateId.isEmpty()) {
                Subscriptions::instance()->update(updateId);
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            else {
                Subscriptions::instance()->updateAll();
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            
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
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        m_responses.enqueue(response);
        DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(parts.at(1));
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        
        if (properties.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            m_responses.enqueue(response);
            DBConnection::connection(this,
            SLOT(onSubscriptionFetched(DBConnection*)))->updateSubscription(parts.at(1), properties, true);
        }
        
        return true;
    }
        
    if (request->method() == QHttpRequest::HTTP_DELETE) {
        m_responses.enqueue(response);
        DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->deleteSubscription(parts.at(1));
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void SubscriptionServer::onConnectionFinished(DBConnection *connection) {
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

void SubscriptionServer::onSubscriptionFetched(DBConnection *connection) {
    if (m_responses.isEmpty()) {
        connection->deleteLater();
        return;
    }
    
    QHttpResponse *response = m_responses.dequeue();
    
    if (connection->status() == DBConnection::Ready) {
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(subscriptionToMap(connection)));
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    
    connection->deleteLater();
}

void SubscriptionServer::onSubscriptionsFetched(DBConnection *connection) {
    if (m_responses.isEmpty()) {
        connection->deleteLater();
        return;
    }
    
    QHttpResponse *response = m_responses.dequeue();
    
    if (connection->status() == DBConnection::Ready) {
        QVariantList subscriptions;
        
        while (connection->nextRecord()) {
            subscriptions << subscriptionToMap(connection);
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(subscriptions));
    }
    else {
        writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    }
    
    connection->deleteLater();
}
