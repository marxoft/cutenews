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

QByteArray Settings::articlesHeaderViewState() {
    return value("UI/articlesHeaderViewState").toByteArray();
}

void Settings::setArticlesHeaderViewState(const QByteArray &state) {
    setValue("UI/articlesHeaderViewState", state);
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

bool Settings::enableJavaScriptInBrowser() {
    return value("Browser/javascriptEnabled", true).toBool();
}

void Settings::setEnableJavaScriptInBrowser(bool enabled) {
    if (enabled != enableJavaScriptInBrowser()) {
        setValue("Browser/javascriptEnabled", enabled);

        if (self) {
            emit self->enableJavaScriptInBrowserChanged(enabled);
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

QByteArray Settings::mainWindowGeometry() {
    return value("UI/windowGeometry").toByteArray();
}

void Settings::setMainWindowGeometry(const QByteArray &geometry) {
    setValue("UI/windowGeometry", geometry);
}

QByteArray Settings::mainWindowState() {
    return value("UI/windowState").toByteArray();
}

void Settings::setMainWindowState(const QByteArray &state) {
    setValue("UI/windowState", state);
}

QByteArray Settings::mainWindowHorizontalSplitterState() {
    return value("UI/horizontalSplitterState").toByteArray();
}

void Settings::setMainWindowHorizontalSplitterState(const QByteArray &state) {
    setValue("UI/horizontalSplitterState", state);
}

QByteArray Settings::mainWindowVerticalSplitterState() {
    return value("UI/verticalSplitterState").toByteArray();
}

void Settings::setMainWindowVerticalSplitterState(const QByteArray &state) {
    setValue("UI/verticalSplitterState", state);
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

QByteArray Settings::transfersHeaderViewState() {
    return value("UI/transfersHeaderViewState").toByteArray();
}

void Settings::setTransfersHeaderViewState(const QByteArray &state) {
    setValue("UI/transfersHeaderViewState", state);
}

bool Settings::webInterfaceAuthenticationEnabled() {
    return value("WebInterface/webInterfaceAuthenticationEnabled", false).toBool();
}

void Settings::setWebInterfaceAuthenticationEnabled(bool enabled) {
    if (enabled != webInterfaceAuthenticationEnabled()) {
        setValue("WebInterface/webInterfaceAuthenticationEnabled", enabled);
        
        if (self) {
            emit self->webInterfaceAuthenticationEnabledChanged(enabled);
        }
    }
}

bool Settings::webInterfaceEnabled() {
    return value("WebInterface/webInterfaceEnabled", false).toBool();
}

void Settings::setWebInterfaceEnabled(bool enabled) {
    if (enabled != webInterfaceEnabled()) {
        setValue("WebInterface/webInterfaceEnabled", enabled);
        
        if (self) {
            emit self->webInterfaceEnabledChanged(enabled);
        }
    }
}

QString Settings::webInterfacePassword() {
    return value("WebInterface/webInterfacePassword").toString();
}

void Settings::setWebInterfacePassword(const QString &password) {
    if (password != webInterfacePassword()) {
        setValue("WebInterface/webInterfacePassword", password);
        
        if (self) {
            emit self->webInterfacePasswordChanged(password);
        }
    }
}

int Settings::webInterfacePort() {
    return value("WebInterface/webInterfacePort", 8080).toInt();
}

void Settings::setWebInterfacePort(int port) {
    if (port != webInterfacePort()) {
        setValue("WebInterface/webInterfacePort", port);
        
        if (self) {
            emit self->webInterfacePortChanged(port);
        }
    }
}

QString Settings::webInterfaceUsername() {
    return value("WebInterface/webInterfaceUsername").toString();
}

void Settings::setWebInterfaceUsername(const QString &username) {
    if (username != webInterfaceUsername()) {
        setValue("WebInterface/webInterfaceUsername", username);
        
        if (self) {
            emit self->webInterfaceUsernameChanged(username);
        }
    }
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) {
    return QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value) {
    QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).setValue(key, value);
}
