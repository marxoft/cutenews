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

import QtQuick 1.0
import org.hildon.components 1.0

ListItem {
    id: root
    
    Image {
        id: icon
        
        width: 48
        height: 48
        anchors {
            left: parent.left
            leftMargin: platformStyle.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        source: iconPath ? iconPath : "image://icon/general_rss"
        smooth: true
    }
    
    Label {
        anchors {
            left: icon.right
            right: parent.right
            top: index > 1 ? parent.top : undefined
            verticalCenter: index > 1 ? undefined : parent.verticalCenter
            margins: platformStyle.paddingMedium
        }
        elide: Text.ElideRight
        color: (subscriptions.activeSubscription == id) || (!read) ? platformStyle.activeTextColor
                                                                   : platformStyle.defaultTextColor
        text: title
    }
    
    Label {
        anchors {
            left: icon.right
            right: parent.right
            bottom: parent.bottom
            margins: platformStyle.paddingMedium
        }
        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
        font.pointSize: platformStyle.fontSizeSmall
        color: (subscriptions.activeSubscription == id) || (!read) ? platformStyle.activeTextColor
                                                                   : platformStyle.secondaryTextColor
        text: subscriptions.activeSubscription == id ? qsTr("Updating...")
              : Qt.formatDateTime(lastUpdated, "dd/MM/yyyy HH:mm")
                + (read ? "" : " - " + unreadArticles + " " + qsTr("unread"))
        visible: index > 1
    }
}
        