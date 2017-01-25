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
    
    IconImage {
        id: icon
        
        anchors {
            left: root.paddingItem.left
            top: root.paddingItem.top
            bottom: root.paddingItem.bottom
        }
        width: height
        source: iconPath ? iconPath : "images/cutenews.png"
    }
    
    MyListItemText {
        id: titleLabel

        anchors {
            left: icon.right
            leftMargin: platformStyle.paddingLarge
            right: indicatorLoader.item ? indicatorLoader.left : root.paddingItem.right
            rightMargin: indicatorLoader.item ? platformStyle.paddingLarge : 0
            top: dateLoader.item ? root.paddingItem.top : undefined
            verticalCenter: dateLoader.item ? undefined : root.paddingItem.verticalCenter
        }
        role: "Title"
        mode: root.mode
        elide: Text.ElideRight
        text: title
    }
    
    Loader {
        id: dateLoader
        
        anchors {
            left: titleLabel.left
            right: titleLabel.right
            bottom: root.paddingItem.bottom
        }
        sourceComponent: lastUpdatedString ? dateLabel : undefined
    }
    
    Loader {
        id: indicatorLoader
        
        anchors {
            right: root.paddingItem.right
            verticalCenter: root.paddingItem.verticalCenter
        }
        sourceComponent: subscriptions.activeSubscription === id ? busyIndicator : !read
        ? countIndicator : undefined
    }
    
    Component {
        id: dateLabel
        
        MyListItemText {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            role: "SubTitle"
            mode: root.mode
            elide: Text.ElideRight
            text: lastUpdatedString
        }
    }
    
    Component {
        id: countIndicator
        
        MyListItemText {
            role: "Title"
            mode: root.mode
            text: unreadArticles
        }
    }
    
    Component {
        id: busyIndicator
        
        BusyIndicator {
            width: platformStyle.graphicSizeTiny
            height: platformStyle.graphicSizeTiny
            running: true
        }
    }
}
