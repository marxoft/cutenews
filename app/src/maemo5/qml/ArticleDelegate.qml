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
    
    Loader {
        id: unreadLoader
        
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            margins: platformStyle.paddingMedium
        }
        width: platformStyle.paddingMedium
        sourceComponent: !read ? unreadIndicator : undefined
    }
    
    Loader {
        id: favouriteLoader
        
        anchors {
            right: parent.right
            rightMargin: platformStyle.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        sourceComponent: favourite ? favouriteIndicator : undefined
    }
    
    Label {
        id: titleLabel
        
        anchors {
            left: unreadLoader.right
            right: favourite ? favouriteLoader.left : parent.right
            top: parent.top
            margins: platformStyle.paddingMedium
        }
        elide: Text.ElideRight
        text: title
    }
    
    Label {
        id: dateLabel
        
        anchors {
            left: titleLabel.left
            right: titleLabel.right
            bottom: parent.bottom
            bottomMargin: platformStyle.paddingMedium
        }
        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
        font.pointSize: platformStyle.fontSizeSmall
        color: platformStyle.secondaryTextColor
        text: dateString
    }
    
    Component {
        id: unreadIndicator
        
        Rectangle {
            anchors.fill: parent
            color: (root.pressed) || ((root.ListView.isCurrentItem)
            && (root.ListView.view.navigationMode == NavigationMode.KeyNavigation))
            ? platformStyle.defaultTextColor : platformStyle.activeTextColor
        }
    }
    
    Component {
        id: favouriteIndicator
        
        Image {
            source: "image://icon/imageviewer_favourite"
            smooth: true
        }
    }
}
