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

// Config
static const QString APP_CONFIG_PATH(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                     + "/.config/cutenews-client/");

// Network
static const int MAX_CONCURRENT_TRANSFERS = 4;
static const int MAX_REDIRECTS = 8;
static const QByteArray USER_AGENT("Wget/1.13.4 (linux-gnu)");

// Subscriptions
static const QString ALL_ARTICLES_SUBSCRIPTION_ID("all_articles");
static const QString FAVOURITES_SUBSCRIPTION_ID("favourite_articles");

// Version
static const QString VERSION_NUMBER("1.0.0");

#endif // DEFINITIONS_H
