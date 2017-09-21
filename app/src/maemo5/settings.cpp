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

#include "settings.h"
#include "definitions.h"
#include <QSettings>
#include <QNetworkProxy>

Settings* Settings::self = 0;

Settings::Settings() :
    QObject()
{
}

Settings::~Settings() {
    self = 0;
}

Settings* Settings::instance() {
    return self ? self : self = new Settings;
}

QStringList Settings::categoryNames() {
    QSettings settings(APP_CONFIG_PATH + "settings", QSettings::IniFormat);
    settings.beginGroup("Categories");
    QStringList names = settings.childKeys();
    names.prepend(tr("Default"));
    settings.endGroup();
    
    return names;
}

QList<Category> Settings::categories() {
    QList<Category> list;
    QSettings settings(APP_CONFIG_PATH + "settings", QSettings::IniFormat);
    settings.beginGroup("Categories");
    
    foreach (const QString &key, settings.childKeys()) {
        Category category;
        category.name = key;
        category.path = settings.value(key).toString();
        list << category;
    }
    
    settings.endGroup();
    
    return list;
}

void Settings::setCategories(const QList<Category> &c) {
    QSettings settings(APP_CONFIG_PATH + "settings", QSettings::IniFormat);
    settings.remove("Categories");
    settings.beginGroup("Categories");
    
    foreach (const Category &category, c) {
        settings.setValue(category.name, category.path);
    }
    
    settings.endGroup();

    if (self) {
        emit self->categoriesChanged();
    }
}

void Settings::addCategory(const QString &name, const QString &path) {
    if (path != downloadPath(name)) {
        setValue("Categories/" + name, path);

        if (self) {
            emit self->categoriesChanged();
        }
    }
}

void Settings::removeCategory(const QString &name) {
    QSettings settings(APP_CONFIG_PATH + "settings", QSettings::IniFormat);
    settings.beginGroup("Categories");
    
    if (settings.contains(name)) {
        settings.remove(name);

        if (self) {
            emit self->categoriesChanged();
        }
    }
    
    settings.endGroup();
}

QString Settings::defaultCategory() {
    return value("Transfers/defaultCategory", tr("Default")).toString();
}

void Settings::setDefaultCategory(const QString &category) {
    if (category != defaultCategory()) {
        setValue("Transfers/defaultCategory", category);

        if (self) {
            emit self->defaultCategoryChanged(category);
        }
    }
}

QString Settings::customTransferCommand() {
    return value("Transfers/customCommand").toString();
}

void Settings::setCustomTransferCommand(const QString &command) {
    if (command != customTransferCommand()) {
        setValue("Transfers/customCommand", command);

        if (self) {
            emit self->customTransferCommandChanged(command);
        }
    }
}

bool Settings::customTransferCommandEnabled() {
    return value("Transfers/customCommandEnabled", false).toBool();
}

void Settings::setCustomTransferCommandEnabled(bool enabled) {
    if (enabled != customTransferCommandEnabled()) {
        setValue("Transfers/customCommandEnabled", enabled);
        
        if (self) {
            emit self->customTransferCommandEnabledChanged(enabled);
        }
    }
}

QString Settings::downloadPath() {
    QString path = value("Transfers/downloadPath", DOWNLOAD_PATH).toString();

    if (!path.endsWith("/")) {
        path.append("/");
    }
    
    return path;
}

QString Settings::downloadPath(const QString &category) {
    return value("Categories/" + category, downloadPath()).toString();
}

void Settings::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        setValue("Transfers/downloadPath", path);
        
        if (self) {
            emit self->downloadPathChanged(downloadPath());
        }
    }
}

bool Settings::enableAutomaticScrollingInWidget() {
    return value("Widget/automaticScrollingEnabled", true).toBool();
}

void Settings::setEnableAutomaticScrollingInWidget(bool enabled) {
    if (enabled != enableAutomaticScrollingInWidget()) {
        setValue("Widget/automaticScrollingEnabled", enabled);
        
        if (self) {
            emit self->enableAutomaticScrollingInWidgetChanged(enabled);
        }
    }
}

bool Settings::eventFeedEnabled() {
    return QSettings(APP_CONFIG_PATH + "eventfeed", QSettings::IniFormat).value("enabled", false).toBool();
}

void Settings::setEventFeedEnabled(bool enabled) {
    if (enabled != eventFeedEnabled()) {
        QSettings(APP_CONFIG_PATH + "eventfeed", QSettings::IniFormat).setValue("enabled", enabled);
        
        if (self) {
            emit self->eventFeedEnabledChanged(enabled);
        }
    }
}

