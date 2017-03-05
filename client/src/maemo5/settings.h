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
    
    Q_PROPERTY(QString loggerFileName READ loggerFileName WRITE setLoggerFileName NOTIFY loggerFileNameChanged)
    Q_PROPERTY(int loggerVerbosity READ loggerVerbosity WRITE setLoggerVerbosity NOTIFY loggerVerbosityChanged)
    Q_PROPERTY(int readArticleExpiry READ readArticleExpiry WRITE setReadArticleExpiry NOTIFY readArticleExpiryChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(bool serverAuthenticationEnabled READ serverAuthenticationEnabled WRITE setServerAuthenticationEnabled
               NOTIFY serverAuthenticationEnabledChanged)
    Q_PROPERTY(QString serverPassword READ serverPassword WRITE setServerPassword NOTIFY serverPasswordChanged)
    Q_PROPERTY(QString serverUsername READ serverUsername WRITE setServerUsername NOTIFY serverUsernameChanged)
    Q_PROPERTY(QString copyShortcut READ copyShortcut WRITE setCopyShortcut NOTIFY copyShortcutChanged)
    Q_PROPERTY(QString deleteShortcut READ deleteShortcut WRITE setDeleteShortcut NOTIFY deleteShortcutChanged)
    Q_PROPERTY(QString downloadShortcut READ downloadShortcut WRITE setDownloadShortcut NOTIFY downloadShortcutChanged)
    Q_PROPERTY(QString editShortcut READ editShortcut WRITE setEditShortcut NOTIFY editShortcutChanged)
    Q_PROPERTY(QString importSubscriptionsShortcut READ importSubscriptionsShortcut WRITE setImportSubscriptionsShortcut
               NOTIFY importSubscriptionsShortcutChanged)
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
    Q_PROPERTY(QString settingsShortcut READ settingsShortcut WRITE setSettingsShortcut NOTIFY settingsShortcutChanged)
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
    Q_PROPERTY(QString transferPriorityShortcut READ transferPriorityShortcut WRITE setTransferPriorityShortcut
               NOTIFY transferPriorityShortcutChanged)
    Q_PROPERTY(QString transfersShortcut READ transfersShortcut WRITE setTransfersShortcut
               NOTIFY transfersShortcutChanged)
    Q_PROPERTY(QString updateAllSubscriptionsShortcut READ updateAllSubscriptionsShortcut
               WRITE setUpdateAllSubscriptionsShortcut NOTIFY updateAllSubscriptionsShortcutChanged)
    Q_PROPERTY(QString updateSubscriptionShortcut READ updateSubscriptionShortcut WRITE setUpdateSubscriptionShortcut
               NOTIFY updateSubscriptionShortcutChanged)
    
public:
    ~Settings();
    
    static Settings* instance();
    
    static QString loggerFileName();
    static int loggerVerbosity();
    
    static int readArticleExpiry();
    
    static QString serverAddress();
    static bool serverAuthenticationEnabled();
    static QString serverPassword();
    static QString serverUsername();
    
    static QString copyShortcut();
    static QString deleteShortcut();
    static QString downloadShortcut();
    static QString editShortcut();
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
    static QString transferPriorityShortcut();
    static QString transfersShortcut();
    static QString updateAllSubscriptionsShortcut();
    static QString updateSubscriptionShortcut();
            
    Q_INVOKABLE static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

public Q_SLOTS:
    static void setLoggerFileName(const QString &fileName);
    static void setLoggerVerbosity(int verbosity);
        
    static void setReadArticleExpiry(int expiry);
    
    static void setServerAddress(const QString &address);
    static void setServerAuthenticationEnabled(bool enabled);
    static void setServerPassword(const QString &password);
    static void setServerUsername(const QString &username);
    
    static void setCopyShortcut(const QString &key);
    static void setDeleteShortcut(const QString &key);
    static void setDownloadShortcut(const QString &key);
    static void setEditShortcut(const QString &key);
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
    static void setTransferPriorityShortcut(const QString &key);
    static void setTransfersShortcut(const QString &key);
    static void setUpdateAllSubscriptionsShortcut(const QString &key);
    static void setUpdateSubscriptionShortcut(const QString &key);
        
    static void setValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void loggerFileNameChanged(const QString &fileName);
    void loggerVerbosityChanged(int verbosity);
    void readArticleExpiryChanged(int expiry);
    void serverAddressChanged(const QString &address);
    void serverAuthenticationEnabledChanged(bool enabled);
    void serverPasswordChanged(const QString &password);
    void serverUsernameChanged(const QString &username);
    void copyShortcutChanged(const QString &key);
    void deleteShortcutChanged(const QString &key);
    void downloadShortcutChanged(const QString &key);
    void editShortcutChanged(const QString &key);
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
    void startAllTransfersShortcutChanged(const QString &key);
    void startTransferShortcutChanged(const QString &key);
    void toggleArticleFavouriteShortcutChanged(const QString &key);
    void toggleArticleReadShortcutChanged(const QString &key);
    void transferCategoryShortcutChanged(const QString &key);
    void transferPriorityShortcutChanged(const QString &key);
    void transfersShortcutChanged(const QString &key);
    void updateAllSubscriptionsShortcutChanged(const QString &key);
    void updateSubscriptionShortcutChanged(const QString &key);

private:
    Settings();
    
    static Settings *self;
};
    
#endif // SETTINGS_H
