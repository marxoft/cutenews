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

#ifndef REQUESTS_H
#define REQUESTS_H

#include "settings.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

inline static QNetworkRequest buildRequest(const QString &path, QNetworkAccessManager::Operation operation
                                           = QNetworkAccessManager::GetOperation) {
    QUrl url = Settings::serverAddress();
    url.setPath(path);
    QNetworkRequest request(url);
    
    switch (operation) {
    case QNetworkAccessManager::PostOperation:
    case QNetworkAccessManager::PutOperation:
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        break;
    default:
        break;
    }
    
    if (Settings::serverAuthenticationEnabled()) {
        request.setRawHeader("Authorization", "Basic " + QString("%1:%2").arg(Settings::serverUsername())
                                                                .arg(Settings::serverPassword()).toUtf8().toBase64());
    }
    
    return request;
}

inline static QNetworkRequest buildRequest(const QString &path, const QVariantMap &params,
                                           QNetworkAccessManager::Operation operation
                                           = QNetworkAccessManager::GetOperation) {
    QUrl url = Settings::serverAddress();
    url.setPath(path);
    QMapIterator<QString, QVariant> iterator(params);
#if QT_VERSION >= 0x050000
    QUrlQuery query;
    
    while (iterator.hasNext()) {
        iterator.next();
        query.addQueryItem(iterator.key(), iterator.value().toString());
    }
    
    url.setQuery(query);
#else
    while (iterator.hasNext()) {
        iterator.next();
        url.addQueryItem(iterator.key(), iterator.value().toString());
    }
#endif
    QNetworkRequest request(url);
    
    switch (operation) {
    case QNetworkAccessManager::PostOperation:
    case QNetworkAccessManager::PutOperation:
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        break;
    default:
        break;
    }
    
    if (Settings::serverAuthenticationEnabled()) {
        request.setRawHeader("Authorization", "Basic " + QString("%1:%2").arg(Settings::serverUsername())
                                                                .arg(Settings::serverPassword()).toUtf8().toBase64());
    }
    
    return request;
}

#endif