QString Settings::loggerFileName() {
    return value("Logger/fileName", APP_CONFIG_PATH + "log").toString();
}

void Settings::setLoggerFileName(const QString &fileName) {
    if (fileName != loggerFileName()) {
        setValue("Logger/fileName", fileName);
        
        if (self) {
            emit self->loggerFileNameChanged(fileName);
        }
    }
}

int Settings::loggerVerbosity() {
    return value("Logger/verbosity", 0).toInt();
}

void Settings::setLoggerVerbosity(int verbosity) {
    if (verbosity != loggerVerbosity()) {
        setValue("Logger/verbosity", verbosity);
        
        if (self) {
            emit self->loggerVerbosityChanged(verbosity);
        }
    }
}

int Settings::maximumConcurrentTransfers() {
    return qBound(1, value("Transfers/maximumConcurrentTransfers", 1).toInt(), MAX_CONCURRENT_TRANSFERS);
}

void Settings::setMaximumConcurrentTransfers(int maximum) {
    if (maximum != maximumConcurrentTransfers()) {
        maximum = qBound(1, maximum, MAX_CONCURRENT_TRANSFERS);
        setValue("Transfers/maximumConcurrentTransfers", maximum);
        
        if (self) {
            emit self->maximumConcurrentTransfersChanged(maximum);
        }
    }
}

void Settings::setNetworkProxy() {
    if (!networkProxyEnabled()) {
        QNetworkProxy::setApplicationProxy(QNetworkProxy());
        return;
    }
    
    QNetworkProxy proxy(QNetworkProxy::ProxyType(networkProxyType()), networkProxyHost(), networkProxyPort());
    
    if (networkProxyAuthenticationEnabled()) {
        proxy.setUser(networkProxyUsername());
        proxy.setPassword(networkProxyPassword());
    }
    
    QNetworkProxy::setApplicationProxy(proxy);
}

bool Settings::networkProxyAuthenticationEnabled() {
    return value("Network/networkProxyAuthenticationEnabled", false).toBool();
}

void Settings::setNetworkProxyAuthenticationEnabled(bool enabled) {
    if (enabled != networkProxyAuthenticationEnabled()) {
        setValue("Network/networkProxyAuthenticationEnabled", enabled);

        if (self) {
            emit self->networkProxyChanged();
        }
    }
}

bool Settings::networkProxyEnabled() {
    return value("Network/networkProxyEnabled", false).toBool();
}

void Settings::setNetworkProxyEnabled(bool enabled) {
    if (enabled != networkProxyEnabled()) {
        setValue("Network/networkProxyEnabled", enabled);
        
        if (self) {
            emit self->networkProxyEnabledChanged(enabled);
        }
    }
}

QString Settings::networkProxyHost() {
    return value("Network/networkProxyHost").toString();
}

void Settings::setNetworkProxyHost(const QString &host) {
    if (host != networkProxyHost()) {
        setValue("Network/networkProxyHost", host);
        
        if (self) {
            emit self->networkProxyChanged();
        }
    }
}

QString Settings::networkProxyPassword() {
    return QByteArray::fromBase64(value("Network/networkProxyPassword").toByteArray());
}

void Settings::setNetworkProxyPassword(const QString &password) {
    QByteArray pass = password.toUtf8().toBase64();
    
    if (pass != networkProxyPassword()) {
        setValue("Network/networkProxyPassword", pass);
        
        if (self) {
            emit self->networkProxyChanged();
        }
    }
}

int Settings::networkProxyPort() {
    return value("Network/networkProxyPort", 80).toInt();
}

void Settings::setNetworkProxyPort(int port) {
    if (port != networkProxyPort()) {
        setValue("Network/networkProxyPort", port);
        
        if (self) {
            emit self->networkProxyChanged();
        }
    }
}

int Settings::networkProxyType() {
    return value("Network/networkProxyType", QNetworkProxy::ProxyType(QNetworkProxy::HttpProxy)).toInt();
}

void Settings::setNetworkProxyType(int type) {
    if (type != networkProxyType()) {
        setValue("Network/networkProxyType", type);
        
        if (self) {
            emit self->networkProxyChanged();
        }
    }
}

QString Settings::networkProxyUsername() {
    return value("Network/networkProxyUsername").toString();
}

void Settings::setNetworkProxyUsername(const QString &username) {
    if (username != networkProxyUsername()) {
        setValue("Network/networkProxyUsername", username);
        
        if (self) {
            emit self->networkProxyChanged();
        }
    }
}

bool Settings::offlineModeEnabled() {
    return value("Network/offlineModeEnabled", false).toBool();
}

