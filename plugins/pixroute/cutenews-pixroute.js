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
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            var response = request.responseText;
            
            try {
                var imageUrl = /http(s|):\/\/img\d+\.pixroute\.com\/\w\/\d+\/\w+\.\w+/.exec(response)[0];
                var fileName = imageUrl.substring(imageUrl.lastIndexOf("/") + 1);
                finished(new EnclosureResult(fileName, new NetworkRequest(imageUrl)));
            }
            catch(err) {
                error(err);
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
