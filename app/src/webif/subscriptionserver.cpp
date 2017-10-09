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
#include "serverresponse.h"
#include "subscription.h"
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

SubscriptionServer::SubscriptionServer(QObject *parent) :
    QObject(parent)
{
}

bool SubscriptionServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first().compare("subscriptions",
        Qt::CaseInsensitive) != 0)) {
        return false;
    }
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const int offset = Utils::urlQueryItemValue(request->url(), "offset", "0").toInt();
            const int limit = Utils::urlQueryItemValue(request->url(), "limit", "0").toInt();
            DBConnection *connection = DBConnection::connection(this, SLOT(onSubscriptionsFetched(DBConnection*)));
            addResponse(connection, response);
            connection->fetchSubscriptions(offset, limit);
            return true;
        }
        
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantList list = QtJson::Json::parse(request->body()).toList();
            QVariantList subscriptions;
            
            foreach (const QVariant &v, list) {
                const QVariantMap properties = v.toMap();
                const QString source = properties.value("source").toString();
            
                if (!source.isEmpty()) {
                    const int sourceType = properties.value("sourceType", Subscription::Url).toInt();
                    const bool downloadEnclosures = properties.value("downloadEnclosures", false).toBool();
                    const int updateInterval = properties.value("updateInterval", 0).toInt();
                    const QString id = Subscriptions::instance()->create(source, sourceType, downloadEnclosures,
                                                                         updateInterval);
                
                    QVariantMap subscription;
                    subscription["description"] = QString();
                    subscription["downloadEnclosures"] = downloadEnclosures;
                    subscription["iconPath"] = QString();
                    subscription["id"] = id;
                    subscription["lastUpdated"] = QDateTime();
                    subscription["source"] = source;
                    subscription["sourceType"] = sourceType;
                    subscription["title"] = tr("New subscription");
                    subscription["unreadArticles"] = 0;
                    subscription["updateInterval"] = updateInterval;
                    subscription["url"] = QString();
                    subscriptions << subscription;
                }
            }
            
            if (!subscriptions.isEmpty()) {
                writeResponse(response, QHttpResponse::STATUS_CREATED, QtJson::Json::serialize(subscriptions));
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1).compare("update", Qt::CaseInsensitive) == 0) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!id.isEmpty()) {
                Subscriptions::instance()->update(id);
            }
            else {
                Subscriptions::instance()->updateAll();
            }
            
            QVariantMap status;
            status["activeSubscription"] = Subscriptions::instance()->activeSubscription();
            status["progress"] = Subscriptions::instance()->progress();
            status["status"] = Subscriptions::instance()->status();
            status["statusText"] = Subscriptions::instance()->statusText();
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(status));
            return true;
        }
        
        response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        response->end();
        return true;
    }
    
    if (parts.at(1).compare("status", Qt::CaseInsensitive) == 0) {
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
    
    if (parts.at(1).compare("cancel", Qt::CaseInsensitive) == 0) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            Subscriptions::instance()->cancel();
            writeResponse(response, QHttpResponse::STATUS_OK);
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1).compare("read", Qt::CaseInsensitive) == 0) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!id.isEmpty()) {
                DBConnection *connection = DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)));
                addResponse(connection, response);
                connection->markSubscriptionRead(id, true, true);
            }
            else {
                DBConnection *connection = DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)));
                addResponse(connection, response);
                connection->markAllSubscriptionsRead();
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1).compare("unread", Qt::CaseInsensitive) == 0) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!id.isEmpty()) {
                DBConnection *connection = DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)));
                addResponse(connection, response);
                connection->markSubscriptionRead(id, false, true);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        DBConnection *connection = DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)));
        addResponse(connection, response);
        connection->fetchSubscription(parts.at(1));
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        
        if (properties.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        }
        else {
            DBConnection *connection = DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)));
            addResponse(connection, response);
            connection->updateSubscription(parts.at(1), properties, true);
        }
        
        return true;
    }
        
    if (request->method() == QHttpRequest::HTTP_DELETE) {
        DBConnection *connection = DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)));
        addResponse(connection, response);
        connection->deleteSubscription(parts.at(1));
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void SubscriptionServer::addResponse(DBConnection *connection, QHttpResponse *response) {
    m_responses.insert(connection, response);
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

QHttpResponse* SubscriptionServer::getResponse(DBConnection *connection) {
    return m_responses.value(connection);
}

void SubscriptionServer::removeResponse(QHttpResponse *response) {
    if (DBConnection *connection = m_responses.key(response)) {
        m_responses.remove(connection);
        connection->deleteLater();
        disconnect(response, 0, this, 0);
    }
}

void SubscriptionServer::onConnectionFinished(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
        if (connection->status() == DBConnection::Ready) {
            writeResponse(response, QHttpResponse::STATUS_OK);
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void SubscriptionServer::onSubscriptionFetched(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
        if (connection->status() == DBConnection::Ready) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(subscriptionToMap(connection)));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void SubscriptionServer::onSubscriptionsFetched(DBConnection *connection) {
    if (QHttpResponse *response = getResponse(connection)) {
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
    }
}

void SubscriptionServer::onResponseDone() {
    if (QHttpResponse *response = qobject_cast<QHttpResponse*>(sender())) {
        removeResponse(response);
    }
}