void Settings::setOfflineModeEnabled(bool enabled) {
    if (enabled != offlineModeEnabled()) {
        setValue("Network/offlineModeEnabled", enabled);
        
        if (self) {
            emit self->offlineModeEnabledChanged(enabled);
        }
    }
}

bool Settings::openArticlesExternallyFromWidget() {
    return value("Widget/openArticlesExternally", false).toBool();
}

void Settings::setOpenArticlesExternallyFromWidget(bool enabled) {
    if (enabled != openArticlesExternallyFromWidget()) {
        setValue("Widget/openArticlesExternally", enabled);
        
        if (self) {
            emit self->openArticlesExternallyFromWidgetChanged(enabled);
        }
    }
}

int Settings::readArticleExpiry() {
    return value("Subscriptions/readArticleExpiry", -1).toInt();
}

void Settings::setReadArticleExpiry(int expiry) {
    if (expiry != readArticleExpiry()) {
        setValue("Subscriptions/readArticleExpiry", expiry);

        if (self) {
            emit self->readArticleExpiryChanged(expiry);
        }
    }
}

int Settings::screenOrientation() {
    return value("UI/screenOrientation", Qt::WA_Maemo5LandscapeOrientation).toInt();
}

void Settings::setScreenOrientation(int orientation) {
    if (orientation != screenOrientation()) {
        setValue("UI/screenOrientation", orientation);
        
        if (self) {
            emit self->screenOrientationChanged(orientation);
        }
    }
}

bool Settings::startTransfersAutomatically() {
    return value("Transfers/startTransfersAutomatically", true).toBool();
}

void Settings::setStartTransfersAutomatically(bool enabled) {
    if (enabled != startTransfersAutomatically()) {
        setValue("Transfers/startTransfersAutomatically", enabled);
        
        if (self) {
            emit self->startTransfersAutomaticallyChanged(enabled);
        }
    }
}

bool Settings::updateSubscriptionsOnStartup() {
    return value("Subscriptions/updateOnStartup", false).toBool();
}

void Settings::setUpdateSubscriptionsOnStartup(bool enabled) {
    if (enabled != updateSubscriptionsOnStartup()) {
        setValue("Subscriptions/updateOnStartup", enabled);
        
        if (self) {
            emit self->updateSubscriptionsOnStartupChanged(enabled);
        }
    }
}

QString Settings::copyShortcut() {
    return value("Shortcuts/copy", tr("C")).toString();
}

void Settings::setCopyShortcut(const QString &key) {
    if (key != copyShortcut()) {
        setValue("Shortcuts/copy", key);
        
        if (self) {
            emit self->copyShortcutChanged(key);
        }
    }
}

QString Settings::deleteShortcut() {
    return value("Shortcuts/delete", tr("Shift+D")).toString();
}

void Settings::setDeleteShortcut(const QString &key) {
    if (key != deleteShortcut()) {
        setValue("Shortcuts/delete", key);
        
        if (self) {
            emit self->deleteShortcutChanged(key);
        }
    }
}

QString Settings::downloadShortcut() {
    return value("Shortcuts/download", tr("D")).toString();
}

void Settings::setDownloadShortcut(const QString &key) {
    if (key != downloadShortcut()) {
        setValue("Shortcuts/download", key);
        
        if (self) {
            emit self->downloadShortcutChanged(key);
        }
    }
}

QString Settings::editShortcut() {
    return value("Shortcuts/edit", tr("E")).toString();
}

void Settings::setEditShortcut(const QString &key) {
    if (key != editShortcut()) {
        setValue("Shortcuts/edit", key);
        
        if (self) {
            emit self->editShortcutChanged(key);
        }
    }
}

QString Settings::importSubscriptionsShortcut() {
    return value("Shortcuts/importSubscriptions", tr("Ctrl+O")).toString();
}

void Settings::setImportSubscriptionsShortcut(const QString &key) {
    if (key != importSubscriptionsShortcut()) {
        setValue("Shortcuts/importSubscriptions", key);
        
        if (self) {
            emit self->importSubscriptionsShortcutChanged(key);
        }
    }
}

QString Settings::markAllArticlesReadShortcut() {
    return value("Shortcuts/markAllArticlessRead", tr("Ctrl+R")).toString();
}

void Settings::setMarkAllArticlesReadShortcut(const QString &key) {
    if (key != markAllArticlesReadShortcut()) {
        setValue("Shortcuts/markAllArticlessRead", key);
        
        if (self) {
            emit self->markAllArticlesReadShortcutChanged(key);
        }
    }
}

QString Settings::markAllSubscriptionsReadShortcut() {
    return value("Shortcuts/markAllSubscriptionsRead", tr("Ctrl+R")).toString();
}

