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
#include "pluginmanager.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"

static QVariantMap pluginConfigToMap(const FeedPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["handlesEnclosures"] = config->handlesEnclosures();
    map["id"] = config->id();
    map["pluginType"] = config->pluginType();
    map["settings"] = config->settings();
    map["version"] = config->version();
    return map;
}

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
        QVariantList configs;
        const FeedPluginList plugins = PluginManager::instance()->plugins();
        
        for (int i = 0; i < plugins.size(); i++) {
            configs << pluginConfigToMap(plugins.at(i).config);
        }
        
        writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(configs));
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}
