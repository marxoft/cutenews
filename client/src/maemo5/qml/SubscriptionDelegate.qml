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
    
    Label {
        id: titleLabel
        
        anchors {
            left: parent.left
            right: !read ? unreadLoader.left : parent.right
            top: index > 1 ? parent.top : undefined
            verticalCenter: index > 1 ? undefined : parent.verticalCenter
            margins: platformStyle.paddingMedium
        }
        elide: Text.ElideRight
        text: title
    }
    
    Loader {
        id: dateLoader
        
        anchors {
            left: titleLabel.left
            right: titleLabel.right
            bottom: parent.bottom
            bottomMargin: platformStyle.paddingMedium
        }
        sourceComponent: index > 1 ? dateLabel : undefined
    }
    
    Loader {
        id: unreadLoader
        
        anchors {
            right: parent.right
            rightMargin: platformStyle.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        sourceComponent: !read ? unreadLabel : undefined
    }
    
    Component {
        id: dateLabel
        
        Label {
            width: parent.width
            verticalAlignment: Text.AlignBottom
            elide: Text.ElideRight
            font.pointSize: platformStyle.fontSizeSmall
            color: platformStyle.secondaryTextColor
            text: subscriptions.activeSubscription == id ? qsTr("Updating...") : lastUpdatedString
        }
    }
    
    Component {
        id: unreadLabel
        
        Label {
            text: unreadArticles
            color: (root.pressed) || ((root.ListView.isCurrentItem)
            && (root.ListView.view.navigationMode == NavigationMode.KeyNavigation))
            ? platformStyle.defaultTextColor : platformStyle.activeTextColor
        }
    }
}
