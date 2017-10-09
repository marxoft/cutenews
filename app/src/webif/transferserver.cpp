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

#include "transferserver.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include "transfers.h"
#include "utils.h"

static QVariantMap transferToMap(const Transfer *transfer) {
    QVariantMap map;
    QHashIterator<int, QByteArray> iterator(Transfer::roleNames());
    
    while (iterator.hasNext()) {
        iterator.next();
        map[iterator.value()] = transfer->data(iterator.key());
    }
    
    return map;
}

TransferServer::TransferServer(QObject *parent) :
    QObject(parent)
{
}

bool TransferServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.isEmpty()) || (parts.size() > 2) || (parts.first().compare("transfers", Qt::CaseInsensitive) != 0)) {
        return false;
    }
    
    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const int offset = Utils::urlQueryItemValue(request->url(), "offset", "0").toInt();
            int limit = Utils::urlQueryItemValue(request->url(), "limit", "0").toInt();
            
            if ((limit <= 0) || (limit > Transfers::instance()->count())) {
                limit = Transfers::instance()->count();
            }
            
            QVariantList list;
            
            for (int i = offset; i < limit; i++) {
                if (const Transfer *transfer = Transfers::instance()->get(i)) {
                    list << transferToMap(transfer);
                }
                else {
                    break;
                }
            }
            
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(list));
            return true;
        }
        
        if (request->method() == QHttpRequest::HTTP_POST) {
            const QVariantList list = QtJson::Json::parse(request->body()).toList();
            QVariantList transfers;
            
            foreach (const QVariant &v, list) {
                const QVariantMap properties = v.toMap();
                const QString url = properties.value("url").toString();
                
                if (!url.isEmpty()) {
                    const QString category = properties.value("category").toString();
                    const int priority = properties.value("priority", Transfer::NormalPriority).toInt();
                    const bool usePlugin = properties.value("usePlugin", false).toBool();
                    const Transfer *transfer = Transfers::instance()->addEnclosureDownload(url, category, priority,
                                                                                           usePlugin);
                
                    if (transfer) {
                        transfers << transferToMap(transfer);
                    }
                }
            }
            
            if (!transfers.isEmpty()) {
                writeResponse(response, QHttpResponse::STATUS_CREATED, QtJson::Json::serialize(transfers));
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "start") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!id.isEmpty()) {
                if (Transfer *transfer = Transfers::instance()->get(id)) {
                    transfer->queue();
                    writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(transferToMap(transfer)));
                }
                else {
                    writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
                }
            }
            else {
                Transfers::instance()->start();
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "pause") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            
            if (!id.isEmpty()) {
                if (Transfer *transfer = Transfers::instance()->get(id)) {
                    transfer->pause();
                    writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(transferToMap(transfer)));
                }
                else {
                    writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
                }
            }
            else {
                Transfers::instance()->pause();
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (parts.at(1) == "cancel") {
        if (request->method() == QHttpRequest::HTTP_GET) {
            const QString id = Utils::urlQueryItemValue(request->url(), "id");
            
            if ((!id.isEmpty()) && (Transfers::instance()->cancel(id))) {
                writeResponse(response, QHttpResponse::STATUS_OK);
                return true;
            }
            
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            return true;
        }
        
        writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        const Transfer *transfer = Transfers::instance()->get(parts.at(1));
        
        if (transfer) {
            writeResponse(response, QHttpResponse::STATUS_OK,  QtJson::Json::serialize(transferToMap(transfer)));
            return true;
        }
        
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_PUT) {
        Transfer *transfer = Transfers::instance()->get(parts.at(1));
        const QVariantMap properties = QtJson::Json::parse(request->body()).toMap();
        
        if ((!transfer) || (properties.isEmpty())) {
            writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
            return true;
        }
        
        QMapIterator<QString, QVariant> iterator(properties);
        
        while (iterator.hasNext()) {
            iterator.next();
            
            if (!transfer->setData(iterator.key().toUtf8(), iterator.value())) {
                writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
                return true;
            }
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(transferToMap(transfer)));
        return true;
    }
    
    if (request->method() == QHttpRequest::HTTP_DELETE) {
        Transfer *transfer = Transfers::instance()->get(parts.at(1));
        
        if (!transfer) {
            return false;
        }
        
        transfer->cancel();
        writeResponse(response, QHttpResponse::STATUS_OK);
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
