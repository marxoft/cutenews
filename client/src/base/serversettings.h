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

#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class QNetworkAccessManager;
class QNetworkReply;

class ServerSettings : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QStringList categoryNames READ categoryNames NOTIFY categoryNamesChanged)
    Q_PROPERTY(QString customTransferCommand READ customTransferCommand WRITE setCustomTransferCommand
               NOTIFY customTransferCommandChanged)
    Q_PROPERTY(bool customTransferCommandEnabled READ customTransferCommandEnabled WRITE setCustomTransferCommandEnabled
               NOTIFY customTransferCommandEnabledChanged)
    Q_PROPERTY(QString defaultCategory READ defaultCategory WRITE setDefaultCategory NOTIFY defaultCategoryChanged)
    Q_PROPERTY(int maximumConcurrentTransfers READ maximumConcurrentTransfers WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(bool offlineModeEnabled READ offlineModeEnabled WRITE setOfflineModeEnabled
               NOTIFY offlineModeEnabledChanged)
    Q_PROPERTY(bool startTransfersAutomatically READ startTransfersAutomatically WRITE setStartTransfersAutomatically
               NOTIFY startTransfersAutomaticallyChanged)
    
public:
    ~ServerSettings();
    
    static ServerSettings* instance();
    
    QStringList categoryNames();
    
    QString customTransferCommand();
    bool customTransferCommandEnabled();
    
    QString defaultCategory();
        
    int maximumConcurrentTransfers();
    
    bool offlineModeEnabled();
        
    bool startTransfersAutomatically();
    
    bool updateSubscriptionsOnStartup();
                
    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

public Q_SLOTS:
    void setDefaultCategory(const QString &category);
    
    void setCustomTransferCommand(const QString &command);
    void setCustomTransferCommandEnabled(bool enabled);
    
    void setMaximumConcurrentTransfers(int maximum);
    
    void setOfflineModeEnabled(bool enabled);
    
    void setStartTransfersAutomatically(bool enabled);
                
    void setValue(const QString &key, const QVariant &value);

    void load();

private Q_SLOTS:
    void onSettingsLoaded(QNetworkReply *reply);

Q_SIGNALS:
    void categoryNamesChanged(const QStringList &categories);
    void defaultCategoryChanged(const QString &category);
    void customTransferCommandChanged(const QString &command);
    void customTransferCommandEnabledChanged(bool enabled);
    void error(const QString &errorString);
    void loaded();
    void maximumConcurrentTransfersChanged(int maximum);
    void offlineModeEnabledChanged(bool enabled);
    void startTransfersAutomaticallyChanged(bool enabled);

private:
    ServerSettings();
    
    QNetworkAccessManager* networkAccessManager();
    
    static ServerSettings *self;
    
    QNetworkAccessManager *m_nam;
    
    QVariantMap m_settings;
};
    
#endif // SETTINGS_H
