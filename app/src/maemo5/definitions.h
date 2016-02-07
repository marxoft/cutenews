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

static const int ALL_ARTICLES_SUBSCRIPTION_ID = -1;
static const int FAVOURITES_SUBSCRIPTION_ID = 0;

static const int MAX_CONCURRENT_TRANSFERS = 1;
static const int MAX_REDIRECTS = 8;

static const int ICON_SIZE = 48;

static const QRegExp ILLEGAL_FILENAME_CHARS_RE("[\"\\/@&~=:?#!|<>*^]");

static const QString VERSION_NUMBER("0.5.0");

static const QString DATABASE_PATH("/home/user/.config/cuteNews/");
static const QString DOWNLOAD_PATH("/home/user/MyDocs/cuteNews/");
static const QString ICON_PATH("/home/user/.config/cuteNews/icons/");
static const QString STORAGE_PATH("/home/user/.config/cuteNews/");
static const QStringList PLUGIN_PATHS = QStringList() << "/opt/cutenews/plugins/"
                                                      << "/home/user/.config/cuteNews/plugins/";

#endif // DEFINITIONS_H
