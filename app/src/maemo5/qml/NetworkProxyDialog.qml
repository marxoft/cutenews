/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

Dialog {
    id: root
    
    title: qsTr("Network proxy")
    height: 350
    
    Flickable {
        id: flicakble
        
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
        
            CheckBox {
                id: proxyCheckBox
            
                width: parent.width
                text: qsTr("Use network proxy")
            }
        
            Label {
                width: parent.width
                text: qsTr("Host")
            }
        
            TextField {
                id: hostField
            
                width: parent.width
            }
        
            Label {
                width: parent.width
                text: qsTr("Port")
            }
        
            SpinBox {
                id: portField
            
                width: parent.width
                minimum: 1
                maximum: 1000000
            }
        
            Label {
                width: parent.width
                text: qsTr("Username")
            }
        
            TextField {
                id: usernameField
            
                width: parent.width
            }
        
            Label {
                width: parent.width
                text: qsTr("Password")
            }
        
            TextField {
                id: passwordField
            
                width: parent.width
                echoMode: TextInput.Password
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
    
    onStatusChanged: {
        if (status == DialogStatus.Open) {
            proxyCheckBox.checked = settings.networkProxyEnabled;
            hostField.text = settings.networkProxyHost;
            portField.value = settings.networkProxyPort;
            usernameField.text = settings.networkProxyUsername;
            passwordField.text = settings.networkProxyPassword;
            flickable.contentY = 0;
        }
    }
    onAccepted: {
        settings.networkProxyEnabled = proxyCheckBox.checked;
        settings.networkProxyHost = hostField.text;
        settings.networkProxyPort = portField.value;
        settings.networkProxyUsername = usernameField.text;
        settings.networkProxyPassword = passwordField.text;
        settings.setNetworkProxy();
    }
}
