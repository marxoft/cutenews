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

import QtQuick 1.1
import com.nokia.meego 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ListItem {
    id: root
    
    height: 96 + UI.PADDING_DOUBLE * 2
    
    Image {
        id: attachmentIcon
        
        anchors {
            right: parent.right
            top: parent.top
            margins: UI.PADDING_DOUBLE
        }
        width: 32
        height: 32
        source: "image://theme/icon-m-content-attachment-inverse"
        smooth: true
        visible: hasEnclosures
    }
    
    Image {
        id: favouriteIcon
        
        anchors {
            right: parent.right
            top: attachmentIcon.visible ? attachmentIcon.bottom : parent.top
            margins: UI.PADDING_DOUBLE
        }
        source: "image://theme/icon-s-common-favorite-mark-inverse"
        visible: favourite
    }
    
    Rectangle {
        id: unreadIndicator
        
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            margins: UI.PADDING_DOUBLE
        }
        width: UI.PADDING_LARGE
        color: settings.activeColor
        visible: !read
    }
  
    Label {
        anchors {
            left: unreadIndicator.right
            right: (hasEnclosures) || (favourite) ? favouriteIcon.left : parent.right
            top: parent.top
            margins: UI.PADDING_DOUBLE
        }
        maximumLineCount: 2
        elide: Text.ElideRight
        font.bold: true
        text: title
    }

    Label {
        anchors {
            left: unreadIndicator.right
            right: (hasEnclosures) || (favourite) ? favouriteIcon.left : parent.right
            bottom: parent.bottom
            margins: UI.PADDING_DOUBLE
        }
        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
        font.pixelSize: UI.FONT_SMALL
        font.family: UI.FONT_FAMILY_LIGHT
        color: read ? platformStyle.textColor : settings.activeColor
        text: Qt.formatDateTime(date, "dd/MM/yyyy HH:mm")
    }
}
        