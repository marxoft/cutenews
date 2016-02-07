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

Dialog {
    id: root
    
    title: qsTr("Settings")
    height: Math.min(350, column.height + platformStyle.paddingMedium)
    
    Flickable {
        id: flickable
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: column.height
        
        Column {
            id: column
    
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: platformStyle.paddingMedium
            
            ValueButton {
                width: parent.width
                text: qsTr("User interface")
                pickSelector: userInterfaceSelector
            }
            
            ValueButton {
                width: parent.width
                text: qsTr("Article view mode")
                pickSelector: viewModeSelector
            }
        
            ValueButton {
                width: parent.width
                text: qsTr("Download path")
                valueText: settings.downloadPath
                onClicked: dialogs.showFileDialog()
            }
        
            CheckBox {
                width: parent.width
                text: qsTr("Start downloads automatically")
                checked: settings.startTransfersAutomatically
                onClicked: settings.startTransfersAutomatically = checked
            }
        
            Button {
                width: parent.width
                text: qsTr("Network proxy")
                onClicked: dialogs.showNetworkProxyDialog() 
            }
        
            Button {
                width: parent.width
                text: qsTr("Url openers")
                onClicked: dialogs.showUrlOpenersDialog()
            }
        }
    }
    
    Button {
        id: button
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        style: DialogButtonStyle {}
        text: qsTr("Done")
        onClicked: root.accept()
    }
    
    ListPickSelector {
        id: userInterfaceSelector
        
        model: UserInterfaceModel {}
        textRole: "name"
        currentIndex: model.match("value", settings.userInterface)
        onSelected: settings.userInterface = model.data(currentIndex, "value")
    }
    
    ListPickSelector {
        id: viewModeSelector
        
        model: ViewModeModel {}
        textRole: "name"
        currentIndex: model.match("value", settings.viewMode)
        onSelected: settings.viewMode = model.data(currentIndex, "value")
    }
    
    QtObject {
        id: dialogs
        
        property FileDialog fileDialog
        property NetworkProxyDialog proxyDialog
        property UrlOpenersDialog urlOpenersDialog
        
        function showFileDialog() {
            if (!fileDialog) {
                fileDialog = fileDialogComponent.createObject(root);
            }
            
            fileDialog.open();
        }
        
        function showNetworkProxyDialog() {
            if (!proxyDialog) {
                proxyDialog = proxyDialogComponent.createObject(root);
            }
            
            proxyDialog.open();
        }
        
        function showUrlOpenersDialog() {
            if (!urlOpenersDialog) {
                urlOpenersDialog = urlOpenersDialogComponent.createObject(root);
            }
            
            urlOpenersDialog.open();
        }
    }
    
    Component {
        id: fileDialogComponent
        
        FileDialog {
            selectFolder: true
            folder: settings.downloadPath
            onAccepted: settings.downloadPath = folder
        }
    }
    
    Component {
        id: proxyDialogComponent
        
        NetworkProxyDialog {}
    }
    
    Component {
        id: urlOpenersDialogComponent
        
        UrlOpenersDialog {}
    }
    
    onStatusChanged: if (status == DialogStatus.Open) flickable.contentY = 0;
}
