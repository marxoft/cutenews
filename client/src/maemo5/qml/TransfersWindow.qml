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
            action: reloadAction
        }
        
        MenuItem {
            action: startAllAction
        }
        
        MenuItem {
            action: pauseAllAction
        }
    }
    
    Action {
        id: reloadAction
        
        text: qsTr("Reload")
        autoRepeat: false
        shortcut: settings.reloadShortcut
        onTriggered: transfers.load()
    }
    
    Action {
        id: startAllAction
        
        text: qsTr("Start all downloads")
        autoRepeat: false
        shortcut: settings.startAllTransfersShortcut
        onTriggered: transfers.start()
    }
    
    Action {
        id: pauseAllAction
        
        text: qsTr("Pause all downloads")
        autoRepeat: false
        shortcut: settings.pauseAllTransfersShortcut
        onTriggered: transfers.pause()
    }
    
    Action {
        id: startAction
        
        text: qsTr("Start")
        autoRepeat: false
        shortcut: settings.startTransferShortcut
        enabled: view.currentIndex >= 0
        onTriggered: transfers.get(view.currentIndex).queue()
    }
    
    Action {
        id: pauseAction
        
        text: qsTr("Pause")
        autoRepeat: false
        shortcut: settings.pauseTransferShortcut
        enabled: view.currentIndex >= 0
        onTriggered: transfers.get(view.currentIndex).pause()
    }
    
    Action {
        id: categoryAction
        
        text: qsTr("Category")
        autoRepeat: false
        shortcut: settings.transferCategoryShortcut
        enabled: view.currentIndex >= 0
        onTriggered: popupManager.open(categoryDialog, root)
    }
    
    Action {
        id: priorityAction
        
        text: qsTr("Priority")
        autoRepeat: false
        shortcut: settings.transferPriorityShortcut
        enabled: view.currentIndex >= 0
        onTriggered: popupManager.open(priorityDialog, root)
    }
    
    Action {
        id: removeAction
        
        text: qsTr("Remove")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        enabled: view.currentIndex >= 0
        onTriggered: popupManager.open(removeDialog, root)
    }
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: transfers
        delegate: TransferDelegate {
            onClicked: popupManager.open(contextMenu, root)
            onPressAndHold: popupManager.open(contextMenu, root)
        }
    }
    
    Label {
        id: label
        
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No downloads")
        visible: false
    }
    
    Component {
        id: contextMenu
        
        Menu {            
            MenuItem {
                text: transfers.data(view.currentIndex, "status") >= Transfer.Queued ? qsTr("Pause") : qsTr("Start")
                onTriggered: transfers.data(view.currentIndex, "status") >= Transfer.Queued
                ? transfers.get(view.currentIndex).pause() : transfers.get(view.currentIndex).queue()
            }
            
            MenuItem {
                action: categoryAction
            }
            
            MenuItem {
                action: priorityAction
            }
            
            MenuItem {
                action: removeAction
            }
        }
    }
    
    Component {
        id: categoryDialog
        
        ListPickSelector {
            title: qsTr("Category")
            model: CategoryNameModel {}
            textRole: "name"
            currentIndex: Math.max(0, model.match(0, "value", transfers.data(view.currentIndex, "category")))
            onSelected: transfers.setData(view.currentIndex, text, "category")
        }
    }
    
    Component {
        id: priorityDialog
        
        ListPickSelector {
            title: qsTr("Priority")
            model: TransferPriorityModel {}
            textRole: "name"
            currentIndex: Math.max(0, model.match(0, "value", transfers.data(view.currentIndex, "priority")))
            onSelected: transfers.setData(view.currentIndex, model.data(currentIndex, "value"), "priority")
        }
    }
    
    Component {
        id: removeDialog
        
        MessageBox {
            text: qsTr("Do you want to remove") + " '" + transferModel.data(view.currentIndex, "name") + "'?"
            onAccepted: transfers.get(view.currentIndex).cancel()
        }
    }
    
    Connections {
        target: transfers
        onStatusChanged: {
            switch (transfers.status) {
            case TransferModel.Active: {
                root.showProgressIndicator = true;
                reloadAction.enabled = false;
                label.visible = false;
                break;
            }
            default: {
                root.showProgressIndicator = false;
                reloadAction.enabled = true;
                label.visible = (transfers.count == 0);
                break;
            }
            }
        }
    }
    
    Component.onCompleted: transfers.load()
}
