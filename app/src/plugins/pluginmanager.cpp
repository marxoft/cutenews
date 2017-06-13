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
#include "externalfeedplugin.h"
#include "javascriptfeedplugin.h"
#include "logger.h"
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>

static bool displayNameLessThan(const FeedPluginPair &pair, const FeedPluginPair &other) {
    return QString::localeAwareCompare(pair.config->displayName(), other.config->displayName()) < 0;
}

PluginManager* PluginManager::self = 0;

PluginManager::PluginManager() :
    QObject(),
    m_lastLoaded(QDateTime::fromTime_t(0))
{
}

PluginManager::~PluginManager() {
    self = 0;
}

PluginManager* PluginManager::instance() {
    return self ? self : self = new PluginManager;
}

int PluginManager::count() const {
    return m_plugins.size();
}

FeedPluginList PluginManager::plugins() const {
    return m_plugins;
}

FeedPluginConfig* PluginManager::getConfig(const QString &id) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("PluginManager::getConfig(). Plugin found: " + id, Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("PluginManager::getConfig(). No Plugin found for id" + id, Logger::HighVerbosity);
    return 0;
}

FeedPluginConfig* PluginManager::getConfigForEnclosure(const QString &url) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->enclosureIsSupported(url)) {
            Logger::log("PluginManager::getConfigForEnclosure(). Plugin found for enclosure: " + url,
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("PluginManager::getConfigForEnclosure(). No Plugin found for enclosure " + url, Logger::HighVerbosity);
    return 0;
}

FeedPluginConfig* PluginManager::getConfigForFilePath(const QString &filePath) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("PluginManager::getConfigForFilePath(). Plugin found: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("PluginManager::getConfigForFilePath(). No Plugin found for filePath " + filePath,
                Logger::HighVerbosity);
    return 0;
}

FeedPlugin* PluginManager::getPlugin(const QString &id) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("PluginManager::getPlugin(). Plugin found: " + id, Logger::HighVerbosity);
            return pair.plugin;
        }
    }
    
    Logger::log("PluginManager::getPlugin(). No Plugin found for id " + id, Logger::HighVerbosity);
    return 0;
}

FeedPlugin* PluginManager::getPluginForEnclosure(const QString &url) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->enclosureIsSupported(url)) {
            Logger::log("PluginManager::getPluginForEnclosure(). Plugin found for enclosure: " + url,
                        Logger::HighVerbosity);
            return pair.plugin;
        }
    }
    
    Logger::log("PluginManager::getPluginForEnclosure(). No Plugin found for enclosure " + url, Logger::HighVerbosity);
    return 0;
}

bool PluginManager::enclosureIsSupported(const QString &url) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->enclosureIsSupported(url)) {
            Logger::log("PluginManager::enclosureIsSupported(). Plugin found for enclosure " + url,
                        Logger::HighVerbosity);
            return true;
        }
    }
    
    Logger::log("PluginManager::enclosureIsSupported(). No Plugin found for enclosure " + url, Logger::HighVerbosity);
    return false;
}

EnclosureRequest* PluginManager::enclosureRequest(const QString &url, QObject *parent) const {
    if (FeedPlugin *plugin = getPluginForEnclosure(url)) {
        return plugin->enclosureRequest(parent);
    }

    return 0;
}

FeedRequest* PluginManager::feedRequest(const QString &id, QObject *parent) const {
    if (FeedPlugin *plugin = getPlugin(id)) {
        return plugin->feedRequest(parent);
    }

    return 0;
}

int PluginManager::load() {
    Logger::log("PluginManager::load(): Loading plugins added since "
                + m_lastLoaded.toString(Qt::ISODate), Logger::LowVerbosity);
    int count = 0;
    QDir dir;
    
    foreach (const QString &path, PLUGIN_PATHS) {
        dir.setPath(path);
        
        foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
            if (info.created() > m_lastLoaded) {
                FeedPluginConfig *config = getConfigForFilePath(info.absoluteFilePath());
                
                if (!config) {
                    config = new FeedPluginConfig(this);
                    
                    if (config->load(info.absoluteFilePath())) {
                        if (config->pluginType() == "qt") {
                            QPluginLoader loader(config->pluginFilePath());
                            QObject *obj = loader.instance();
                            
                            if (obj) {
                                if (FeedPlugin *plugin = qobject_cast<FeedPlugin*>(obj)) {
                                    m_plugins << FeedPluginPair(config, plugin);
                                    ++count;
                                    Logger::log("PluginManager::load(). Qt Plugin loaded: " + config->id(),
                                                Logger::MediumVerbosity);
                                }
                                else {
                                    loader.unload();
                                    Logger::log("PluginManager::load(). Error loading Qt plugin: "
                                                + config->id());
                                }
                            }
                            else {
                                Logger::log("PluginManager::load(). Qt plugin is NULL: " + config->id());
                            }
                        }
                        else if (config->pluginType() == "js") {
                            JavaScriptFeedPlugin *js =
                            new JavaScriptFeedPlugin(config->id(), config->pluginFilePath(), this);
                            m_plugins << FeedPluginPair(config, js);
                            ++count;
                            Logger::log("PluginManager::load(). JavaScript plugin loaded: " + config->id(),
                                        Logger::MediumVerbosity);
                        }
                        else {
                            ExternalFeedPlugin *ext =
                            new ExternalFeedPlugin(config->id(), config->pluginFilePath(), this);
                            m_plugins << FeedPluginPair(config, ext);
                            ++count;
                            Logger::log("PluginManager::load(). External plugin loaded: " + config->id(),
                                        Logger::MediumVerbosity);
                        }
                    }
                    else {
                        delete config;
                    }
                }
            }
            else {
                break;
            }
        }
    }

    Logger::log(QString("PluginManager::load() %1 plugins loaded").arg(count), Logger::LowVerbosity);

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
        emit loaded(count);
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
