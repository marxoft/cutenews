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

var USER_AGENT = "Wget/1.13.4 (linux-gnu)";

var request = null;

function getEnclosure(url, settings) {
    if (url.substring(url.length - 4) == ".mp4") {
        var fileName = url.substring(url.lastIndexOf("/") + 1);
        finished(new EnclosureResult(fileName, new NetworkRequest(url)));
        return true;
    }
    
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;
            
            try {
                var fileName = response.split("og:title\" content=\"")[1].split("\"")[0];
                var videoUrl = response.split("og:video\" content=\"")[1].split("\"")[0];
                finished(new EnclosureResult(fileName, new NetworkRequest(videoUrl)));
            }
            catch(err) {
                error(err);
            }
        }
    }
    
    request.open("GET", url);
    request.setRequestHeader("User-Agent", USER_AGENT);
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
