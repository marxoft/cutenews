/*
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

var RSS_URL = "http://www.dailymotion.com/rss/videos/";
var USER_AGENT = "Wget/1.13.4 (linux-gnu)";
var VIDEO_COOKIE = "ff=off";
var VIDEO_FORMATS = ["2160", "1080", "720", "480", "380", "240", "144"];

var request = null;

function getFeed(settings) {
    if (!settings.query) {
        error("No query specified");
        return false;
    }
    
    var url = RSS_URL;
    
    if (settings.type == "search") {
        url += "search/";
    }
    
    url += settings.query.replace(/\s+/g, "+");
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            finished(request.responseText);
        }
    }
    
    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.send();
    return true;
}

function getEnclosure(url, settings) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;
            
            try {
                var config = JSON.parse(/var config = ({.+});\n/.exec(response)[1]);
                var qualities = config.metadata.qualities;
                
                for (var i = Math.max(0, VIDEO_FORMATS.indexOf(settings.videoFormat)); i < VIDEO_FORMATS.length; i++) {
                    var quality = qualities[VIDEO_FORMATS[i]];
                    
                    if ((quality) && (quality.length > 0)) {
                        for (var i = 0; i < quality.length; i++) {
                            if (quality[i].type === "video/mp4") {
                                var fileName = config.metadata.title + ".mp4";
                                var videoUrl = quality[i].url;
                                finished(new EnclosureResult(fileName, new NetworkRequest(videoUrl)));
                                return;
                            }
                        }
                    }
                }
                
                error("No video streams found");
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
    request.setRequestHeader("Cookie", VIDEO_COOKIE);
    request.send();
    return true;
}

function cancel() {
    if (request) {
        request.abort();
        request = null;
    }
    
    return true;
}
