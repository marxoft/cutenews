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
import cuteNews 1.0

Window {
    id: root
    
    title: qsTr("Downloads")
    menuBar: MenuBar {
        MenuItem {
            text: qsTr("Start all downloads")
            onTriggered: transfers.start()
        }
        
        MenuItem {
            text: qsTr("Pause all downloads")
            onTriggered: transfers.pause()
        }
    }
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: TransferModel {
            id: transferModel
        }
        delegate: TransferDelegate {
            onClicked: popups.open(contextMenu, root)
            onPressAndHold: popups.open(contextMenu, root)
        }
    }
    
    Label {
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No downloads")
        visible: transfers.count == 0
    }
    
    Component {
        id: contextMenu
        
        Menu {            
            MenuItem {
                text: transferModel.data(view.currentIndex, "status") == Transfer.Downloading ? qsTr("Pause")
                : qsTr("Start")
                onTriggered: transferModel.data(view.currentIndex, "status") == Transfer.Downloading
                ? transfers.get(view.currentIndex).pause() : transfers.get(view.currentIndex).queue()
            }
            
            MenuItem {
                text: qsTr("Category")
                onTriggered: popups.open(categoryDialog, root)
            }
            
            MenuItem {
                text: qsTr("Priority")
                onTriggered: popups.open(priorityDialog, root)
            }
            
            MenuItem {
                text: qsTr("Remove")
                onTriggered: transfers.get(view.currentIndex).cancel()
            }
        }
    }
    
    Component {
        id: categoryDialog
        
        ListPickSelector {
            title: qsTr("Category")
            model: CategoryNameModel {}
            textRole: "name"
            onSelected: transferModel.setData(view.currentIndex, text, "category")
        }
    }
    
    Component {
        id: priorityDialog
        
        ListPickSelector {
            title: qsTr("Priority")
            model: TransferPriorityModel {}
            textRole: "name"
            onSelected: transferModel.setData(view.currentIndex, model.data(currentIndex, "value"), "priority")
        }
    }
}
