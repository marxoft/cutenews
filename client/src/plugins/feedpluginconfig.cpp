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

FeedPluginConfig::FeedPluginConfig(QObject *parent) :
    QObject(parent),
    m_articles(false),
    m_enclosures(false),
    m_feeds(false),
    m_version(1)
{
}

FeedPluginConfig::FeedPluginConfig(const QVariantMap &config, QObject *parent) :
    QObject(parent),
    m_articles(false),
    m_enclosures(false),
    m_feeds(false),
    m_version(1)
{
    load(config);
}

QString FeedPluginConfig::displayName() const {
    return m_displayName;
}

QString FeedPluginConfig::id() const {
    return m_id;
}

QString FeedPluginConfig::pluginType() const {
    return m_pluginType;
}

bool FeedPluginConfig::supportsArticles() const {
    return m_articles;
}

QRegExp FeedPluginConfig::articleRegExp() const {
    return m_articleRegExp;
}

QVariantList FeedPluginConfig::articleSettings() const {
    return m_articleSettings;
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

void FeedPluginConfig::load(const QVariantMap &config) {
    m_displayName = config.value("displayName").toString();
    m_id = config.value("id").toString();
    m_pluginType = config.value("pluginType").toString();
    m_articles = config.value("supportsArticles", false).toBool();
    m_articleRegExp = QRegExp(config.value("articleRegExp").toString());
    m_articleSettings = config.value("articleSettings").toList();
    m_enclosures = config.value("supportsEnclosures", false).toBool();
    m_enclosureRegExp = QRegExp(config.value("enclosureRegExp").toString());
    m_enclosureSettings = config.value("enclosureSettings").toList();
    m_feeds = config.value("supportsFeeds", false).toBool();
    m_feedSettings = config.value("feedSettings").toList();
    m_version = qMax(1, config.value("version").toInt());
    emit changed();
}

bool FeedPluginConfig::articleIsSupported(const QString &url) const {
    return (!m_articleRegExp.isEmpty()) && (m_articleRegExp.indexIn(url) == 0);
}

bool FeedPluginConfig::enclosureIsSupported(const QString &url) const {
    return (!m_enclosureRegExp.isEmpty()) && (m_enclosureRegExp.indexIn(url) == 0);
}
