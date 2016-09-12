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

PluginManager::PluginManager(QObject *parent) :
    QObject(parent),
    m_lastLoaded(QDateTime::fromTime_t(0))
{
}

PluginManager::~PluginManager() {
    self = 0;
}

PluginManager* PluginManager::instance() {
    return self ? self : self = new PluginManager;
}

FeedPluginList PluginManager::plugins() const {
    return m_plugins;
}

FeedPluginConfig* PluginManager::getConfig(const QString &id) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("PluginManager::getConfig(). PluginFound: " + id, Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("PluginManager::getConfigForFeed(). No Plugin found for id" + id, Logger::HighVerbosity);
    return 0;
}

FeedPluginConfig* PluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("PluginManager::getConfigByFilePath(). PluginFound: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("PluginManager::getConfigByFilePath(). No Plugin found for filePath " + filePath,
                Logger::HighVerbosity);
    return 0;
}

FeedPlugin* PluginManager::getPlugin(const QString &id) const {
    foreach (const FeedPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("PluginManager::getPlugin(). PluginFound: " + id, Logger::HighVerbosity);
            return pair.plugin;
        }
    }
    
    Logger::log("PluginManager::getPlugin(). No Plugin found for id " + id, Logger::HighVerbosity);
    return 0;
}

EnclosureRequest* PluginManager::enclosureRequest(const QString &id, QObject *parent) const {
    if (FeedPlugin *plugin = getPlugin(id)) {
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
    Logger::log("PluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate), Logger::LowVerbosity);
    int count = 0;
    QDir dir;
    
    foreach (const QString &path, PLUGIN_PATHS) {
        dir.setPath(path);
        
        foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
            if (info.lastModified() > m_lastLoaded) {
                FeedPluginConfig *config = getConfigByFilePath(info.absoluteFilePath());
                
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
