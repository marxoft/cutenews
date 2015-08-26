/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "subscriptionplugins.h"
#include "definitions.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

QMap<QString, SubscriptionPlugin> SubscriptionPlugins::pluginMap;

SubscriptionPlugins::SubscriptionPlugins(QObject *parent) :
    QObject(parent)
{
}

SubscriptionPlugin SubscriptionPlugins::getPluginFromName(const QString &name) {
    return pluginMap.value(name);
}

QList<SubscriptionPlugin> SubscriptionPlugins::plugins() {
    return pluginMap.values();
}

QStringList SubscriptionPlugins::pluginNames() {
    return pluginMap.keys();
}

QString SubscriptionPlugins::fileName(const QString &pluginName) {
    return getPluginFromName(pluginName).fileName;
}

QString SubscriptionPlugins::filePath(const QString &pluginName) {
    return getPluginFromName(pluginName).filePath;
}

QString SubscriptionPlugins::command(const QString &pluginName) {
    return getPluginFromName(pluginName).command;
}

bool SubscriptionPlugins::enclosureRedirect(const QString &pluginName) {
    return getPluginFromName(pluginName).enclosureRedirect;
}

bool SubscriptionPlugins::hasParams(const QString &pluginName) {
    return getPluginFromName(pluginName).hasParams;
}

void SubscriptionPlugins::load() {
    pluginMap.clear();
    QDir dir;

    foreach (QString path, PLUGIN_PATHS) {
        dir.setPath(path);

        foreach (QString fileName, dir.entryList(QStringList() << "*.plugin", QDir::Files)) {
#ifdef CUTENEWS_DEBUG
            qDebug() << "SubscriptionPlugins::load: Plugin found:" << fileName;
#endif
            QFile file(dir.absoluteFilePath(fileName));
            
            if (!file.open(QIODevice::ReadOnly)) {
#ifdef CUTENEWS_DEBUG
                qDebug() << "SubscriptionPlugins::load: File error:" << file.errorString();
#endif
                continue;
            }
            
            QXmlStreamReader reader(&file);
            SubscriptionPlugin plugin;
            
            while ((!reader.atEnd()) && (!reader.hasError())) {                
                if (reader.name() == "plugin") {
                    const QXmlStreamAttributes attributes = reader.attributes();
                    const QString name = attributes.value("title").toString();
                    const QString command = attributes.value("exec").toString();
            
                    if ((!name.isEmpty()) && (!command.isEmpty())) {
                        plugin.fileName = fileName;
                        plugin.filePath = file.fileName();
                        plugin.name = name;
                        plugin.command = command;
                        plugin.enclosureRedirect = (attributes.hasAttribute("enclosureRedirect"))
                                                   && ((attributes.value("enclosureRedirect") == "true")
                                                   || (attributes.value("enclosureRedirect") == "1"));
                    }                    
                }
                else if (reader.name() == "params") {
                    plugin.hasParams = true;
                    break;
                }
                
                reader.readNextStartElement();
            }
            
            if (!plugin.name.isEmpty()) {
#ifdef CUTENEWS_DEBUG
                qDebug() << "SubscriptionPlugins::load: Adding plugin:" << plugin.name;
#endif
                pluginMap[plugin.name] = plugin;
            }
        }
    }
}
