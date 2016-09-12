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
#include "logger.h"
#include "settings.h"
#include "utils.h"
#include <QNetworkAccessManager>
#include <QSettings>

Transfers* Transfers::self = 0;

Transfers::Transfers() :
    QObject(),
    m_nam(new QNetworkAccessManager(this))
{
    m_queueTimer.setSingleShot(true);
    m_queueTimer.setInterval(1000);
    
    connect(&m_queueTimer, SIGNAL(timeout()), this, SLOT(startNextTransfers()));
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

void Transfers::addEnclosureDownload(const QString &url, const QString &subscriptionId) {
    Logger::log(QString("Transfers::addEnclosureDownload(). URL: %1, Subscription ID: %2").arg(url).arg(subscriptionId),
                Logger::LowVerbosity);
    EnclosureDownload *transfer = new EnclosureDownload(this);
    transfer->setNetworkAccessManager(m_nam);
    transfer->setId(Utils::createId());
    transfer->setDownloadPath(Settings::downloadPath() + ".incomplete/" + transfer->id());
    transfer->setFileName(url.mid(url.lastIndexOf("/") + 1));
    transfer->setName(transfer->fileName());
    transfer->setSubscriptionId(subscriptionId);
    transfer->setUrl(url);
    connect(transfer, SIGNAL(statusChanged()), this, SLOT(onTransferStatusChanged()));
    
    m_transfers << transfer;
    emit countChanged(count());
    emit transferAdded(transfer);
    
    if (Settings::startTransfersAutomatically()) {
        transfer->queue();
    }
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
    foreach (Transfer *transfer, m_transfers) {
        transfer->queue();
    }
    
    return true;
}

bool Transfers::pause() {
    foreach (Transfer *transfer, m_transfers) {
        transfer->pause();
    }
    
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

void Transfers::save() {
    QSettings settings(APP_CONFIG_PATH + "transfers", QSettings::IniFormat);
    settings.clear();
    settings.beginWriteArray("transfers");
    
    for (int i = 0; i < m_transfers.size(); i++) {
        const Transfer *transfer = m_transfers.at(i);
        settings.setArrayIndex(i);
        settings.setValue("category", transfer->property("category"));
        settings.setValue("customCommand", transfer->property("customCommand"));
        settings.setValue("customCommandOverrideEnabled", transfer->property("customCommandOverrideEnabled"));
        settings.setValue("downloadPath", transfer->property("downloadPath"));
        settings.setValue("fileName", transfer->property("fileName"));
        settings.setValue("id", transfer->property("id"));
        settings.setValue("name", transfer->property("name"));
        settings.setValue("priority", transfer->property("priority"));
        settings.setValue("size", transfer->property("size"));
        settings.setValue("subscriptionId", transfer->property("subscriptionId"));
        settings.setValue("transferType", transfer->property("transferType"));
        settings.setValue("url", transfer->property("url"));
    }
    
    settings.endArray();
    Logger::log(QString("Transfers::save(). %1 transfers saved").arg(m_transfers.size()), Logger::LowVerbosity);
}

void Transfers::load() {
    QSettings settings(APP_CONFIG_PATH  + "transfers", QSettings::IniFormat);
    const int size = settings.beginReadArray("transfers");
    Logger::log(QString("Transfers::load(). Loading %1 transfers").arg(size), Logger::LowVerbosity);

    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        Transfer *transfer;

        switch (settings.value("transferType").toInt()) {
        case Transfer::Download:
            transfer = new Download(this);
            break;
        default:
            transfer = new EnclosureDownload(this);
            break;
        }
        
        transfer->setProperty("category", settings.value("category"));
        transfer->setProperty("customCommand", settings.value("customCommand"));
        transfer->setProperty("customCommandOverrideEnabled", transfer->property("customCommandOverrideEnabled"));
        transfer->setProperty("downloadPath", settings.value("downloadPath"));
        transfer->setProperty("fileName", settings.value("fileName"));
        transfer->setProperty("id", settings.value("id"));
        transfer->setProperty("name", settings.value("name"));
        transfer->setProperty("priority", settings.value("priority"));
        transfer->setProperty("size", settings.value("size"));
        transfer->setProperty("subscriptionId", settings.value("subscriptionId"));
        transfer->setProperty("url", settings.value("url"));
        connect(transfer, SIGNAL(statusChanged()), this, SLOT(onTransferStatusChanged()));
    
        m_transfers << transfer;
        emit countChanged(count());
        emit transferAdded(transfer);
    
        if (Settings::startTransfersAutomatically()) {
            transfer->queue();
        }
    }
    
    settings.endArray();
}

void Transfers::getNextTransfers() {
    const int max = Settings::maximumConcurrentTransfers();
    
    for (int priority = Transfer::HighPriority; priority <= Transfer::LowPriority; priority++) {
        foreach (Transfer *transfer, m_transfers) {
            if ((transfer->priority() == priority) && (transfer->status() == Transfer::Queued)) {
                if (active() < max) {
                    addActiveTransfer(transfer);
                }
                else {
                    return;
                }
            }
        }
    }
}

void Transfers::startNextTransfers() {
    getNextTransfers();
    
    foreach (Transfer *transfer, m_active) {
        transfer->start();
    }
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

void Transfers::onTransferStatusChanged() {
    if (Transfer *transfer = qobject_cast<Transfer*>(sender())) {
        switch (transfer->status()) {
        case Transfer::Paused:
            removeActiveTransfer(transfer);
            break;
        case Transfer::Failed:
            removeActiveTransfer(transfer);
            save();
            break;
        case Transfer::Canceled:
        case Transfer::Completed:
            removeTransfer(transfer);
            save();
            break;
        case Transfer::Queued:
            break;
        default:
            return;
        }
                
        if (active() < Settings::maximumConcurrentTransfers()) {
            m_queueTimer.start();
        }
    }
}

void Transfers::setMaximumConcurrentTransfers(int maximum) {
    int act = active();
    
    if (act < maximum) {
        startNextTransfers();
    }
    else if (act > maximum) {
        for (int priority = Transfer::LowPriority; priority >= Transfer::HighPriority; priority--) {
            for (int i = m_active.size() - 1; i >= 0; i--) {
                if (m_active.at(i)->priority() == priority) {
                    m_active.at(i)->pause();
                    act--;
                
                    if (act == maximum) {
                        return;
                    }
                }
            }
        }
    }
}
