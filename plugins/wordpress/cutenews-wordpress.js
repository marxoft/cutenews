/**
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

var JSON_PATH = "/wp-json";
var POSTS_PATH = JSON_PATH + "/wp/v2/posts";
var USER_AGENT = "Mozilla/5.0 (X11; Linux x86_64; rv:53.0) Gecko/20100101 Firefox/53.0";

var request = null;

function getFeed(settings) {
    if (!settings.host) {
        error(qsTr("No host specified"));
        return;
    }

    var host = settings.host;

    if (host.substr(0, 4) != "http") {
        host = "http://" + host;
    }

    if (host[host.length - 1] == "/") {
        host = host.substr(0, host.length - 1);
    }

    var limit = settings.limit || 20;
    var url = host + POSTS_PATH + "?limit=" + limit + "&per_page=" + limit;
    
    if (settings.search) {
        url += "&search=" + settings.search;
    }

    var date = new Date(settings.lastUpdated);

    if (date.getFullYear() > 1970) {
        url += "&after=" + date.toISOString();
    }

    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var posts = JSON.parse(request.responseText);
                var xml = "<rss version=\"2.0\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                + "xmlns:content=\"http://purl.org/rss/1.0/modules/content/\">"
                + "<channel><title>WordPress - " + settings.host
                + (settings.search ? " - " + settings.search : "")
                + "</title><description>WordPress blog posts</description><link>" + host + "</link><image><url>"
                + "https://secure.gravatar.com/blavatar/e6392390e3bcfadff3671c5a5653d95b?s=96&#038;"
                + "d=https%3A%2F%2Fs2.wp.com%2Fi%2Fbuttonw-com.png</url></image>";
                
                for (var i = 0; i < posts.length; i++) {
                    var post = posts[i];
                    xml += "<item><link>" + post.link + "</link><dc:date>" + (post.modified || post.date)
                    + "</dc:date><title>" + post.title.rendered + "</title><description><![CDATA["
                    + post.content.rendered + "]]></description></item>";
                }

                xml += "</channel></rss>";

                finished(xml);
            }
            catch(e) {
                error(e);
            }
        }
    }

    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
}

function cancel() {
    if (request) {
        request.abort();
        request = null;
    }

    return true;
}
