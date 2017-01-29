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

#include "serversettings.h"
#include "json.h"
#include "requests.h"
#include <QNetworkReply>

ServerSettings* ServerSettings::self = 0;

ServerSettings::ServerSettings() :
    QObject(),
    m_nam(0)
{
}

ServerSettings::~ServerSettings() {
    self = 0;
}

ServerSettings* ServerSettings::instance() {
    return self ? self : self = new ServerSettings;
}

QStringList ServerSettings::categoryNames() {
    return value("categoryNames").toStringList();
}

QString ServerSettings::defaultCategory() {
    return value("defaultCatgegory", tr("Default")).toString();
}

void ServerSettings::setDefaultCategory(const QString &category) {
    if (category != defaultCategory()) {
        setValue("defaultCategory", category);
    }
}

QString ServerSettings::customTransferCommand() {
    return value("customTransferCommand").toString();
}

void ServerSettings::setCustomTransferCommand(const QString &command) {
    if (command != customTransferCommand()) {
        setValue("customTransferCommand", command);
    }
}

bool ServerSettings::customTransferCommandEnabled() {
    return value("customTransferCommandEnabled", false).toBool();
}

void ServerSettings::setCustomTransferCommandEnabled(bool enabled) {
    if (enabled != customTransferCommandEnabled()) {
        setValue("customTransferCommandEnabled", enabled);
    }
}

int ServerSettings::maximumConcurrentTransfers() {
    return value("maximumConcurrentTransfers", 1).toInt();
}

void ServerSettings::setMaximumConcurrentTransfers(int maximum) {
    if (maximum != maximumConcurrentTransfers()) {
        setValue("maximumConcurrentTransfers", maximum);
    }
}

bool ServerSettings::offlineModeEnabled() {
    return value("offlineModeEnabled", false).toBool();
}

void ServerSettings::setOfflineModeEnabled(bool enabled) {
    if (enabled != offlineModeEnabled()) {
        setValue("offlineModeEnabled", enabled);
    }
}

bool ServerSettings::startTransfersAutomatically() {
    return value("startTransfersAutomatically", true).toBool();
}

void ServerSettings::setStartTransfersAutomatically(bool enabled) {
    if (enabled != startTransfersAutomatically()) {
        setValue("startTransfersAutomatically", enabled);
    }
}

QVariant ServerSettings::value(const QString &key, const QVariant &defaultValue) {
    return m_settings.value(key, defaultValue);
}

void ServerSettings::setValue(const QString &key, const QVariant &value) {
    QVariantMap settings;
    settings[key] = value;
    networkAccessManager()->put(buildRequest("/settings", QNetworkAccessManager::PutOperation),
                                             QtJson::Json::serialize(settings));
}

void ServerSettings::load() {
    networkAccessManager()->get(buildRequest("/settings"));
}

QNetworkAccessManager* ServerSettings::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSettingsLoaded(QNetworkReply*)));
    }
    
    return m_nam;
}

void ServerSettings::onSettingsLoaded(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    const QVariantMap settings = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    reply->deleteLater();
    
    if (settings.isEmpty()) {
        return;
    }
    
    if (settings.value("categoryNames") != m_settings.value("categoryNames")) {
        m_settings["categoryNames"] = settings.value("categoryNames");
        emit categoryNamesChanged(categoryNames());
    }
    
    if (settings.value("defaultCategory") != m_settings.value("defaulCategory")) {
        m_settings["defaultCategory"] = settings.value("defaultCategory");
        emit defaultCategoryChanged(defaultCategory());
    }
    
    if (settings.value("customTransferCommand") != m_settings.value("customTransferCommand")) {
        m_settings["customTransferCommand"] = settings.value("customTransferCommand");
        emit customTransferCommandChanged(customTransferCommand());
    }
    
    if (settings.value("customTransferCommandEnabled") != m_settings.value("customTransferCommandEnabled")) {
        m_settings["customTransferCommandEnabled"] = settings.value("customTransferCommandEnabled");
        emit customTransferCommandEnabledChanged(customTransferCommandEnabled());
    }
    
    if (settings.value("maximumConcurrentTransfers") != m_settings.value("maximumConcurrentTransfers")) {
        m_settings["maximumConcurrentTransfers"] = settings.value("maximumConcurrentTransfers");
        emit maximumConcurrentTransfersChanged(maximumConcurrentTransfers());
    }
    
    if (settings.value("offlineModeEnabled") != m_settings.value("offlineModeEnabled")) {
        m_settings["offlineModeEnabled"] = settings.value("offlineModeEnabled");
        emit offlineModeEnabledChanged(offlineModeEnabled());
    }
    
    if (settings.value("startTransfersAutomatically") != m_settings.value("startTransfersAutomatically")) {
        m_settings["startTransfersAutomatically"] = settings.value("startTransfersAutomatically");
        emit startTransfersAutomaticallyChanged(startTransfersAutomatically());
    }
}