void Settings::setMarkAllSubscriptionsReadShortcut(const QString &key) {
    if (key != markAllSubscriptionsReadShortcut()) {
        setValue("Shortcuts/markAllSubscriptionsRead", key);
        
        if (self) {
            emit self->markAllSubscriptionsReadShortcutChanged(key);
        }
    }
}

QString Settings::markSubscriptionReadShortcut() {
    return value("Shortcuts/markSubscriptionRead", tr("R")).toString();
}

void Settings::setMarkSubscriptionReadShortcut(const QString &key) {
    if (key != markSubscriptionReadShortcut()) {
        setValue("Shortcuts/markSubscriptionRead", key);
        
        if (self) {
            emit self->markSubscriptionReadShortcutChanged(key);
        }
    }
}

QString Settings::newContentShortcut() {
    return value("Shortcuts/newContent", tr("Ctrl+N")).toString();
}

void Settings::setNewContentShortcut(const QString &key) {
    if (key != newContentShortcut()) {
        setValue("Shortcuts/newContent", key);
        
        if (self) {
            emit self->newContentShortcutChanged(key);
        }
    }
}

QString Settings::nextArticleShortcut() {
    return value("Shortcuts/nextArticle", tr("Right")).toString();
}

void Settings::setNextArticleShortcut(const QString &key) {
    if (key != nextArticleShortcut()) {
        setValue("Shortcuts/nextArticle", key);
        
        if (self) {
            emit self->nextArticleShortcutChanged(key);
        }
    }
}

QString Settings::nextUnreadArticleShortcut() {
    return value("Shortcuts/nextUnreadArticle", tr("Shift+Right")).toString();
}

void Settings::setNextUnreadArticleShortcut(const QString &key) {
    if (key != nextUnreadArticleShortcut()) {
        setValue("Shortcuts/nextUnreadArticle", key);
        
        if (self) {
            emit self->nextUnreadArticleShortcutChanged(key);
        }
    }
}

QString Settings::openExternallyShortcut() {
    return value("Shortcuts/openExternally", tr("O")).toString();
}

void Settings::setOpenExternallyShortcut(const QString &key) {
    if (key != openExternallyShortcut()) {
        setValue("Shortcuts/openExternally", key);
        
        if (self) {
            emit self->openExternallyShortcutChanged(key);
        }
    }
}

QString Settings::pauseAllTransfersShortcut() {
    return value("Shortcuts/pauseAllTransfers", tr("Ctrl+P")).toString();
}

void Settings::setPauseAllTransfersShortcut(const QString &key) {
    if (key != pauseAllTransfersShortcut()) {
        setValue("Shortcuts/pauseAllTransfers", key);
        
        if (self) {
            emit self->pauseAllTransfersShortcutChanged(key);
        }
    }
}

QString Settings::pauseTransferShortcut() {
    return value("Shortcuts/pauseTransfer", tr("P")).toString();
}

void Settings::setPauseTransferShortcut(const QString &key) {
    if (key != pauseTransferShortcut()) {
        setValue("Shortcuts/pauseTransfer", key);
        
        if (self) {
            emit self->pauseTransferShortcutChanged(key);
        }
    }
}

QString Settings::previousArticleShortcut() {
    return value("Shortcuts/previousArticle", tr("Left")).toString();
}

void Settings::setPreviousArticleShortcut(const QString &key) {
    if (key != previousArticleShortcut()) {
        setValue("Shortcuts/previousArticle", key);
        
        if (self) {
            emit self->previousArticleShortcutChanged(key);
        }
    }
}

QString Settings::reloadShortcut() {
    return value("Shortcuts/reload", tr("Ctrl+L")).toString();
}

void Settings::setReloadShortcut(const QString &key) {
    if (key != reloadShortcut()) {
        setValue("Shortcuts/reload", key);
        
        if (self) {
            emit self->reloadShortcutChanged(key);
        }
    }
}

QString Settings::searchShortcut() {
    return value("Shortcuts/search", tr("Ctrl+F")).toString();
}

void Settings::setSearchShortcut(const QString &key) {
    if (key != searchShortcut()) {
        setValue("Shortcuts/search", key);
        
        if (self) {
            emit self->searchShortcutChanged(key);
        }
    }
}

QString Settings::settingsShortcut() {
    return value("Shortcuts/settings", tr("Ctrl+S")).toString();
}

void Settings::setSettingsShortcut(const QString &key) {
    if (key != settingsShortcut()) {
        setValue("Shortcuts/settings", key);
        
        if (self) {
            emit self->settingsShortcutChanged(key);
        }
    }
}

