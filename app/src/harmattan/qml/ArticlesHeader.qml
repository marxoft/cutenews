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
    
    property alias title: titleLabel.text
    property alias count: countBubble.value

    z: 1001
    width: parent ? parent.width : implicitWidth
    height: 72
    
    Label {
        id: titleLabel

        anchors {
            left: parent.left
            right: countBubble.visible ? countBubble.left : parent.right
            verticalCenter: parent.verticalCenter
            margins: UI.PADDING_DOUBLE
        }
        font.pixelSize: UI.FONT_XLARGE
        elide: Text.ElideRight
    }
    
    ColorCountBubble {
        id: countBubble
        
        anchors {
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }
        visible: value > 0
    }
    
    Image {
        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            right: parent.right
            rightMargin: UI.PADDING_DOUBLE
            bottom: parent.bottom
        }
        source: "image://theme/meegotouch-groupheader-inverted-background"
    }
}
