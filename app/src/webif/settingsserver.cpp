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

#include "settingsserver.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include "settings.h"
#include <QMetaObject>
#include <QMetaProperty>

SettingsServer::SettingsServer(QObject *parent) :
    QObject(parent)
{
}

bool SettingsServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    if (request->path() != "/settings") {
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        QVariantMap settings;
        const QMetaObject mo = Settings::staticMetaObject;
        
        
        for (int i = mo.propertyOffset(); i < mo.propertyCount(); i++) {
            const QMetaProperty property = mo.property(i);
            settings[property.name()] = property.read(Settings::instance());
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(settings));
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        const QByteArray body = request->body();
        const QVariantMap settings = QtJson::Json::parse(QString::fromUtf8(body)).toMap();
        
        if (settings.isEmpty()) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            return true;
        }
        
        QMapIterator<QString, QVariant> iterator(settings);
        
        while (iterator.hasNext()) {
            iterator.next();
            
            if (!Settings::instance()->setProperty(iterator.key().toUtf8(), iterator.value())) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
                return true;
            }
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, body);
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
