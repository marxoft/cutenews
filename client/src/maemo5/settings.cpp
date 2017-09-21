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

QString Settings::serverAddress() {
    return value("Server/serverAddress").toString();
}

void Settings::setServerAddress(const QString &address) {
    if (address != serverAddress()) {
        setValue("Server/serverAddress", address);
        
        if (self) {
            emit self->serverAddressChanged(address);
        }
    }
}

bool Settings::serverAuthenticationEnabled() {
    return value("Server/serverAuthenticationEnabled", false).toBool();
}

void Settings::setServerAuthenticationEnabled(bool enabled) {
    if (enabled != serverAuthenticationEnabled()) {
        setValue("Server/serverAuthenticationEnabled", enabled);
        
        if (self) {
            emit self->serverAuthenticationEnabledChanged(enabled);
        }
    }
}

QString Settings::serverPassword() {
    return value("Server/serverPassword").toString();
}

void Settings::setServerPassword(const QString &password) {
    if (password != serverPassword()) {
        setValue("Server/serverPassword", password);
        
        if (self) {
            emit self->serverPasswordChanged(password);
        }
    }
}

QString Settings::serverUsername() {
    return value("Server/serverUsername").toString();
}

void Settings::setServerUsername(const QString &username) {
    if (username != serverUsername()) {
        setValue("Server/serverUsername", username);
        
        if (self) {
            emit self->serverUsernameChanged(username);
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
    return value("Shortcuts/markSubscriptionRead", tr("Shift+R")).toString();
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
    return value("Shortcuts/newContent", ("Ctrl+N")).toString();
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
    return value("Shortcuts/settings", tr("Ctrl+F")).toString();
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
