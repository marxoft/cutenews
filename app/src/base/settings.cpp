/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

#if (defined MEEGO_EDITION_HARMATTAN) || (SYMBIAN_OS)
QString Settings::activeColor() const {
    return value("Appearance/activeColor", "#0881cb").toString();
}

void Settings::setActiveColor(const QString &color) {
    if (color != activeColor()) {
        setValue("Appearance/activeColor", color);
        emit activeColorChanged();
    }
}

QString Settings::activeColorString() const {
    return value("Appearance/activeColorString", "color7").toString();
}

void Settings::setActiveColorString(const QString &s) {
    if (s != activeColorString()) {
        setValue("Appearance/activeColorString", s);
        emit activeColorStringChanged();
    }
}
#endif

QString Settings::downloadPath() const {
    QString path = value("Transfers/downloadPath", DOWNLOAD_PATH).toString();

    if (!path.endsWith("/")) {
        path.append("/");
    }
    
    return path;
}  

void Settings::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        setValue("Transfers/downloadPath", path);
        emit downloadPathChanged();
    }
}

int Settings::maximumConcurrentTransfers() const {
    return qBound(1, value("Transfers/maximumConcurrentTransfers", 1).toInt(), MAX_CONCURRENT_TRANSFERS);
}

void Settings::setMaximumConcurrentTransfers(int maximum) {
    if (maximum != maximumConcurrentTransfers()) {
        setValue("Transfers/maximumConcurrentTransfers", qBound(1, maximum, MAX_CONCURRENT_TRANSFERS));
        emit maximumConcurrentTransfersChanged();
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

bool Settings::networkProxyEnabled() const {
    return value("Network/networkProxyEnabled", false).toBool();
}

void Settings::setNetworkProxyEnabled(bool enabled) {
    if (enabled != networkProxyEnabled()) {
        setValue("Network/networkProxyEnabled", enabled);
        emit networkProxyChanged();
    }
}

QString Settings::networkProxyHost() const {
    return value("Network/networkProxyHost").toString();
}

void Settings::setNetworkProxyHost(const QString &host) {
    if (host != networkProxyHost()) {
        setValue("Network/networkProxyHost", host);
        emit networkProxyChanged();
    }
}

QString Settings::networkProxyPassword() const {
    return QByteArray::fromBase64(value("Network/networkProxyPassword").toByteArray());
}

void Settings::setNetworkProxyPassword(const QString &password) {
    QByteArray pass = password.toUtf8().toBase64();
    
    if (pass != networkProxyPassword()) {
        setValue("Network/networkProxyPassword", pass);
        emit networkProxyChanged();
    }
}

int Settings::networkProxyPort() const {
    return value("Network/networkProxyPort", 80).toInt();
}

void Settings::setNetworkProxyPort(int port) {
    if (port != networkProxyPort()) {
        setValue("Network/networkProxyPort", port);
        emit networkProxyChanged();
    }
}

int Settings::networkProxyType() const {
    return value("Network/networkProxyType", QNetworkProxy::ProxyType(QNetworkProxy::HttpProxy)).toInt();
}

void Settings::setNetworkProxyType(int type) {
    if (type != networkProxyType()) {
        setValue("Network/networkProxyType", type);
        emit networkProxyChanged();
    }
}

QString Settings::networkProxyUsername() const {
    return value("Network/networkProxyUsername").toString();
}

void Settings::setNetworkProxyUsername(const QString &username) {
    if (username != networkProxyUsername()) {
        setValue("Network/networkProxyUsername", username);
        emit networkProxyChanged();
    }
}

int Settings::screenOrientation() const {
#ifdef Q_WS_MAEMO_5
    return value("Appearance/screenOrientation", Qt::WA_Maemo5LandscapeOrientation).toInt();
#else
    return value("Appearance/screenOrientation", 0).toInt();
#endif
}

void Settings::setScreenOrientation(int orientation) {
    if (orientation != screenOrientation()) {
        setValue("Appearance/screenOrientation", orientation);
        emit screenOrientationChanged();
    }
}

QStringList Settings::searchHistory() const {
    return value("Search/searchHistory").toStringList();
}

void Settings::setSearchHistory(const QStringList &searches) {
    setValue("Search/searchHistory", searches);
    emit searchHistoryChanged();
}

void Settings::addSearch(const QString &query) {
    QStringList searches = searchHistory();
    searches.removeOne(query);
    searches.prepend(query);
    setSearchHistory(searches);
}

void Settings::removeSearch(const QString &query) {
    QStringList searches = searchHistory();
    searches.removeOne(query);
    setSearchHistory(searches);
}

QString Settings::searchOrder() const {
    return value("Search/searchOrder").toString();
}

void Settings::setSearchOrder(const QString &order) {
    if (order != searchOrder()) {
        setValue("Search/searchOrder", order);
        emit searchOrderChanged();
    }
}

bool Settings::startTransfersAutomatically() const {
    return value("Transfers/startTransfersAutomatically", true).toBool();
}

void Settings::setStartTransfersAutomatically(bool enabled) {
    if (enabled != startTransfersAutomatically()) {
        setValue("Transfers/startTransfersAutomatically", enabled);
        emit startTransfersAutomaticallyChanged();
    }
}

int Settings::updateInterval() const {
    return value("Subscriptions/updateInterval", 0).toInt();
}

void Settings::setUpdateInterval(int interval) {
    if (interval != updateInterval()) {
        setValue("Subscriptions/updateInterval", interval);
        emit updateIntervalChanged();
    }
}

bool Settings::updateOnStartup() const {
    return value("Subscriptions/updateOnStartup", false).toBool();
}

void Settings::setUpdateOnStartup(bool enabled) {
    if (enabled != updateOnStartup()) {
        setValue("Subscriptions/updateOnStartup", enabled);
        emit updateOnStartupChanged();
    }
}

bool Settings::workOffline() const {
    return value("Subscriptions/workOffline", false).toBool();
}

void Settings::setWorkOffline(bool enabled) {
    if (enabled != workOffline()) {
        setValue("Subscriptions/workOffline", enabled);
        emit workOfflineChanged();
    }
}

#ifdef Q_WS_MAEMO_5
QString Settings::userInterface() const {
    return value("Appearance/userInterface", QString("touch")).toString();
}

void Settings::setUserInterface(const QString &ui) {
    if (ui != userInterface()) {
        setValue("Appearance/userInterface", ui);
        emit userInterfaceChanged();
    }
}
#endif

QString Settings::viewMode() const {
    return value("Appearance/viewMode", QString("light")).toString();
}

void Settings::setViewMode(const QString &mode) {
    if (mode != viewMode()) {
        setValue("Appearance/viewMode", mode);
        emit viewModeChanged();
    }
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) const {
    return QSettings().value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value) {
    QSettings().setValue(key, value);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Settings::setValue" << key << value;
#endif
}
