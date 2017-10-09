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

#include "feedpluginconfig.h"

class QNetworkAccessManager;
class QNetworkReply;

typedef QList<FeedPluginConfig*> FeedPluginList;

class PluginManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
    
    ~PluginManager();

    static PluginManager* instance();
    
    QString errorString() const;
    
    Status status() const;
    
    FeedPluginList plugins() const;

    Q_INVOKABLE FeedPluginConfig* getConfig(const QString &id) const;
    Q_INVOKABLE FeedPluginConfig* getConfigForArticle(const QString &url) const;
    Q_INVOKABLE FeedPluginConfig* getConfigForEnclosure(const QString &url) const;
    
    Q_INVOKABLE bool articleIsSupported(const QString &url) const;
    Q_INVOKABLE bool enclosureIsSupported(const QString &url) const;

public Q_SLOTS:
    void load();
    void clear();

private Q_SLOTS:
    void onPluginsLoaded(QNetworkReply *reply);

Q_SIGNALS:
    void error(const QString &errorString);
    void loaded(int count);
    void statusChanged(PluginManager::Status s);

private:
    PluginManager();
    
    void setErrorString(const QString &e);
    
    void setStatus(Status s);
    
    QNetworkAccessManager* networkAccessManager();
    
    static PluginManager *self;
    
    QNetworkAccessManager *m_nam;
    
    QString m_errorString;
    
    Status m_status;
    
    FeedPluginList m_plugins;
};

#endif // PLUGINMANAGER_H
