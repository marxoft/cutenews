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

import QtQuick 1.1
import cuteNews 1.0

MaskedItem {
    id: root

    property url source
    property url placeholderSource
    property alias sourceSize: icon.sourceSize
    property alias asynchronous: icon.asynchronous
    property alias fillMode: icon.fillMode
    property alias smooth: icon.smooth
    property alias status: icon.status
    
    mask: Image {
        width: root.width
        height: root.height
        source: "images/mask.png"
        fillMode: Image.Stretch
        smooth: true
    }

    Image {
        id: frame

        anchors.fill: parent
        source: "images/frame.png"
        sourceSize.width: width
        sourceSize.height: height
        smooth: icon.smooth
        fillMode: Image.Stretch
        visible: icon.status == Image.Ready

        Image {
            id: icon

            property bool complete: false

            function update() {
                if (complete) {
                    if (!root.source.toString()) {
                        source = root.placeholderSource;
                    }
                    else {
                        source = root.source;
                    }
                }
            }

            z: -1
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            smooth: true
            fillMode: Image.PreserveAspectFit
            clip: true
            asynchronous: true
            onStatusChanged: if ((status == Image.Error) && (source == root.source)) source = root.placeholderSource;
        }
    }

    onSourceChanged: icon.update()
    onPlaceholderSourceChanged: icon.update()
    Component.onCompleted: {
        icon.complete = true;
        icon.update();
    }
}
