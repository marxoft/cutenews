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

#include "subscriptions.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include "opmlparser.h"
#include "requests.h"
#include "subscription.h"
#include <QFile>
#include <QNetworkReply>

Subscriptions* Subscriptions::self = 0;

Subscriptions::Subscriptions() :
    QObject(),
    m_nam(0),
    m_progress(0),
    m_status(Idle)
{
    m_statusTimer.setInterval(DefaultStatusInterval);
    m_statusTimer.setSingleShot(true);
    
    connect(DBNotify::instance(), SIGNAL(subscriptionsAdded(QVariantList)), this, SLOT(getStatus()));
    connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(getStatus()));
}

Subscriptions::~Subscriptions() {
    self = 0;
}

Subscriptions* Subscriptions::instance() {
    return self ? self : self = new Subscriptions;
}

QString Subscriptions::activeSubscription() const {
    return m_activeSubscription;
}

void Subscriptions::setActiveSubscription(const QString &id) {
    if (id != activeSubscription()) {
        m_activeSubscription = id;
        emit activeSubscriptionChanged(id);
    }
}

int Subscriptions::progress() const {
    return m_progress;
}

void Subscriptions::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

Subscriptions::Status Subscriptions::status() const {
    return m_status;
}

void Subscriptions::setStatus(Subscriptions::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString Subscriptions::statusText() const {
    return m_statusText;
}

void Subscriptions::setStatusText(const QString &t) {
    if (t != statusText()) {
        m_statusText = t;
        emit statusTextChanged(t);
    }
}

void Subscriptions::getStatus(int interval) {
    Logger::log("Subscriptions::getStatus(). Interval: " + QString::number(interval), Logger::LowVerbosity);
    
    if (status() != Active) {
        m_statusTimer.setInterval(interval);
        getStatus();
    }
}

void Subscriptions::getStatus() {
    Logger::log("Subscriptions::getStatus()", Logger::MediumVerbosity);
    networkAccessManager(SLOT(checkStatus(QNetworkReply*)))->get(buildRequest("/subscriptions/status"));
}

void Subscriptions::create(const QString &source, int sourceType, bool downloadEnclosures, int updateInterval) {
    Logger::log("Subscriptions::create(). Source: " + source, Logger::LowVerbosity);
    QVariantMap subscription;
    subscription["source"] = source;
    subscription["sourceType"] = sourceType;
    subscription["downloadEnclosures"] = downloadEnclosures;
    subscription["updateInterval"] = updateInterval;
    DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->addSubscription(subscription);
}

bool Subscriptions::importFromOpml(const QString &fileName, bool downloadEnclosures, int updateInterval) {
    Logger::log("Subscriptions::importFromOpml(). Filename: " + fileName, Logger::LowVerbosity);
    QFile file(fileName);
    OpmlParser parser(&file);
    
    if (!parser.readHead()) {
        return false;
    }
    
    QVariantList subscriptions;
    
    while (parser.readNextSubscription()) {
        if (!parser.xmlUrl().isEmpty()) {
            QVariantMap subscription;
            subscription["description"] = parser.description();
            subscription["downloadEnclosures"] = downloadEnclosures;
            subscription["source"] = parser.xmlUrl();
            subscription["sourceType"] = Subscription::Url;
            subscription["updateInterval"] = updateInterval;
            subscriptions << subscription;
        }
    }
    
    if (subscriptions.isEmpty()) {
        return false;
    }
    
    DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->addSubscriptions(subscriptions);
    return true;
}

void Subscriptions::update(const QString &id) {
    Logger::log("Subscriptions::update(). ID: " + id, Logger::LowVerbosity);
    setStatus(Active);
    QVariantMap params;
    params["id"] = id;
    networkAccessManager(SLOT(checkStatus(QNetworkReply*)))->get(buildRequest("/subscriptions/update", params));
}

void Subscriptions::update(const QStringList &ids) {
    const QString id = ids.join(",");
    Logger::log("Subscriptions::update(). IDs: " + id, Logger::LowVerbosity);
    setStatus(Active);
    QVariantMap params;
    params["id"] = id;
    networkAccessManager(SLOT(checkStatus(QNetworkReply*)))->get(buildRequest("/subscriptions/update", params));
}

bool Subscriptions::updateAll() {
    Logger::log("Subscriptions::updateAll()", Logger::LowVerbosity);
    
    if (status() == Active) {
        return false;
    }
    
    setStatus(Active);
    networkAccessManager(SLOT(checkStatus(QNetworkReply*)))->get(buildRequest("/subscriptions/update"));
    return true;
}

void Subscriptions::cancel() {
    if (status() != Active) {
        return;
    }
    
    networkAccessManager(SLOT(checkStatus(QNetworkReply*)))->get(buildRequest("/subscriptions/cancel"));
}

QNetworkAccessManager* Subscriptions::networkAccessManager(const char *slot) {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }
    else {
        disconnect(m_nam, SIGNAL(finished(QNetworkReply*)), this, 0);
    }
    
    connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, slot);
    return m_nam;
}

void Subscriptions::checkStatus(QNetworkReply *reply) {
    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        setStatusText(tr("Canceled"));
        reply->deleteLater();
        return;
    default:
        setStatus(Error);
        setStatusText(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    const QVariantMap result = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    
    if (!result.isEmpty()) {
        setActiveSubscription(result.value("activeSubscription").toString());
        setProgress(result.value("progress").toInt());
        setStatus(Status(result.value("status").toInt()));
        setStatusText(result.value("statusText").toString());
        
        if (status() == Active) {
            m_statusTimer.start();
        }
    }
    else {
        setStatus(Error);
        setStatusText(tr("Cannot parse status"));
    }
    
    reply->deleteLater();
}

void Subscriptions::onConnectionFinished(DBConnection *connection) {
    connection->deleteLater();
}
