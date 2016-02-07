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

#ifndef SUBSCRIPTIONPLUGINS_H
#define SUBSCRIPTIONPLUGINS_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QStringList>

struct SubscriptionPlugin {
    QString fileName;
    QString filePath;
    QString name;
    QString command;
    bool enclosureRedirect;
    bool hasParams;
};

class SubscriptionPlugins : public QObject
{
    Q_OBJECT
    
public:
    explicit SubscriptionPlugins(QObject *parent = 0);
            
    static SubscriptionPlugin getPluginFromName(const QString &name);
    
    static QList<SubscriptionPlugin> plugins();
    
    Q_INVOKABLE static QStringList pluginNames();
    
    Q_INVOKABLE static QString fileName(const QString &pluginName);
    Q_INVOKABLE static QString filePath(const QString &pluginName);
    Q_INVOKABLE static QString command(const QString &pluginName);
    Q_INVOKABLE static bool enclosureRedirect(const QString &pluginName);
    Q_INVOKABLE static bool hasParams(const QString &pluginName);
    
public Q_SLOTS:
    static void load();
    
private:
    static QMap<QString, SubscriptionPlugin> pluginMap;
};

#endif // SUBSCRIPTIONPLUGINS_H
