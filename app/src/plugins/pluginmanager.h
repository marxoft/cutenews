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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "feedplugin.h"
#include "feedpluginconfig.h"
#include <QDateTime>

struct FeedPluginPair
{
    FeedPluginPair(FeedPluginConfig *c, FeedPlugin* p) :
        config(c),
        plugin(p)
    {
    }

    FeedPluginConfig *config;
    FeedPlugin *plugin;
};

typedef QList<FeedPluginPair> FeedPluginList;

class PluginManager : public QObject
{
    Q_OBJECT

public:
    ~PluginManager();

    static PluginManager* instance();

    FeedPluginList plugins() const;

    Q_INVOKABLE FeedPluginConfig* getConfig(const QString &id) const;
    Q_INVOKABLE FeedPluginConfig* getConfigForEnclosure(const QString &url) const;
    Q_INVOKABLE FeedPlugin* getPlugin(const QString &id) const;
    Q_INVOKABLE FeedPlugin* getPluginForEnclosure(const QString &url) const;
    
    Q_INVOKABLE EnclosureRequest* enclosureRequest(const QString &url, QObject *parent = 0) const;
    Q_INVOKABLE FeedRequest* feedRequest(const QString &id, QObject *parent = 0) const;

public Q_SLOTS:
    int load();

Q_SIGNALS:
    void loaded(int count);

private:
    PluginManager();
    
    FeedPluginConfig* getConfigForFilePath(const QString &filePath) const;

    static PluginManager *self;

    QDateTime m_lastLoaded;

    FeedPluginList m_plugins;
};

#endif // PLUGINMANAGER_H
