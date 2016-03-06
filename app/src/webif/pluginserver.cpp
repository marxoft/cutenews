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

#include "pluginserver.h"
#include "json.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "subscriptionplugins.h"

static void writeResponse(QHttpResponse *response, int responseCode, const QByteArray &data = QByteArray()) {
    response->setHeader("Content-Type", "application/json");
    response->setHeader("Content-Length", QByteArray::number(data.size()));
    response->writeHead(responseCode);
    response->end(data);
}

PluginServer::PluginServer(QObject *parent) :
    QObject(parent)
{
}

bool PluginServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {    
    if (request->path() != "/plugins") {
        return false;
    }
    
    if (request->method() == QHttpRequest::HTTP_GET) {
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(SubscriptionPlugins::pluginNames()));
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
