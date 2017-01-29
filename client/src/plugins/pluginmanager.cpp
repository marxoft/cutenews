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

#include "pluginmanager.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include "requests.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

PluginManager* PluginManager::self = 0;

PluginManager::PluginManager() :
    QObject(),
    m_nam(0),
    m_status(Idle)
{
}

PluginManager::~PluginManager() {
    self = 0;
}

PluginManager* PluginManager::instance() {
    return self ? self : self = new PluginManager;
}

QString PluginManager::errorString() const {
    return m_errorString;
}

void PluginManager::setErrorString(const QString &e) {
    m_errorString = e;
    
    if (!e.isEmpty()) {
        emit error(e);
    }
}

PluginManager::Status PluginManager::status() const {
    return m_status;
}

void PluginManager::setStatus(PluginManager::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

FeedPluginList PluginManager::plugins() const {
    return m_plugins;
}

FeedPluginConfig* PluginManager::getConfig(const QString &id) const {
    foreach (FeedPluginConfig *config, m_plugins) {
        if (config->id() == id) {
            Logger::log("PluginManager::getConfig(). PluginFound: " + id, Logger::HighVerbosity);
            return config;
        }
    }
    
    Logger::log("PluginManager::getConfig(). No Plugin found for id" + id, Logger::HighVerbosity);
    return 0;
}

void PluginManager::load() {
    Logger::log("PluginManager::load()", Logger::LowVerbosity);
    
    if (status() == Active) {
        return;
    }
    
    clear();
    setStatus(Active);
    networkAccessManager()->get(buildRequest("/plugins"));
}

void PluginManager::clear() {
    qDeleteAll(m_plugins);
    m_plugins.clear();
}

QNetworkAccessManager* PluginManager::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(onPluginsLoaded(QNetworkReply*)));
    }
    
    return m_nam;
}

void PluginManager::onPluginsLoaded(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        const QVariantList plugins = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toList();
    
        foreach (const QVariant &p, plugins) {
            m_plugins << new FeedPluginConfig(p.toMap(), this);
        }
    
        const int count = m_plugins.size();
        Logger::log(QString("PluginManager::onPluginsLoaded() %1 plugins loaded").arg(count), Logger::LowVerbosity);
        setErrorString(QString());
        setStatus(Ready);
        emit loaded(count);
    }
    else {
        setErrorString(reply->errorString());
        setStatus(Error);
    }
    
    reply->deleteLater();
}
