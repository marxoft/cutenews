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

import QtQuick 1.1
import com.nokia.symbian 1.1
import cuteNews 1.0

MyPage {
    id: root
    
    title: qsTr("Client")
    tools: ToolBarLayout {
        BackToolButton {
            onClicked: {
                settings.serverUsername = usernameField.text;
                settings.serverPassword = passwordField.text;
                settings.serverAuthenticationEnabled = authenticationSwitch.checked;
                settings.serverAddress = addressField.text;
            }
        }
    }
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: inputContext.visible ? height : column.height + platformStyle.paddingLarge * 2
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                leftMargin: platformStyle.paddingLarge
                right: parent.right
                rightMargin: platformStyle.paddingLarge
                top: parent.top
            }
            spacing: platformStyle.paddingLarge
            
            ValueSelector {
                id: orientationSelector
                
                x: -platformStyle.paddingLarge
                width: parent.width + platformStyle.paddingLarge * 2
                title: qsTr("Screen orientation")
                model: ScreenOrientationModel {}
                value: settings.screenOrientation
                visible: !inputContext.visible
                onAccepted: settings.screenOrientation = value
            }
            
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: qsTr("Server address")
                visible: addressField.visible
            }
            
            MyTextField {
                id: addressField
                
                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                text: settings.serverAddress
                visible: (!inputContext.visible) || (activeFocus)
            }
            
            MySwitch {
                id: authenticationSwitch
                
                width: parent.width
                text: qsTr("Enable server authentication");
                visible: !inputContext.visible
                checked: settings.serverAuthenticationEnabled
            }
            
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: qsTr("Username")
                visible: usernameField.visible
            }
            
            MyTextField {
                id: usernameField
                
                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                text: settings.serverUsername
                visible: (!inputContext.visible) || (activeFocus)
                onAccepted: passwordField.forceActiveFocus()
            }
            
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: qsTr("Password")
                visible: passwordField.visible
            }
            
            MyTextField {
                id: passwordField
                
                width: parent.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                text: settings.serverPassword
                visible: (!inputContext.visible) || (activeFocus)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
}
