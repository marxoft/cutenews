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
    QObject(parent),
    m_enclosures(false),
    m_feeds(true),
    m_version(1)
{
}

QString FeedPluginConfig::displayName() const {
    return m_displayName;
}

QString FeedPluginConfig::filePath() const {
    return m_filePath;
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

bool FeedPluginConfig::supportsEnclosures() const {
    return m_enclosures;
}

QRegExp FeedPluginConfig::enclosureRegExp() const {
    return m_enclosureRegExp;
}

QVariantList FeedPluginConfig::enclosureSettings() const {
    return m_enclosureSettings;
}

bool FeedPluginConfig::supportsFeeds() const {
    return m_feeds;
}

QVariantList FeedPluginConfig::feedSettings() const {
    return m_feedSettings;
}

int FeedPluginConfig::version() const {
    return m_version;
}

bool FeedPluginConfig::load(const QString &filePath) {
    m_filePath = filePath;
    QFile file(filePath);

    if ((!file.exists()) || (!file.open(QFile::ReadOnly))) {
        Logger::log("FeedPluginConfig::load(): Unable to open config file: " + filePath);
        return false;
    }

    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(file.readAll()), ok);
    file.close();

    if (!ok) {
        Logger::log("FeedPluginConfig::load(): Error parsing config file: " + filePath);
        return false;
    }

    const QVariantMap config = v.toMap();

    if (!config.contains("name")) {
        Logger::log("FeedPluginConfig::load(): 'name' parameter is missing");
        return false;
    }
    
    Logger::log("FeedPluginConfig::load(): Config file loaded: " + filePath, Logger::MediumVerbosity);    
    const int slash = filePath.lastIndexOf("/");
    const QString fileName = filePath.mid(slash + 1);
    const int dot = fileName.lastIndexOf(".");
    m_displayName = config.value("name").toString();
    m_id = fileName.left(dot);
    m_pluginType = config.value("type").toString();
    m_enclosures = config.value("supportsEnclosures", false).toBool();
    m_enclosureRegExp = QRegExp(config.value("enclosureRegExp").toString());
    m_enclosureSettings = config.value("enclosureSettings").toList();
    m_feeds = config.value("supportsFeeds", false).toBool();
    m_feedSettings = config.value("feedSettings").toList();
    m_version = qMax(1, config.value("version").toInt());
    
    if (m_pluginType == "qt") {
        m_pluginFilePath = filePath.left(slash + 1) + LIB_PREFIX + m_id + LIB_SUFFIX;
    }
    else {
        m_pluginFilePath = filePath.left(slash + 1) + m_id;
        
        if (!m_pluginType.isEmpty()) {
            m_pluginFilePath.append("." + m_pluginType);
        }
    }
    
    emit changed();
    return true;
}

bool FeedPluginConfig::enclosureIsSupported(const QString &url) const {
    return (!m_enclosureRegExp.isEmpty()) && (m_enclosureRegExp.indexIn(url) == 0);
}
