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
    return value("UI/screenOrientation", 0).toInt();
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

QVariant Settings::value(const QString &key, const QVariant &defaultValue) {
    return QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value) {
    QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).setValue(key, value);
}
