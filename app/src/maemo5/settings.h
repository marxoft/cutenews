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
    Q_PROPERTY(bool enableAutomaticScrollingInWidget READ enableAutomaticScrollingInWidget
               WRITE setEnableAutomaticScrollingInWidget NOTIFY enableAutomaticScrollingInWidgetChanged)
    Q_PROPERTY(int maximumConcurrentTransfers READ maximumConcurrentTransfers WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(bool networkProxyEnabled READ networkProxyEnabled WRITE setNetworkProxyEnabled
               NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyHost READ networkProxyHost WRITE setNetworkProxyHost NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyPassword READ networkProxyPassword WRITE setNetworkProxyPassword
               NOTIFY networkProxyChanged)
    Q_PROPERTY(int networkProxyPort READ networkProxyPort WRITE setNetworkProxyPort NOTIFY networkProxyChanged)
    Q_PROPERTY(int networkProxyType READ networkProxyType WRITE setNetworkProxyType NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyUsername READ networkProxyUsername WRITE setNetworkProxyUsername
               NOTIFY networkProxyChanged)
    Q_PROPERTY(bool openArticlesExternallyFromWidget READ openArticlesExternallyFromWidget
               WRITE setOpenArticlesExternallyFromWidget NOTIFY openArticlesExternallyFromWidgetChanged)
    Q_PROPERTY(bool startTransfersAutomatically READ startTransfersAutomatically WRITE setStartTransfersAutomatically
               NOTIFY startTransfersAutomaticallyChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(bool updateOnStartup READ updateOnStartup WRITE setUpdateOnStartup NOTIFY updateOnStartupChanged)
    Q_PROPERTY(bool workOffline READ workOffline WRITE setWorkOffline NOTIFY workOfflineChanged)
    Q_PROPERTY(QString userInterface READ userInterface WRITE setUserInterface NOTIFY userInterfaceChanged)
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    
public:
    ~Settings();
    
    static Settings* instance();
    
    static QString downloadPath();
    
    static bool enableAutomaticScrollingInWidget();
    
    static int maximumConcurrentTransfers();
    
    static bool networkProxyEnabled();
    static QString networkProxyHost();
    static QString networkProxyPassword();
    static int networkProxyPort();
    static int networkProxyType();
    static QString networkProxyUsername();
    
    static bool openArticlesExternallyFromWidget();
        
    static bool startTransfersAutomatically();
    
    static int updateInterval();
    static bool updateOnStartup();
    static bool workOffline();
    
    static QString userInterface();
    
    static QString viewMode();

public Q_SLOTS:
    static void setDownloadPath(const QString &path);
    
    static void setEnableAutomaticScrollingInWidget(bool enabled);
        
    static void setMaximumConcurrentTransfers(int maximum);
    
    static void setNetworkProxy();
    static void setNetworkProxyEnabled(bool enabled);
    static void setNetworkProxyHost(const QString &host);
    static void setNetworkProxyPassword(const QString &password);
    static void setNetworkProxyPort(int port);
    static void setNetworkProxyType(int type);
    static void setNetworkProxyUsername(const QString &username);
    
    static void setOpenArticlesExternallyFromWidget(bool enabled);
        
    static void setStartTransfersAutomatically(bool enabled);
    
    static void setUpdateInterval(int interval);
    static void setUpdateOnStartup(bool enabled);
    static void setWorkOffline(bool enabled);
    
    static void setUserInterface(const QString &ui);
    
    static void setViewMode(const QString &mode);

    static void setValue(const QString &key, const QVariant &value);
    static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

Q_SIGNALS:
    void downloadPathChanged();
    void enableAutomaticScrollingInWidgetChanged();
    void maximumConcurrentTransfersChanged();
    void networkProxyChanged();
    void openArticlesExternallyFromWidgetChanged();
    void startTransfersAutomaticallyChanged();
    void updateIntervalChanged();
    void updateOnStartupChanged();
    void workOfflineChanged();
    void userInterfaceChanged();
    void viewModeChanged();

private:
    Settings();
    
    static Settings *self;
};
    
#endif // SETTINGS_H
