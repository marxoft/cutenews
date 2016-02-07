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
import com.nokia.extras 1.1
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

ListItem {
    id: root
    
    height: icon.height + UI.PADDING_DOUBLE * 2
    subItemIndicator: true
    
    Icon {
        id: icon
        
        width: 64
        height: 64
        anchors {
            left: parent.left
            leftMargin: UI.PADDING_DOUBLE
            verticalCenter: parent.verticalCenter
        }
        source: iconPath ? iconPath : "images/icon.png"
        smooth: true
    }
    
    Label {
        anchors {
            left: icon.right
            right: parent.right
            top: index > 1 ? parent.top : undefined
            verticalCenter: index > 1 ? undefined : parent.verticalCenter
            margins: UI.PADDING_DOUBLE
        }
        font.bold: true
        elide: Text.ElideRight
        text: title
    }
    
    Label {
        anchors {
            left: icon.right
            right: parent.right
            bottom: parent.bottom
            margins: UI.PADDING_DOUBLE
        }
        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
        font.pixelSize: UI.FONT_SMALL
        font.family: UI.FONT_FAMILY_LIGHT
        color: (subscriptions.activeSubscription == id) || (!read) ? settings.activeColor
                                                                   : platformStyle.textColor
        text: subscriptions.activeSubscription == id ? qsTr("Updating...")
              : Qt.formatDateTime(lastUpdated, "dd/MM/yyyy HH:mm")
                + (read ? "" : " - " + unreadArticles + " " + qsTr("unread"))
        visible: index > 1
    }
}
        