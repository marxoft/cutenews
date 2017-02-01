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
            
            Button {
                width: parent.width
                text: qsTr("Url openers")
                onClicked: popups.open(urlOpenersDialog, root)
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
                onClicked: popups.open(logDialog, root)
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
                onClicked: popups.open(deleteDialog, root)
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Maximum concurrent downloads")
                model: ConcurrentTransfersModel {}
                value: serversettings.maximumConcurrentTransfers
                enabled: settings.serverAddress != ""
                onSelected: serversettings.maximumConcurrentTransfers = value
            }
        
            CheckBox {
                width: parent.width
                text: qsTr("Start downloads automatically")
                enabled: settings.serverAddress != ""
                checked: serversettings.startTransfersAutomatically
                onClicked: serversettings.startTransfersAutomatically = checked
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
    
    Component {
        id: deleteDialog
        
        DeleteDialog {}
    }
    
    Component {
        id: urlOpenersDialog
        
        UrlOpenersDialog {}
    }
    
    Component {
        id: logDialog
        
        LogDialog {}
    }
    
    onAccepted: {
        settings.serverUsername = usernameField.text;
        settings.serverPassword = passwordField.text;
        settings.serverAuthenticationEnabled = authCheckBox.checked;
        settings.serverAddress = addressField.text;
    }
    
    Component.onCompleted: if (settings.serverAddress) serversettings.load();
}