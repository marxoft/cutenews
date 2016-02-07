/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Item {
    id: root
    
    property int value
    
    width: 32
    height: 32
    
    BorderImage {
        anchors.fill: parent
        border {
            left: 10
            right: 10
            top: 10
            bottom: 10
        }
        source: "image://theme/" + settings.activeColorString + "-meegotouch-countbubble-inverted-background-large"
    }
    
    Label {
        id: label
        
        anchors.centerIn: parent
        color: UI.COLOR_INVERTED_FOREGROUND
        font.pixelSize: UI.FONT_LSMALL
        text: root.value
    }
    
    QtObject {
        id: internal

        function getBubbleWidth() {
            if (root.value < 10) {
                return 32;
            }
            else if (root.value < 100) {
                return 40;
            }
            else if (root.value < 1000) {
                return 52;
            }
            else {
                return label.paintedWidth + 19
            }
        }
    }
    
    onValueChanged: width = internal.getBubbleWidth()
}
