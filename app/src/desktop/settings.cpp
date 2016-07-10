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
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

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

QString Settings::downloadPath() {
    QString path = value("Transfers/downloadPath", DOWNLOAD_PATH).toString();

    if (!path.endsWith("/")) {
        path.append("/");
    }
    
    return path;
}  

void Settings::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        setValue("Transfers/downloadPath", path);
        
        if (self) {
            emit self->downloadPathChanged(downloadPath());
        }
    }
}

QByteArray Settings::mainWindowGeometry() {
    return value("MainWindow/windowGeometry").toByteArray();
}

void Settings::setMainWindowGeometry(const QByteArray &geometry) {
    setValue("MainWindow/windowGeometry", geometry);
}

QByteArray Settings::mainWindowState() {
    return value("MainWindow/windowState").toByteArray();
}

void Settings::setMainWindowState(const QByteArray &state) {
    setValue("MainWindow/windowState", state);
}

QByteArray Settings::mainWindowHorizontalSplitterState() {
    return value("MainWindow/horizontalSplitterState").toByteArray();
}

void Settings::setMainWindowHorizontalSplitterState(const QByteArray &state) {
    setValue("MainWindow/horizontalSplitterState", state);
}

QByteArray Settings::mainWindowVerticalSplitterState() {
    return value("MainWindow/verticalSplitterState").toByteArray();
}

void Settings::setMainWindowVerticalSplitterState(const QByteArray &state) {
    setValue("MainWindow/verticalSplitterState", state);
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
    
    QNetworkProxy proxy(QNetworkProxy::ProxyType(networkProxyType()), networkProxyHost(), networkProxyPort(),
                        networkProxyUsername(), networkProxyPassword());
    
    QNetworkProxy::setApplicationProxy(proxy);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Settings::setNetworkProxy" << networkProxyType() << networkProxyHost() << networkProxyPort()
                                            << networkProxyUsername() << networkProxyPassword();
#endif
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

int Settings::updateInterval() {
    return value("Subscriptions/updateInterval", 0).toInt();
}

void Settings::setUpdateInterval(int interval) {
    if (interval != updateInterval()) {
        setValue("Subscriptions/updateInterval", interval);
        
        if (self) {
            emit self->updateIntervalChanged(interval);
        }
    }
}

bool Settings::updateOnStartup() {
    return value("Subscriptions/updateOnStartup", false).toBool();
}

void Settings::setUpdateOnStartup(bool enabled) {
    if (enabled != updateOnStartup()) {
        setValue("Subscriptions/updateOnStartup", enabled);
        
        if (self) {
            emit self->updateOnStartupChanged(enabled);
        }
    }
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

bool Settings::workOffline() {
    return value("Subscriptions/workOffline", false).toBool();
}

void Settings::setWorkOffline(bool enabled) {
    if (enabled != workOffline()) {
        setValue("Subscriptions/workOffline", enabled);
        
        if (self) {
            emit self->workOfflineChanged(enabled);
        }
    }
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) {
    return QSettings().value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value) {
    QSettings().setValue(key, value);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Settings::setValue" << key << value;
#endif
}