QString Settings::showArticleEnclosuresShortcut() {
    return value("Shortcuts/showArticleEnclosures", tr("E")).toString();
}

void Settings::setShowArticleEnclosuresShortcut(const QString &key) {
    if (key != showArticleEnclosuresShortcut()) {
        setValue("Shortcuts/showArticleEnclosures", key);
        
        if (self) {
            emit self->showArticleEnclosuresShortcutChanged(key);
        }
    }
}

QString Settings::startAllTransfersShortcut() {
    return value("Shortcuts/startAllTransfers", tr("Ctrl+S")).toString();
}

void Settings::setStartAllTransfersShortcut(const QString &key) {
    if (key != startAllTransfersShortcut()) {
        setValue("Shortcuts/startAllTransfers", key);
        
        if (self) {
            emit self->startAllTransfersShortcutChanged(key);
        }
    }
}

QString Settings::startTransferShortcut() {
    return value("Shortcuts/startTransfer", tr("S")).toString();
}

void Settings::setStartTransferShortcut(const QString &key) {
    if (key != startTransferShortcut()) {
        setValue("Shortcuts/startTransfer", key);
        
        if (self) {
            emit self->startTransferShortcutChanged(key);
        }
    }
}

QString Settings::toggleArticleFavouriteShortcut() {
    return value("Shortcuts/toggleArticleFavourite", tr("F")).toString();
}

void Settings::setToggleArticleFavouriteShortcut(const QString &key) {
    if (key != toggleArticleFavouriteShortcut()) {
        setValue("Shortcuts/toggleArticleFavourite", key);
        
        if (self) {
            emit self->toggleArticleFavouriteShortcutChanged(key);
        }
    }
}

QString Settings::toggleArticleReadShortcut() {
    return value("Shortcuts/toggleArticleRead", tr("R")).toString();
}

void Settings::setToggleArticleReadShortcut(const QString &key) {
    if (key != toggleArticleReadShortcut()) {
        setValue("Shortcuts/toggleArticleRead", key);
        
        if (self) {
            emit self->toggleArticleReadShortcutChanged(key);
        }
    }
}

QString Settings::transferCategoryShortcut() {
    return value("Shortcuts/transferCategory", tr("C")).toString();
}

void Settings::setTransferCategoryShortcut(const QString &key) {
    if (key != transferCategoryShortcut()) {
        setValue("Shortcuts/transferCategory", key);
        
        if (self) {
            emit self->transferCategoryShortcutChanged(key);
        }
    }
}

QString Settings::transferPriorityShortcut() {
    return value("Shortcuts/transferPriority", tr("T")).toString();
}

void Settings::setTransferPriorityShortcut(const QString &key) {
    if (key != transferPriorityShortcut()) {
        setValue("Shortcuts/transferPriority", key);
        
        if (self) {
            emit self->transferPriorityShortcutChanged(key);
        }
    }
}

QString Settings::transfersShortcut() {
    return value("Shortcuts/transfers", tr("Ctrl+D")).toString();
}

void Settings::setTransfersShortcut(const QString &key) {
    if (key != transfersShortcut()) {
        setValue("Shortcuts/transfers", key);
        
        if (self) {
            emit self->transfersShortcutChanged(key);
        }
    }
}

QString Settings::updateAllSubscriptionsShortcut() {
    return value("Shortcuts/updateAllSubscriptions", tr("Ctrl+U")).toString();
}

void Settings::setUpdateAllSubscriptionsShortcut(const QString &key) {
    if (key != updateAllSubscriptionsShortcut()) {
        setValue("Shortcuts/updateAllSubscriptions", key);
        
        if (self) {
            emit self->updateAllSubscriptionsShortcutChanged(key);
        }
    }
}

QString Settings::updateSubscriptionShortcut() {
    return value("Shortcuts/updateSubscription", tr("U")).toString();
}

void Settings::setUpdateSubscriptionShortcut(const QString &key) {
    if (key != updateSubscriptionShortcut()) {
        setValue("Shortcuts/updateSubscription", key);
        
        if (self) {
            emit self->updateSubscriptionShortcutChanged(key);
        }
    }
}

bool Settings::volumeKeysEnabled() {
    return value("Shortcuts/volumeKeysEnabled", false).toBool();
}

void Settings::setVolumeKeysEnabled(bool enabled) {
    if (enabled != volumeKeysEnabled()) {
        setValue("Shortcuts/volumeKeysEnabled", enabled);

        if (self) {
            emit self->volumeKeysEnabledChanged(enabled);
        }
    }
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) {
    return QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value) {
    QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).setValue(key, value);
}
