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
import cuteNews 1.0

MyPage {
    id: root
    
    title: qsTr("Server")
    tools: ToolBarLayout {
        BackToolButton {}
    }
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: column.height + platformStyle.paddingLarge
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: platformStyle.paddingLarge
            
            ValueSelector {
                id: concurrentSelector
                
                width: parent.width
                title: qsTr("Maximum concurrent downloads")
                model: ConcurrentTransfersModel {}
                value: serversettings.maximumConcurrentTransfers
                enabled: settings.serverAddress != ""
                onAccepted: serversettings.maximumConcurrentTransfers = value
            }
            
            MySwitch {
                id: automaticSwitch
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: qsTr("Start downloads automatically")
                checked: serversettings.startTransfersAutomatically
                enabled: settings.serverAddress != ""
                onCheckedChanged: serversettings.startTransfersAutomatically = checked
            }
            
            MySwitch {
                id: offlineSwitch
                
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: qsTr("Enable offline mode")
                checked: serversettings.offlineModeEnabled
                enabled: settings.serverAddress != ""
                onCheckedChanged: serversettings.offlineModeEnabled = checked
            }
            
            TextDelegate {
                id: deleteButton
                
                width: parent.width
                flickableMode: true
                subItemIndicator: true
                text: qsTr("Delete read articles")
                enabled: settings.serverAddress != ""
                onClicked: appWindow.pageStack.push(Qt.resolvedUrl("DeletePage.qml"))
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
