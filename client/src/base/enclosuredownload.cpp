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

#include "enclosuredownload.h"
#include "json.h"
#include <QNetworkReply>

EnclosureDownload::EnclosureDownload(const QVariantMap &properties, QObject *parent) :
    Transfer(Transfer::EnclosureDownload, parent),
    m_customCommandOverrideEnabled(false)
{
    load(properties);
}

QVariant EnclosureDownload::data(int role) const {
    switch (role) {
    case CategoryRole:
        return category();
    case CustomCommandRole:
        return customCommand();
    case CustomCommandOverrideEnabledRole:
        return customCommandOverrideEnabled();
    case DownloadPathRole:
        return downloadPath();
    case FileNameRole:
        return fileName();
    case SubscriptionIdRole:
        return subscriptionId();
    default:
        return Transfer::data(role);
    }
}

bool EnclosureDownload::setData(int role, const QVariant &value) {
    switch (role) {
    case CategoryRole:
        setCategory(value.toString());
        return true;
    case CustomCommandRole:
        setCustomCommand(value.toString());
        return true;
    case CustomCommandOverrideEnabledRole:
        setCustomCommandOverrideEnabled(value.toBool());
        return true;
    default:
        return Transfer::setData(role, value);
    }
}

QString EnclosureDownload::category() const {
    return m_category;
}

void EnclosureDownload::setCategory(const QString &c) {
    QVariantMap properties;
    properties["category"] = c;
    update(properties);
}

void EnclosureDownload::updateCategory(const QString &c) {
    if (c != category()) {
        m_category = c;
        emit categoryChanged();
        emit dataChanged(this, CategoryRole);
    }
}

QString EnclosureDownload::customCommand() const {
    return m_customCommand;
}

void EnclosureDownload::setCustomCommand(const QString &c) {
    QVariantMap properties;
    properties["customCommand"] = c;
    update(properties);
}

void EnclosureDownload::updateCustomCommand(const QString &c) {
    if (c != customCommand()) {
        m_customCommand = c;
        emit customCommandChanged();
        emit dataChanged(this, CustomCommandRole);
    }
}

bool EnclosureDownload::customCommandOverrideEnabled() const {
    return m_customCommandOverrideEnabled;
}

void EnclosureDownload::setCustomCommandOverrideEnabled(bool enabled) {
    QVariantMap properties;
    properties["customCommandOverrideEnabled"] = enabled;
    update(properties);
}

void EnclosureDownload::updateCustomCommandOverrideEnabled(bool enabled) {
    if (enabled != customCommandOverrideEnabled()) {
        m_customCommandOverrideEnabled = enabled;
        emit customCommandOverrideEnabledChanged();
        emit dataChanged(this, CustomCommandOverrideEnabledRole);
    }
}

QString EnclosureDownload::downloadPath() const {
    return m_downloadPath;
}

void EnclosureDownload::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        m_downloadPath = path;
        emit downloadPathChanged();
        emit dataChanged(this, DownloadPathRole);
    }
}

QString EnclosureDownload::fileName() const {
    return m_fileName;
}

void EnclosureDownload::setFileName(const QString &name) {
    if (name != fileName()) {
        m_fileName = name;
        emit fileNameChanged();
        emit dataChanged(this, FileNameRole);
    }    
}

QString EnclosureDownload::subscriptionId() const {
    return m_subscriptionId;
}

void EnclosureDownload::setSubscriptionId(const QString &i) {
    if (i != subscriptionId()) {
        m_subscriptionId = i;
        emit subscriptionIdChanged();
        emit dataChanged(this, SubscriptionIdRole);
    }
}

void EnclosureDownload::load(const QVariantMap &properties) {
    Transfer::load(properties);
    
    if (!properties.isEmpty()) {
        updateCategory(properties.value("category").toString());
        updateCustomCommand(properties.value("customCommand").toString());
        updateCustomCommandOverrideEnabled(properties.value("customCommandOverrideEnabled", false).toBool());
        setDownloadPath(properties.value("downloadPath").toString());
        setFileName(properties.value("fileName").toString());
        setSubscriptionId(properties.value("subscriptionId").toString());
    }
}
