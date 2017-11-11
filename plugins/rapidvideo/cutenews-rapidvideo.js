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

var request = null;

function getEnclosure(url, settings) {
    request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4) {
            try {
                var response = request.responseText;
                var fileName = /"og:title" content="([^"]+)/.exec(response)[1];

                try {
                    var sources = JSON.parse(/"sources": (\[[^\]]+\])/.exec(response)[1]);

                    if (!sources.length) {
                        error(qsTr("No video formats found"));
                        return;
                    }

                    var format = settings.format || "1080";

                    for (var i = sources.length - 1; i >= 0; i--) {
                        var source = sources[i];

                        if (source.res == format) {
                            finished(new EnclosureResult(fileName, new NetworkRequest(source.file)));
                            return;
                        }
                    }

                    finished(new EnclosureResult(fileName, new NetworkRequest(sources[0].file)));
                }
                catch(e) {
                    var source = /<source src="([^"]+)/.exec(request.responseText)[1];
                    finished(new EnclosureResult(fileName, new NetworkRequest(source)));
                }
            }
            catch(e) {
                error(e);
            }
        }
    }

    request.open("GET", url);
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
