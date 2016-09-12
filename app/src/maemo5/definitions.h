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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

// Home
#if QT_VERSION >= 0x050000
static const QString HOME_PATH(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
#else
static const QString HOME_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
#endif

// Plugins
static const QStringList PLUGIN_PATHS = QStringList() << QString("/opt/cutenews/plugins/")
                                                      << QString(HOME_PATH + "/cutenews/plugins/");

static const QString LIB_PREFIX("lib");
static const QString LIB_SUFFIX(".so");

// Icons
static const QString DEFAULT_ICON("/opt/cutenews/icons/cutenews.png");
static const int ICON_SIZE = 16;

// Cache
static const QString CACHE_PREFIX("http://localhost");
static const QString CACHE_PATH(HOME_PATH + "/cutenews/cache/");

// Database
static const QString DATABASE_PATH(HOME_PATH + "/cutenews/");
static const QString DATABASE_NAME(DATABASE_PATH + "cutenews.db");

// Config
static const QString APP_CONFIG_PATH(HOME_PATH + "/.config/cutenews/");
static const QString PLUGIN_CONFIG_PATH(APP_CONFIG_PATH + "plugins/");

// Downloads
static const QString DOWNLOAD_PATH(HOME_PATH + "/Downloads/cutenews/");

// Network
static const int DOWNLOAD_BUFFER_SIZE = 64000;
static const int MAX_CONCURRENT_TRANSFERS = 4;
static const int MAX_REDIRECTS = 8;
static const QByteArray USER_AGENT("Wget/1.13.4 (linux-gnu)");

// Subscriptions
static const QString ALL_ARTICLES_SUBSCRIPTION_ID("all_articles");
static const QString FAVOURITES_SUBSCRIPTION_ID("favourite_articles");

// Web interface
static const QString WEB_INTERFACE_PATH("/opt/cutenews/webif/");

// Version
static const QString VERSION_NUMBER("1.0.0");

#endif // DEFINITIONS_H
