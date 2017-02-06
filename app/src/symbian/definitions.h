/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include <QDesktopServices>
#include <QStringList>

// Home
static const QString ROOT_PATH("C:/");
static const QString HOME_PATH("E:/");
static const QString CARD_PATH("F:/");

// Plugins
static const QStringList PLUGIN_PATHS = QStringList() << QString(ROOT_PATH + "cutenews/plugins/")
                                                      << QString(HOME_PATH + "cutenews/plugins/")
                                                      << QString(CARD_PATH + "cutenews/plugins/");

static const QString LIB_PREFIX;
static const QString LIB_SUFFIX(".qtplugin");

// Icons
static const int ICON_SIZE = 64;

// Cache
static const QString CACHE_AUTHORITY("http://localhost/");
static const QString CACHE_PATH(HOME_PATH + "cutenews/.cache/");

// Database
static const QString DATABASE_PATH;
static const QString DATABASE_NAME("cutenews.db");

// Config
static const QString APP_CONFIG_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.config/cutenews/");
static const QString PLUGIN_CONFIG_PATH(APP_CONFIG_PATH + "plugins/");

// Downloads
static const QString DOWNLOAD_PATH(HOME_PATH + "cutenews/");

// Network
static const int DOWNLOAD_BUFFER_SIZE = 512000;
static const int MAX_CONCURRENT_TRANSFERS = 4;
static const int MAX_REDIRECTS = 8;
static const QByteArray USER_AGENT("Wget/1.13.4 (linux-gnu)");

// Subscriptions
static const QString ALL_ARTICLES_SUBSCRIPTION_ID("all_articles");
static const QString FAVOURITES_SUBSCRIPTION_ID("favourite_articles");

// Version
static const QString VERSION_NUMBER("1.0.0");

#endif // DEFINITIONS_H
