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

#include "transfers.h"
#include "definitions.h"
#include "download.h"
#include "enclosuredownload.h"
#include "json.h"
#include "logger.h"
#include "requests.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

Transfers* Transfers::self = 0;

Transfers::Transfers() :
    QObject(),
    m_nam(0),
    m_status(Idle)
{
}

Transfers::~Transfers() {
    self = 0;
}

Transfers* Transfers::instance() {
    return self ? self : self = new Transfers;
}

int Transfers::active() const {
    return m_active.size();
}

int Transfers::count() const {
    return m_transfers.size();
}

QString Transfers::errorString() const {
    return m_errorString;
}

void Transfers::setErrorString(const QString &e) {
    m_errorString = e;
    
    if (!e.isEmpty()) {
        emit error(e);
    }
}

Transfers::Status Transfers::status() const {
    return m_status;
}

void Transfers::setStatus(Transfers::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

void Transfers::addEnclosureDownload(const QString &url, const QString &category, int priority) {
    Logger::log("Transfers::addEnclosureDownload(). URL: " + url, Logger::LowVerbosity);
    
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QVariantMap properties;
    properties["url"] = url;
    properties["category"] = category;
    properties["priority"] = priority;
    QNetworkReply *reply =
        networkAccessManager()->post(buildRequest("/transfers", QNetworkAccessManager::PostOperation),
                                                  QtJson::Json::serialize(QVariantList() << properties));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersLoaded()));
}

Transfer* Transfers::get(int i) const {
    if ((i >= 0) && (i < m_transfers.size())) {
        return m_transfers.at(i);
    }
    
    return 0;
}

Transfer* Transfers::get(const QString &id) const {
    foreach (Transfer *transfer, m_transfers) {
        if (transfer->id() == id) {
            return transfer;
        }
    }
    
    return 0;
}

bool Transfers::start() {
    if (status() == Active) {
        return false;
    }
    
    setStatus(Active);
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/start"));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersChanged()));
    return true;
}

bool Transfers::pause() {
    if (status() == Active) {
        return false;
    }
    
    setStatus(Active);
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/pause"));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersChanged()));
    return true;
}

bool Transfers::start(const QString &id) {
    if (Transfer *transfer = get(id)) {
        transfer->queue();
        return true;
    }
    
    return false;
}

bool Transfers::pause(const QString &id) {
    if (Transfer *transfer = get(id)) {
        transfer->pause();
        return true;
    }
    
    return false;
}

bool Transfers::cancel(const QString &id) {
    if (Transfer *transfer = get(id)) {
        transfer->cancel();
        return true;
    }
    
    return false;
}

void Transfers::load() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    clear();
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers"));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersLoaded()));
}

void Transfers::clear() {
    qDeleteAll(m_transfers);
    m_transfers.clear();
    m_active.clear();
    emit countChanged(0);
    emit activeChanged(0);
}

void Transfers::removeTransfer(Transfer *transfer) {
    removeActiveTransfer(transfer);
    m_transfers.removeOne(transfer);
    transfer->deleteLater();
    emit countChanged(count());
}

void Transfers::addActiveTransfer(Transfer *transfer) {
    m_active << transfer;
    emit activeChanged(active());
}

void Transfers::removeActiveTransfer(Transfer *transfer) {
    m_active.removeOne(transfer);
    emit activeChanged(active());
}

QNetworkAccessManager* Transfers::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }
    
    return m_nam;
}

void Transfers::onTransfersChanged() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(reply->errorString());
        setStatus(Error);
    }
    
    reply->deleteLater();
}

void Transfers::onTransfersLoaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        const QVariantList list = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toList();
        
        foreach (const QVariant &v, list) {
            const QVariantMap result = v.toMap();
            const Transfer::TransferType type = Transfer::TransferType(result.value("transferType", Transfer::Download)
                                                                       .toInt());
            Transfer *transfer;
            
            switch (type) {
            case Transfer::EnclosureDownload:
                transfer = new EnclosureDownload(result, this);
                break;
            default:
                transfer = new Download(result, this);
                break;
            }
            
            transfer->setNetworkAccessManager(networkAccessManager());
            m_transfers << transfer;
            
            switch (transfer->status()) {
            case Transfer::Connecting:
            case Transfer::Downloading:
            case Transfer::Uploading:
            case Transfer::ExecutingCustomCommand:
                addActiveTransfer(transfer);
                break;
            default:
                break;
            }
        }
        
        setErrorString(QString());
        setStatus(Ready);
        emit countChanged(count());
    }
    else {
        setErrorString(reply->errorString());
        setStatus(Error);
    }
    
    reply->deleteLater();
}
