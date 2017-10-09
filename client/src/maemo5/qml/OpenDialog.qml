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

import QtQuick 1.0
import org.hildon.components 1.0
import cuteNews 1.0

ListPickSelector {
    id: root

    property string url

    title: qsTr("Open")
    textRole: "name"
    model: SelectionModel {}
    onUrlChanged: {
        model.clear();
        var articleConfig = plugins.getConfigForArticle(url);
        var enclosureConfig = plugins.getConfigForEnclosure(url);

        if (articleConfig) {
            model.append(qsTr("Fetch article from") + " " + articleConfig.displayName, 0);
        }

        model.append(qsTr("Open URL in browser"), 1);
        model.append(qsTr("Open URL externally"), 2);

        if (enclosureConfig) {
            model.append(qsTr("Open URL externally via") + " " + enclosureConfig.displayName, 3);
        }

        currentIndex = 0;
    }
    onSelected: {
        switch (model.data(currentIndex, "value")) {
            case 0:
                windowStack.push(Qt.resolvedUrl("ArticleRequestWindow.qml"), {url: url});
                break;
            case 1:
                windowStack.push(Qt.resolvedUrl("BrowserWindow.qml"), {url: url});
                break;
            case 2:
                urlopener.open(url);
                break;
            case 3:
                urlopener.openWithPlugin(url);
                break;
            default:
                break;
        }
    }
}
