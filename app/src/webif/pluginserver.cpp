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
#include "pluginsettings.h"
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

static void pluginConfigSetting(const PluginSettings &plugin, QVariantMap &setting, const QString &group = QString()) {
    QString key = setting.value("key").toString();

    if (key.isEmpty()) {
        return;
    }
    
    if (!group.isEmpty()) {
        key.prepend("/");
        key.prepend(group);
    }

    const QString type = setting.value("type").toString();

    if (type == "group") {
        QVariantList settings = setting.value("settings").toList();
        
        for (int i = 0; i < settings.size(); i++) {
            QVariantMap map = settings.at(i).toMap();
            pluginConfigSetting(plugin, map, key);
            settings[i] = map;
        }

        setting["settings"] = settings;
    }
    else {
        setting["value"] = plugin.value(key, setting.value("value"));
    }
}       

static QVariantList pluginConfigArticleSettings(const FeedPluginConfig *config) {
    QVariantList settings = config->articleSettings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        pluginConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

static QVariantList pluginConfigEnclosureSettings(const FeedPluginConfig *config) {
    QVariantList settings = config->enclosureSettings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        pluginConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

PluginServer::PluginServer(QObject *parent) :
    QObject(parent)
{
}

bool PluginServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {    
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);

    if ((parts.size() > 3) || (parts.first().compare("plugins", Qt::CaseInsensitive) != 0)) {
       return false;
    } 

    if (parts.size() == 1) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            QVariantList configs;
            const FeedPluginList plugins = PluginManager::instance()->plugins();
            
            for (int i = 0; i < plugins.size(); i++) {
                configs << pluginConfigToMap(plugins.at(i).config);
            }
            
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(configs));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        }

        return true;
    }

    const FeedPluginConfig *config = PluginManager::instance()->getConfig(parts.at(1));

    if (!config) {
        writeResponse(response, QHttpResponse::STATUS_NOT_FOUND);
        return true;
    }

    if (parts.size() == 2) {
        if (request->method() == QHttpRequest::HTTP_GET) {
            writeResponse(response, QHttpResponse::STATUS_OK, QtJson::Json::serialize(pluginConfigToMap(config)));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        }

        return true;
    }
    
    if (parts.size() == 3) {
        if (parts.at(2).compare("articlesettings", Qt::CaseInsensitive) == 0) {
            if (request->method() == QHttpRequest::HTTP_GET) {
                writeResponse(response, QHttpResponse::STATUS_OK,
                        QtJson::Json::serialize(pluginConfigArticleSettings(config)));
            }
            else if (request->method() == QHttpRequest::HTTP_PUT) {
                PluginSettings ps(config->id());
                ps.setValues(QtJson::Json::parse(QString::fromUtf8(request->body())).toMap());
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            }
        }
        else if (parts.at(2).compare("enclosuresettings", Qt::CaseInsensitive) == 0) {
            if (request->method() == QHttpRequest::HTTP_GET) {
                writeResponse(response, QHttpResponse::STATUS_OK,
                        QtJson::Json::serialize(pluginConfigEnclosureSettings(config)));
            }
            else if (request->method() == QHttpRequest::HTTP_PUT) {
                PluginSettings ps(config->id());
                ps.setValues(QtJson::Json::parse(QString::fromUtf8(request->body())).toMap());
                writeResponse(response, QHttpResponse::STATUS_OK);
            }
            else {
                writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
            }
        }

        return true;
    }

    writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
    return true;
}

