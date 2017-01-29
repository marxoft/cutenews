/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "feedpluginconfig.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include <QFile>

FeedPluginConfig::FeedPluginConfig(QObject *parent) :
    QObject(parent)
{
}

FeedPluginConfig::FeedPluginConfig(const QVariantMap &properties, QObject *parent) :
    QObject(parent)
{
    load(properties);
}

QString FeedPluginConfig::displayName() const {
    return m_displayName;
}

QString FeedPluginConfig::filePath() const {
    return m_filePath;
}

bool FeedPluginConfig::handlesEnclosures() const {
    return m_handlesEnclosures;
}

QString FeedPluginConfig::id() const {
    return m_id;
}

QString FeedPluginConfig::pluginFilePath() const {
    return m_pluginFilePath;
}

QString FeedPluginConfig::pluginType() const {
    return m_pluginType;
}

QVariantList FeedPluginConfig::settings() const {
    return m_settings;
}

int FeedPluginConfig::version() const {
    return m_version;
}

void FeedPluginConfig::load(const QVariantMap &properties) {
    m_displayName = properties.value("displayName").toString();
    m_filePath = properties.value("filePath").toString();
    m_handlesEnclosures = properties.value("handlesEnclosures", false).toBool();
    m_id = properties.value("id").toString();
    m_pluginFilePath = properties.value("pluginFilePath").toString();
    m_pluginType = properties.value("pluginType", "qt").toString();
    m_settings = properties.value("settings").toList();
    m_version = properties.value("version", 1).toInt();
    emit changed();
}
