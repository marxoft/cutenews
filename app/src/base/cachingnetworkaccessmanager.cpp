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

#include "cachingnetworkaccessmanager.h"
#include "definitions.h"
#include "logger.h"
#include "settings.h"
#include <QNetworkDiskCache>
#include <QNetworkRequest>

CachingNetworkAccessManager::CachingNetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

QNetworkReply* CachingNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                          const QNetworkRequest &req, QIODevice *outgoingData) {
    if (op != QNetworkAccessManager::GetOperation) {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }
    
    const QString path = req.url().path();
    
    if (!path.startsWith(CACHE_PATH)) {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }

    QNetworkDiskCache *dc = qobject_cast<QNetworkDiskCache*>(cache());

    if (!dc) {
        dc = new QNetworkDiskCache(this);
        setCache(dc);
    }

    const QString cacheDir = path.left(path.lastIndexOf("/"));

    if (dc->cacheDirectory() != cacheDir) {
        dc->setCacheDirectory(cacheDir);
    }

    const QByteArray url = QByteArray::fromBase64(path.mid(path.lastIndexOf("/") + 1).toUtf8());
    Logger::log("CachingNetworkAccessManager::createRequest(). Retrieving cached URL: " + url, Logger::HighVerbosity);
    QNetworkRequest request(req);
    request.setUrl(QUrl::fromEncoded(url));
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         Settings::offlineModeEnabled() ? QNetworkRequest::AlwaysCache : QNetworkRequest::PreferCache);
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
