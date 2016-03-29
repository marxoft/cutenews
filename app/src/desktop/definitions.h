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

#include <QRegExp>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

static const int ALL_ARTICLES_SUBSCRIPTION_ID = -1;
static const int FAVOURITES_SUBSCRIPTION_ID = 0;

static const int MAX_CONCURRENT_TRANSFERS = 1;
static const int MAX_REDIRECTS = 8;

static const int ICON_SIZE = 16;

static const QRegExp ILLEGAL_FILENAME_CHARS_RE("[\"\\/@&~=:?#!|<>*^]");

static const QString VERSION_NUMBER("0.7.0");

static const QString WEB_INTERFACE_PATH("/opt/cutenews/webif/");

#if QT_VERSION >= 0x050000
static const QString DATABASE_PATH(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/cuteNews/");
static const QString DOWNLOAD_PATH(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/cuteNews/");
static const QString ICON_PATH(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/cuteNews/icons/");
static const QString STORAGE_PATH(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/cuteNews/");
#else
static const QString DATABASE_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.config/cuteNews/");
static const QString DOWNLOAD_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/cuteNews/");
static const QString ICON_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.config/cuteNews/icons/");
static const QString STORAGE_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.config/cuteNews/");
#endif
static const QStringList PLUGIN_PATHS = QStringList() << "/opt/cutenews/plugins/" << STORAGE_PATH + "plugins/";

#endif // DEFINITIONS_H
