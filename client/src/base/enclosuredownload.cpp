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
#include <QNetworkReply>

EnclosureDownload::EnclosureDownload(const QVariantMap &properties, QObject *parent) :
    Transfer(Transfer::EnclosureDownload, parent),
    m_customCommandOverrideEnabled(false),
    m_usePlugin(false)
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
    case PluginSettingsRole:
        return pluginSettings();
    case UsePluginRole:
        return usePlugin();
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
    case PluginSettingsRole:
        setPluginSettings(value.toMap());
        return true;
    case UsePluginRole:
        setUsePlugin(value.toBool());
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

bool EnclosureDownload::usePlugin() const {
    return m_usePlugin;
}

void EnclosureDownload::setUsePlugin(bool enabled) {
    QVariantMap properties;
    properties["usePlugin"] = enabled;
    update(properties);
}

void EnclosureDownload::updateUsePlugin(bool enabled) {
    if (enabled != usePlugin()) {
        m_usePlugin = enabled;
        emit usePluginChanged();
        emit dataChanged(this, UsePluginRole);
    }
}

QVariantMap EnclosureDownload::pluginSettings() const {
    return m_pluginSettings;
}

void EnclosureDownload::setPluginSettings(const QVariantMap &settings) {
    QVariantMap properties;
    properties["pluginSettings"] = settings;
    update(properties);
}

void EnclosureDownload::updatePluginSettings(const QVariantMap &settings) {
    m_pluginSettings = settings;
    emit pluginSettingsChanged();
    emit dataChanged(this, PluginSettingsRole);
}

void EnclosureDownload::load(const QVariantMap &properties) {
    Transfer::load(properties);
    
    if (!properties.isEmpty()) {
        updateCategory(properties.value("category").toString());
        updateCustomCommand(properties.value("customCommand").toString());
        updateCustomCommandOverrideEnabled(properties.value("customCommandOverrideEnabled", false).toBool());
        setDownloadPath(properties.value("downloadPath").toString());
        setFileName(properties.value("fileName").toString());
        updateUsePlugin(properties.value("usePlugin", false).toBool());
        updatePluginSettings(properties.value("pluginSettings").toMap());
    }
}
