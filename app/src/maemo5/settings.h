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
#include <QStringList>
#include <QVariant>

struct Category {
    QString name;
    QString path;
};

class Settings : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QStringList categoryNames READ categoryNames NOTIFY categoriesChanged)
    Q_PROPERTY(QString customTransferCommand READ customTransferCommand WRITE setCustomTransferCommand
               NOTIFY customTransferCommandChanged)
    Q_PROPERTY(bool customTransferCommandEnabled READ customTransferCommandEnabled
               WRITE setCustomTransferCommandEnabled NOTIFY customTransferCommandEnabledChanged)
    Q_PROPERTY(QString defaultCategory READ defaultCategory WRITE setDefaultCategory NOTIFY defaultCategoryChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(bool enableAutomaticScrollingInWidget READ enableAutomaticScrollingInWidget
               WRITE setEnableAutomaticScrollingInWidget NOTIFY enableAutomaticScrollingInWidgetChanged)
    Q_PROPERTY(bool enableJavaScriptInBrowser READ enableJavaScriptInBrowser
               WRITE setEnableJavaScriptInBrowser NOTIFY enableJavaScriptInBrowserChanged)
    Q_PROPERTY(bool eventFeedEnabled READ eventFeedEnabled WRITE setEventFeedEnabled NOTIFY eventFeedEnabledChanged)
    Q_PROPERTY(QString loggerFileName READ loggerFileName WRITE setLoggerFileName NOTIFY loggerFileNameChanged)
    Q_PROPERTY(int loggerVerbosity READ loggerVerbosity WRITE setLoggerVerbosity NOTIFY loggerVerbosityChanged)
    Q_PROPERTY(int maximumConcurrentTransfers READ maximumConcurrentTransfers WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(bool networkProxyAuthenticationEnabled READ networkProxyAuthenticationEnabled
               WRITE setNetworkProxyAuthenticationEnabled NOTIFY networkProxyChanged)
    Q_PROPERTY(bool networkProxyEnabled READ networkProxyEnabled WRITE setNetworkProxyEnabled
               NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyHost READ networkProxyHost WRITE setNetworkProxyHost NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyPassword READ networkProxyPassword WRITE setNetworkProxyPassword
               NOTIFY networkProxyChanged)
    Q_PROPERTY(int networkProxyPort READ networkProxyPort WRITE setNetworkProxyPort NOTIFY networkProxyChanged)
    Q_PROPERTY(int networkProxyType READ networkProxyType WRITE setNetworkProxyType NOTIFY networkProxyChanged)
    Q_PROPERTY(QString networkProxyUsername READ networkProxyUsername WRITE setNetworkProxyUsername
               NOTIFY networkProxyChanged)
    Q_PROPERTY(bool offlineModeEnabled READ offlineModeEnabled WRITE setOfflineModeEnabled
               NOTIFY offlineModeEnabledChanged)
    Q_PROPERTY(bool openArticlesExternallyFromWidget READ openArticlesExternallyFromWidget
               WRITE setOpenArticlesExternallyFromWidget NOTIFY openArticlesExternallyFromWidgetChanged)
    Q_PROPERTY(int readArticleExpiry READ readArticleExpiry WRITE setReadArticleExpiry
               NOTIFY readArticleExpiryChanged)
    Q_PROPERTY(int screenOrientation READ screenOrientation WRITE setScreenOrientation
               NOTIFY screenOrientationChanged)
    Q_PROPERTY(bool startTransfersAutomatically READ startTransfersAutomatically WRITE setStartTransfersAutomatically
               NOTIFY startTransfersAutomaticallyChanged)
    Q_PROPERTY(bool updateSubscriptionsOnStartup READ updateSubscriptionsOnStartup
               WRITE setUpdateSubscriptionsOnStartup NOTIFY updateSubscriptionsOnStartupChanged)
    Q_PROPERTY(QString copyShortcut READ copyShortcut WRITE setCopyShortcut NOTIFY copyShortcutChanged)
    Q_PROPERTY(QString deleteShortcut READ deleteShortcut WRITE setDeleteShortcut NOTIFY deleteShortcutChanged)
    Q_PROPERTY(QString downloadShortcut READ downloadShortcut WRITE setDownloadShortcut
               NOTIFY downloadShortcutChanged)
    Q_PROPERTY(QString editShortcut READ editShortcut WRITE setEditShortcut NOTIFY editShortcutChanged)
    Q_PROPERTY(QString fetchArticleShortcut READ fetchArticleShortcut WRITE setFetchArticleShortcut
               NOTIFY fetchArticleShortcutChanged)
    Q_PROPERTY(QString importSubscriptionsShortcut READ importSubscriptionsShortcut
               WRITE setImportSubscriptionsShortcut NOTIFY importSubscriptionsShortcutChanged)
    Q_PROPERTY(QString markAllArticlesReadShortcut READ markAllArticlesReadShortcut
               WRITE setMarkAllArticlesReadShortcut NOTIFY markAllArticlesReadShortcutChanged)
    Q_PROPERTY(QString markAllSubscriptionsReadShortcut READ markAllSubscriptionsReadShortcut
               WRITE setMarkAllSubscriptionsReadShortcut NOTIFY markAllSubscriptionsReadShortcutChanged)
    Q_PROPERTY(QString markSubscriptionReadShortcut READ markSubscriptionReadShortcut
               WRITE setMarkSubscriptionReadShortcut NOTIFY markSubscriptionReadShortcutChanged)
    Q_PROPERTY(QString newContentShortcut READ newContentShortcut WRITE setNewContentShortcut
               NOTIFY newContentShortcutChanged)
    Q_PROPERTY(QString nextArticleShortcut READ nextArticleShortcut WRITE setNextArticleShortcut
               NOTIFY nextArticleShortcutChanged)
    Q_PROPERTY(QString nextUnreadArticleShortcut READ nextUnreadArticleShortcut WRITE setNextUnreadArticleShortcut
               NOTIFY nextUnreadArticleShortcutChanged)
    Q_PROPERTY(QString openExternallyShortcut READ openExternallyShortcut WRITE setOpenExternallyShortcut
               NOTIFY openExternallyShortcutChanged)
    Q_PROPERTY(QString pauseAllTransfersShortcut READ pauseAllTransfersShortcut WRITE setPauseAllTransfersShortcut
               NOTIFY pauseAllTransfersShortcutChanged)
    Q_PROPERTY(QString pauseTransferShortcut READ pauseTransferShortcut WRITE setPauseTransferShortcut
               NOTIFY pauseTransferShortcutChanged)
    Q_PROPERTY(QString previousArticleShortcut READ previousArticleShortcut WRITE setPreviousArticleShortcut
               NOTIFY previousArticleShortcutChanged)
    Q_PROPERTY(QString reloadShortcut READ reloadShortcut WRITE setReloadShortcut NOTIFY reloadShortcutChanged)
    Q_PROPERTY(QString searchShortcut READ searchShortcut WRITE setSearchShortcut NOTIFY searchShortcutChanged)
    Q_PROPERTY(QString settingsShortcut READ settingsShortcut WRITE setSettingsShortcut
               NOTIFY settingsShortcutChanged)
    Q_PROPERTY(QString showArticleEnclosuresShortcut READ showArticleEnclosuresShortcut
               WRITE setShowArticleEnclosuresShortcut NOTIFY showArticleEnclosuresShortcutChanged)
    Q_PROPERTY(QString startAllTransfersShortcut READ startAllTransfersShortcut WRITE setStartAllTransfersShortcut
               NOTIFY startAllTransfersShortcutChanged)
    Q_PROPERTY(QString startTransferShortcut READ startTransferShortcut WRITE setStartTransferShortcut
               NOTIFY startTransferShortcutChanged)
    Q_PROPERTY(QString toggleArticleFavouriteShortcut READ toggleArticleFavouriteShortcut
               WRITE setToggleArticleFavouriteShortcut NOTIFY toggleArticleFavouriteShortcutChanged)
    Q_PROPERTY(QString toggleArticleReadShortcut READ toggleArticleReadShortcut WRITE setToggleArticleReadShortcut
               NOTIFY toggleArticleReadShortcutChanged)
    Q_PROPERTY(QString transferCategoryShortcut READ transferCategoryShortcut WRITE setTransferCategoryShortcut
               NOTIFY transferCategoryShortcutChanged)
    Q_PROPERTY(QString transferCommandShortcut READ transferCommandShortcut WRITE setTransferCommandShortcut
               NOTIFY transferCommandShortcutChanged)
    Q_PROPERTY(QString transferPriorityShortcut READ transferPriorityShortcut WRITE setTransferPriorityShortcut
               NOTIFY transferPriorityShortcutChanged)
    Q_PROPERTY(QString transfersShortcut READ transfersShortcut WRITE setTransfersShortcut
               NOTIFY transfersShortcutChanged)
    Q_PROPERTY(QString updateAllSubscriptionsShortcut READ updateAllSubscriptionsShortcut
               WRITE setUpdateAllSubscriptionsShortcut NOTIFY updateAllSubscriptionsShortcutChanged)
    Q_PROPERTY(QString updateSubscriptionShortcut READ updateSubscriptionShortcut WRITE setUpdateSubscriptionShortcut
               NOTIFY updateSubscriptionShortcutChanged)
    Q_PROPERTY(bool volumeKeysEnabled READ volumeKeysEnabled WRITE setVolumeKeysEnabled
               NOTIFY volumeKeysEnabledChanged)
    
public:
    ~Settings();
    
    static Settings* instance();
    
    static QStringList categoryNames();
    static QList<Category> categories();
    static void setCategories(const QList<Category> &c);
    
    static QString customTransferCommand();
    static bool customTransferCommandEnabled();
    
    static QString defaultCategory();
    
    static QString downloadPath();
    Q_INVOKABLE static QString downloadPath(const QString &category);
    
    static bool enableAutomaticScrollingInWidget();

    static bool enableJavaScriptInBrowser();
    
    static bool eventFeedEnabled();
    
    static QString loggerFileName();
    static int loggerVerbosity();
    
    static int maximumConcurrentTransfers();
    
    static bool networkProxyAuthenticationEnabled();
    static bool networkProxyEnabled();
    static QString networkProxyHost();
    static QString networkProxyPassword();
    static int networkProxyPort();
    static int networkProxyType();
    static QString networkProxyUsername();
    
    static bool offlineModeEnabled();
    
    static bool openArticlesExternallyFromWidget();
    
    static int readArticleExpiry();

    static int screenOrientation();
    
    static bool startTransfersAutomatically();
    
    static bool updateSubscriptionsOnStartup();

    static QString copyShortcut();
    static QString deleteShortcut();
    static QString downloadShortcut();
    static QString editShortcut();
    static QString fetchArticleShortcut();
    static QString importSubscriptionsShortcut();
    static QString markAllArticlesReadShortcut();
    static QString markAllSubscriptionsReadShortcut();
    static QString markSubscriptionReadShortcut();
    static QString newContentShortcut();
    static QString nextArticleShortcut();
    static QString nextUnreadArticleShortcut();
    static QString openExternallyShortcut();
    static QString pauseAllTransfersShortcut();
    static QString pauseTransferShortcut();
    static QString previousArticleShortcut();
    static QString reloadShortcut();
    static QString searchShortcut();
    static QString settingsShortcut();
    static QString showArticleEnclosuresShortcut();
    static QString startAllTransfersShortcut();
    static QString startTransferShortcut();
    static QString toggleArticleFavouriteShortcut();
    static QString toggleArticleReadShortcut();
    static QString transferCategoryShortcut();
    static QString transferCommandShortcut();
    static QString transferPriorityShortcut();
    static QString transfersShortcut();
    static QString updateAllSubscriptionsShortcut();
    static QString updateSubscriptionShortcut();
    static bool volumeKeysEnabled();
        
    Q_INVOKABLE static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

public Q_SLOTS:
    static void addCategory(const QString &name, const QString &path);
    static void setDefaultCategory(const QString &category);
    static void removeCategory(const QString &name);
    
    static void setCustomTransferCommand(const QString &command);
    static void setCustomTransferCommandEnabled(bool enabled);
    
    static void setDownloadPath(const QString &path);
    
    static void setEnableAutomaticScrollingInWidget(bool enabled);

    static void setEnableJavaScriptInBrowser(bool enabled);
    
    static void setEventFeedEnabled(bool enabled);
    
    static void setLoggerFileName(const QString &fileName);
    static void setLoggerVerbosity(int verbosity);
        
    static void setMaximumConcurrentTransfers(int maximum);
    
    static void setNetworkProxy();
    static void setNetworkProxyAuthenticationEnabled(bool enabled);
    static void setNetworkProxyEnabled(bool enabled);
    static void setNetworkProxyHost(const QString &host);
    static void setNetworkProxyPassword(const QString &password);
    static void setNetworkProxyPort(int port);
    static void setNetworkProxyType(int type);
    static void setNetworkProxyUsername(const QString &username);
    
    static void setOfflineModeEnabled(bool enabled);
    
    static void setOpenArticlesExternallyFromWidget(bool enabled);
    
    static void setReadArticleExpiry(int expiry);
    
    static void setScreenOrientation(int orientation);

    static void setStartTransfersAutomatically(bool enabled);
    
    static void setUpdateSubscriptionsOnStartup(bool enabled);
    
    static void setCopyShortcut(const QString &key);
    static void setDeleteShortcut(const QString &key);
    static void setDownloadShortcut(const QString &key);
    static void setEditShortcut(const QString &key);
    static void setFetchArticleShortcut(const QString &key);
    static void setImportSubscriptionsShortcut(const QString &key);
    static void setMarkAllArticlesReadShortcut(const QString &key);
    static void setMarkAllSubscriptionsReadShortcut(const QString &key);
    static void setMarkSubscriptionReadShortcut(const QString &key);
    static void setNewContentShortcut(const QString &key);
    static void setNextArticleShortcut(const QString &key);
    static void setNextUnreadArticleShortcut(const QString &key);
    static void setOpenExternallyShortcut(const QString &key);
    static void setPauseAllTransfersShortcut(const QString &key);
    static void setPauseTransferShortcut(const QString &key);
    static void setPreviousArticleShortcut(const QString &key);
    static void setReloadShortcut(const QString &key);
    static void setSearchShortcut(const QString &key);
    static void setSettingsShortcut(const QString &key);
    static void setShowArticleEnclosuresShortcut(const QString &key);
    static void setStartAllTransfersShortcut(const QString &key);
    static void setStartTransferShortcut(const QString &key);
    static void setToggleArticleFavouriteShortcut(const QString &key);
    static void setToggleArticleReadShortcut(const QString &key);
    static void setTransferCategoryShortcut(const QString &key);
    static void setTransferCommandShortcut(const QString &key);
    static void setTransferPriorityShortcut(const QString &key);
    static void setTransfersShortcut(const QString &key);
    static void setUpdateAllSubscriptionsShortcut(const QString &key);
    static void setUpdateSubscriptionShortcut(const QString &key);
    static void setVolumeKeysEnabled(bool enabled);
    
    static void setValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void categoriesChanged();
    void defaultCategoryChanged(const QString &category);
    void customTransferCommandChanged(const QString &command);
    void customTransferCommandEnabledChanged(bool enabled);
    void downloadPathChanged(const QString &path);
    void enableAutomaticScrollingInWidgetChanged(bool enabled);
    void enableJavaScriptInBrowserChanged(bool enabled);
    void eventFeedEnabledChanged(bool enabled);
    void loggerFileNameChanged(const QString &fileName);
    void loggerVerbosityChanged(int verbosity);
    void maximumConcurrentTransfersChanged(int maximum);
    void networkProxyChanged();
    void networkProxyEnabledChanged(bool enabled);
    void offlineModeEnabledChanged(bool enabled);
    void openArticlesExternallyFromWidgetChanged(bool enabled);
    void readArticleExpiryChanged(int expiry);
    void screenOrientationChanged(int orientation);
    void startTransfersAutomaticallyChanged(bool enabled);
    void updateSubscriptionsOnStartupChanged(bool enabled);
    void copyShortcutChanged(const QString &key);
    void deleteShortcutChanged(const QString &key);
    void downloadShortcutChanged(const QString &key);
    void editShortcutChanged(const QString &key);
    void fetchArticleShortcutChanged(const QString &key);
    void importSubscriptionsShortcutChanged(const QString &key);
    void markAllArticlesReadShortcutChanged(const QString &key);
    void markAllSubscriptionsReadShortcutChanged(const QString &key);
    void markSubscriptionReadShortcutChanged(const QString &key);
    void newContentShortcutChanged(const QString &key);
    void nextArticleShortcutChanged(const QString &key);
    void nextUnreadArticleShortcutChanged(const QString &key);
    void openExternallyShortcutChanged(const QString &key);
    void pauseAllTransfersShortcutChanged(const QString &key);
    void pauseTransferShortcutChanged(const QString &key);
    void previousArticleShortcutChanged(const QString &key);
    void reloadShortcutChanged(const QString &key);
    void searchShortcutChanged(const QString &key);
    void settingsShortcutChanged(const QString &key);
    void showArticleEnclosuresShortcutChanged(const QString &key);
    void showSubscriptionPropertiesShortcutChanged(const QString &key);
    void startAllTransfersShortcutChanged(const QString &key);
    void startTransferShortcutChanged(const QString &key);
    void toggleArticleFavouriteShortcutChanged(const QString &key);
    void toggleArticleReadShortcutChanged(const QString &key);
    void transferCategoryShortcutChanged(const QString &key);
    void transferCommandShortcutChanged(const QString &key);
    void transferPriorityShortcutChanged(const QString &key);
    void transfersShortcutChanged(const QString &key);
    void updateAllSubscriptionsShortcutChanged(const QString &key);
    void updateSubscriptionShortcutChanged(const QString &key);
    void volumeKeysEnabledChanged(bool enabled);

private:
    Settings();
    
    static Settings *self;
};
    
#endif // SETTINGS_H
