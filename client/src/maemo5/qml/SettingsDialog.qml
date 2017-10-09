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
    height: 360
    
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
            
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
                text: qsTr("Client")
            }
            
            Label {
                width: parent.width
                text: qsTr("Server address")
            }
            
            TextField {
                id: addressField
                
                width: parent.width
                text: settings.serverAddress
            }
            
            CheckBox {
                id: authCheckBox
                
                width: parent.width
                text: qsTr("Enable server authentication")
                checked: settings.serverAuthenticationEnabled
            }
            
            Label {
                width: parent.width
                text: qsTr("Server username")
            }
            
            TextField {
                id: usernameField
                
                width: parent.width
                text: settings.serverUsername
            }
            
            Label {
                width: parent.width
                text: qsTr("Server password")
            }
            
            TextField {
                id: passwordField
                
                width: parent.width
                echoMode: TextInput.Password
                text: settings.serverPassword
            }

            CheckBox {
                width: parent.width
                text: qsTr("Enable JavaScript in browser")
                checked: settings.enableJavaScriptInBrowser
                onCheckedChanged: settings.enableJavaScriptInBrowser = checked
            }
            
            Button {
                width: parent.width
                text: qsTr("URL openers")
                onClicked: popupManager.open(Qt.resolvedUrl("UrlOpenersDialog.qml"), root)
            }
             
            Button {
                width: parent.width
                text: qsTr("Keyboard shortcuts")
                onClicked: popupManager.open(Qt.resolvedUrl("ShortcutsDialog.qml"), root)
            }

            ListSelectorButton {
                width: parent.width
                text: qsTr("Screen orientation")
                model: ScreenOrientationModel {}
                value: settings.screenOrientation
                onSelected: settings.screenOrientation = value
            }
           
            ListSelectorButton {
                width: parent.width
                text: qsTr("Logging verbosity")
                model: LoggerVerbosityModel {}
                value: settings.loggerVerbosity
                onSelected: settings.loggerVerbosity = value
            }
            
            Button {
                width: parent.width
                text: qsTr("View log")
                onClicked: popupManager.open(Qt.resolvedUrl("LogDialog.qml"), root)
            }
            
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
                text: qsTr("Server")
            }
            
            CheckBox {
                width: parent.width
                text: qsTr("Work offline")
                enabled: settings.serverAddress != ""
                checked: serversettings.offlineModeEnabled
                onCheckedChanged: serversettings.offlineModeEnabled = checked
            }
            
            Button {
                width: parent.width
                text: qsTr("Delete read articles")
                enabled: settings.serverAddress != ""
                onClicked: popupManager.open(Qt.resolvedUrl("DeleteDialog.qml"), root)
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Maximum concurrent downloads")
                model: ConcurrentTransfersModel {}
                value: serversettings.maximumConcurrentTransfers
                enabled: settings.serverAddress != ""
                onSelected: serversettings.maximumConcurrentTransfers = value
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Custom download command (%f for filename)")
            }

            TextField {
                width: parent.width
                enabled: settings.serverAddress != ""
                text: serversettings.customTransferCommand
                onTextChanged: serversettings.customTransferCommand = text
            }

            CheckBox {
                width: parent.width
                text: qsTr("Enable custom download command")
                enabled: settings.serverAddress != ""
                checked: serversettings.customTransferCommandEnabled
                onCheckedChanged: serversettings.customTransferCommandEnabled = checked
            }
        
            CheckBox {
                width: parent.width
                text: qsTr("Start downloads automatically")
                enabled: settings.serverAddress != ""
                checked: serversettings.startTransfersAutomatically
                onCheckedChanged: serversettings.startTransfersAutomatically = checked
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

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: flickable
            anchors.right: parent.right
            anchors.bottom: button.top
        }

        PropertyChanges {
            target: flickable
            anchors.rightMargin: 0
            anchors.bottomMargin: platformStyle.paddingMedium
            clip: true
        }

        PropertyChanges {
            target: button
            width: parent.width
        }

        PropertyChanges {
            target: root
            height: 680
        }
    }
    
    onAccepted: {
        settings.serverUsername = usernameField.text;
        settings.serverPassword = passwordField.text;
        settings.serverAuthenticationEnabled = authCheckBox.checked;
        settings.serverAddress = addressField.text;
    }
    
    Component.onCompleted: if (settings.serverAddress) serversettings.load();
}
