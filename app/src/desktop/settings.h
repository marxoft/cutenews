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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>

class Settings : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(QByteArray mainWindowGeometry READ mainWindowGeometry WRITE setMainWindowGeometry)
    Q_PROPERTY(QByteArray mainWindowState READ mainWindowState WRITE setMainWindowState)
    Q_PROPERTY(QByteArray mainWindowHorizontalSplitterState READ mainWindowHorizontalSplitterState
               WRITE setMainWindowHorizontalSplitterState)
    Q_PROPERTY(QByteArray mainWindowVerticalSplitterState READ mainWindowVerticalSplitterState
               WRITE setMainWindowVerticalSplitterState)
    Q_PROPERTY(int maximumConcurrentTransfers READ maximumConcurrentTransfers WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(bool networkProxyEnabled READ networkProxyEnabled WRITE setNetworkProxyEnabled
               NOTIFY networkProxyEnabledChanged)
    Q_PROPERTY(QString networkProxyHost READ networkProxyHost WRITE setNetworkProxyHost NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyPassword READ networkProxyPassword WRITE setNetworkProxyPassword
               NOTIFY networkProxyChanged)
    Q_PROPERTY(int networkProxyPort READ networkProxyPort WRITE setNetworkProxyPort NOTIFY networkProxyChanged)
    Q_PROPERTY(int networkProxyType READ networkProxyType WRITE setNetworkProxyType NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyUsername READ networkProxyUsername WRITE setNetworkProxyUsername
               NOTIFY networkProxyChanged)
    Q_PROPERTY(bool startTransfersAutomatically READ startTransfersAutomatically WRITE setStartTransfersAutomatically
               NOTIFY startTransfersAutomaticallyChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(bool updateOnStartup READ updateOnStartup WRITE setUpdateOnStartup NOTIFY updateOnStartupChanged)
    Q_PROPERTY(bool webInterfaceAuthenticationEnabled READ webInterfaceAuthenticationEnabled
               WRITE setWebInterfaceAuthenticationEnabled NOTIFY webInterfaceAuthenticationEnabledChanged)
    Q_PROPERTY(bool webInterfaceEnabled READ webInterfaceEnabled WRITE setWebInterfaceEnabled
               NOTIFY webInterfaceEnabledChanged)
    Q_PROPERTY(QString webInterfacePassword READ webInterfacePassword WRITE setWebInterfacePassword
               NOTIFY webInterfacePasswordChanged)
    Q_PROPERTY(int webInterfacePort READ webInterfacePort WRITE setWebInterfacePort NOTIFY webInterfacePortChanged)
    Q_PROPERTY(QString webInterfaceUsername READ webInterfaceUsername WRITE setWebInterfaceUsername
               NOTIFY webInterfaceUsernameChanged)
    Q_PROPERTY(bool workOffline READ workOffline WRITE setWorkOffline NOTIFY workOfflineChanged)
    
public:
    ~Settings();
    
    static Settings* instance();
    
    static QString downloadPath();
    
    static QByteArray mainWindowGeometry();
    static QByteArray mainWindowState();
    static QByteArray mainWindowHorizontalSplitterState();
    static QByteArray mainWindowVerticalSplitterState();
    
    static int maximumConcurrentTransfers();
    
    static bool networkProxyEnabled();
    static QString networkProxyHost();
    static QString networkProxyPassword();
    static int networkProxyPort();
    static int networkProxyType();
    static QString networkProxyUsername();
        
    static bool startTransfersAutomatically();
    
    static int updateInterval();
    static bool updateOnStartup();
    
    static bool webInterfaceAuthenticationEnabled();
    static bool webInterfaceEnabled();
    static QString webInterfacePassword();
    static int webInterfacePort();
    static QString webInterfaceUsername();
    
    static bool workOffline();
        
    static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

public Q_SLOTS:
    static void setDownloadPath(const QString &path);
    
    static void setMainWindowGeometry(const QByteArray &geometry);
    static void setMainWindowState(const QByteArray &state);
    static void setMainWindowHorizontalSplitterState(const QByteArray &state);
    static void setMainWindowVerticalSplitterState(const QByteArray &state);
        
    static void setMaximumConcurrentTransfers(int maximum);
    
    static void setNetworkProxy();
    static void setNetworkProxyEnabled(bool enabled);
    static void setNetworkProxyHost(const QString &host);
    static void setNetworkProxyPassword(const QString &password);
    static void setNetworkProxyPort(int port);
    static void setNetworkProxyType(int type);
    static void setNetworkProxyUsername(const QString &username);
        
    static void setStartTransfersAutomatically(bool enabled);
    
    static void setUpdateInterval(int interval);
    static void setUpdateOnStartup(bool enabled);
    
    static void setWebInterfaceAuthenticationEnabled(bool enabled);
    static void setWebInterfaceEnabled(bool enabled);
    static void setWebInterfacePassword(const QString &password);
    static void setWebInterfacePort(int port);
    static void setWebInterfaceUsername(const QString &username);
    
    static void setWorkOffline(bool enabled);
        
    static void setValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void downloadPathChanged(const QString &path);
    void maximumConcurrentTransfersChanged(int maximum);
    void networkProxyChanged();
    void networkProxyEnabledChanged(bool enabled);
    void startTransfersAutomaticallyChanged(bool enabled);
    void updateIntervalChanged(int interval);
    void updateOnStartupChanged(bool enabled);
    void webInterfaceAuthenticationEnabledChanged(bool enabled);
    void webInterfaceEnabledChanged(bool enabled);
    void webInterfacePasswordChanged(const QString &password);
    void webInterfacePortChanged(int port);
    void webInterfaceUsernameChanged(const QString &username);
    void workOfflineChanged(bool enabled);

private:
    Settings();
    
    static Settings *self;
};
    
#endif // SETTINGS_H
