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
import com.nokia.symbian 1.1

MyListItem {
    id: root
        
    MyListItemText {
        id: titleLabel

        anchors {
            left: unreadLoader.right
            leftMargin: platformStyle.paddingLarge
            right: iconLoader.item ? iconLoader.left : root.paddingItem.right
            rightMargin: iconLoader.item ? platformStyle.paddingLarge : 0
            top: root.paddingItem.top
        }
        role: "Title"
        mode: root.mode
        elide: Text.ElideRight
        text: title
    }

    MyListItemText {
        id: dateLabel

        anchors {
            left: titleLabel.left
            right: titleLabel.right
            bottom: root.paddingItem.bottom
        }
        role: "SubTitle"
        mode: root.mode
        elide: Text.ElideRight
        text: dateString
    }
    
    Loader {
        id: unreadLoader
        
        anchors {
            left: root.paddingItem.left
            top: root.paddingItem.top
            bottom: root.paddingItem.bottom
        }
        width: platformStyle.paddingLarge
        sourceComponent: read ? undefined : unreadIndicator
    }
    
    Loader {
        id: iconLoader
        
        anchors {
            right: root.paddingItem.right
            verticalCenter: root.paddingItem.verticalCenter
        }
        width: platformStyle.graphicSizeTiny
        height: platformStyle.graphicSizeTiny
        sourceComponent: favourite ? icon : undefined
    }
    
    Component {
        id: unreadIndicator
        
        Rectangle {
            anchors.fill: parent
            color: root.mode == "pressed" ? platformStyle.colorNormalLight : platformStyle.colorNormalLink
        }
    }
    
    Component {
        id: icon
        
        Image {
            anchors.fill: parent
            smooth: true
            source: "images/favourite.svg"
        }
    }
}
