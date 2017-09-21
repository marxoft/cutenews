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
#include "serverresponse.h"

static QVariantMap pluginConfigToMap(const FeedPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["id"] = config->id();
    map["pluginType"] = config->pluginType();
    map["supportsArticles"] = config->supportsArticles();
    map["articleRegExp"] = config->articleRegExp().pattern();
    map["articleSettings"] = config->articleSettings();
    map["supportsEnclosures"] = config->supportsEnclosures();
    map["enclosureRegExp"] = config->enclosureRegExp().pattern();
    map["enclosureSettings"] = config->enclosureSettings();
    map["supportsFeeds"] = config->supportsFeeds();
    map["feedSettings"] = config->feedSettings();
    map["version"] = config->version();
    return map;
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
