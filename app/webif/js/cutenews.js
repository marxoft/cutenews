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

var ARTICLES_PATH = "/articles";
var PLUGINS_PATH = "/plugins";
var SETTINGS_PATH = "/settings"
var SUBSCRIPTIONS_PATH = "/subscriptions";
var TRANSFERS_PATH = "/transfers";

var CuteNews = function (address, username, password) {
    this.ipaddress = address || "";
    this.username = username || "";
    this.password = password || "";
}

CuteNews.prototype.del = function (path, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }    
    
    request.open("DELETE", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.send(null);
}

CuteNews.prototype.get = function (path, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }
    
    request.open("GET", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.send(null);
}

CuteNews.prototype.post = function (path, json_data, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 201) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }
    
    request.open("POST", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.setRequestHeader("Content-Type", "application/json");
    request.send(JSON.stringify(json_data));
}

CuteNews.prototype.put = function (path, json_data, callback_ok, callback_error) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 200) {
                if (callback_ok) {
                    callback_ok.call(this, request.responseText ? JSON.parse(request.responseText) : null);
                }
            }
            else if (callback_error) {
                callback_error.call(this, request.responseText ? JSON.parse(request.responseText) : null);
            }
        }
    }
    
    request.open("PUT", this.ipaddress + path);
    
    if ((this.username) && (this.password)) {
        request.setRequestHeader("Authorization", "Basic " + btoa(this.username + ":" + this.password));
    }
    
    request.setRequestHeader("Content-Type", "application/json");
    request.send(JSON.stringify(json_data));
}

CuteNews.prototype.addSubscription = function (source, sourceType, downloadEnclosures, callback_ok, callback_error) {
    this.post(SUBSCRIPTIONS_PATH, {"source": source, "sourceType": sourceType,
                                   "downloadEnclosures": downloadEnclosures}, callback_ok, callback_error);
}

CuteNews.prototype.deleteSubscription = function (id, callback_ok, callback_error) {
    this.del(SUBSCRIPTIONS_PATH + "/" + id, callback_ok, callback_error);
}

CuteNews.prototype.getSubscription = function (id, callback_ok, callback_error) {
    this.get(SUBSCRIPTIONS_PATH + "/" + id, callback_ok, callback_error);
}

CuteNews.prototype.getSubscriptions = function (offset, limit, callback_ok, callback_error) {
    var path = SUBSCRIPTIONS_PATH + "?offset=" + (offset ? offset : "0")
               + (limit ? "&limit=" + limit : "");
    this.get(path, callback_ok, callback_error);
}

CuteNews.prototype.setSubscriptionProperties = function (id, properties, callback_ok, callback_error) {
    this.put(SUBSCRIPTIONS_PATH + "/" + id, properties, callback_ok, callback_error);
}

CuteNews.prototype.updateSubscription = function (id, callback_ok, callback_error) {
    this.get(SUBSCRIPTIONS_PATH + "/update?id=" + id, callback_ok, callback_error);
}

CuteNews.prototype.updateSubscriptions = function (callback_ok, callback_error) {
    this.get(SUBSCRIPTIONS_PATH + "/update", callback_ok, callback_error);
}

CuteNews.prototype.cancelSubscriptionUpdate = function (callback_ok, callback_error) {
    this.get(SUBSCRIPTIONS_PATH + "/cancel", callback_ok, callback_error);
}

CuteNews.prototype.getSubscriptionUpdateStatus = function (callback_ok, callback_error) {
    this.get(SUBSCRIPTIONS_PATH + "/status", callback_ok, callback_error);
}

CuteNews.prototype.deleteArticle = function (id, callback_ok, callback_error) {
    this.del(ARTICLES_PATH + "/" + id, callback_ok, callback_error);
}

CuteNews.prototype.getArticle = function (id, callback_ok, callback_error) {
    this.get(ARTICLES_PATH + "/" + id, callback_ok, callback_error);
}

CuteNews.prototype.getArticles = function (subscriptionId, offset, limit, callback_ok, callback_error) {
    var path = ARTICLES_PATH + "?sort=date&sortDescending=true&subscriptionId=" + subscriptionId
               + "&offset=" + (offset ? offset : "0") + (limit ? "&limit=" + limit : "");
    this.get(path, callback_ok, callback_error);
}

CuteNews.prototype.markArticleFavourite = function (id, favourite, callback_ok, callback_error) {
    this.put(ARTICLES_PATH + "/" + id, {"favourite": favourite}, callback_ok, callback_error);
}

CuteNews.prototype.markArticleRead = function (id, read, callback_ok, callback_error) {
    this.put(ARTICLES_PATH + "/" + id, {"read": read}, callback_ok, callback_error);
}

CuteNews.prototype.getPlugins = function (callback_ok, callback_error) {
    this.get(PLUGINS_PATH, callback_ok, callback_error);
}

CuteNews.prototype.getSettings = function (callback_ok, callback_error) {
    this.get(SETTINGS_PATH, callback_ok, callback_error);
}

CuteNews.prototype.setSettings = function (settings, callback_ok, callback_error) {
    this.put(SETTINGS_PATH, settings, callback_ok, callback_error);
}

CuteNews.prototype.addDownload = function (url, subscriptionId, callback_ok, callback_error) {
    this.post(TRANSFERS_PATH, {"url": url, "subscriptionId": subscriptionId}, callback_ok, callback_error);
}

CuteNews.prototype.cancelDownload = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/cancel?id=" + id, callback_ok, callback_error);
}

CuteNews.prototype.getDownload = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/" + id, callback_ok, callback_error);
}

CuteNews.prototype.getDownloads = function (offset, limit, callback_ok, callback_error) {
    var path = TRANSFERS_PATH + "?offset=" + (offset ? offset : "0")
               + (limit ? "&limit=" + limit : "");
    this.get(path, callback_ok, callback_error);
}

CuteNews.prototype.pauseDownload = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/pause?id=" + id, callback_ok, callback_error);
}

CuteNews.prototype.pauseDownloads = function (callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/pause", callback_ok, callback_error);
}

CuteNews.prototype.setDownloadProperties = function (id, properties, callback_ok, callback_error) {
    this.put(TRANSFERS_PATH + "/" + id, properties, callback_ok, callback_error);
}

CuteNews.prototype.startDownload = function (id, callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/start?id=" + id, callback_ok, callback_error);
}

CuteNews.prototype.startDownloads = function (callback_ok, callback_error) {
    this.get(TRANSFERS_PATH + "/start", callback_ok, callback_error);
}
