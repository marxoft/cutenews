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
import com.nokia.meego 1.0
import cuteNews 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MyPage {
    id: root

    title: qsTr("Network")
    tools: ToolBarLayout {

        BackToolIcon {
            onClicked: settings.setNetworkProxy()
        }
    }
    
    PageHeader {
        id: header
        
        title: root.title
    }

    Flickable {
        id: flicker

        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        contentHeight: column.height + UI.PADDING_DOUBLE

        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: UI.PADDING_DOUBLE
            }
            spacing: UI.PADDING_DOUBLE

            MySwitch {
                width: parent.width
                text: qsTr("Enable network proxy")
                checked: settings.networkProxyEnabled
                onCheckedChanged: settings.networkProxyEnabled = checked
            }

            ValueSelector {
                id: typeSelector

                x: -UI.PADDING_DOUBLE
                width: parent.width + UI.PADDING_DOUBLE * 2
                title: qsTr("Proxy type")
                model: NetworkProxyTypeModel {}
                value: settings.networkProxyType
            }

            Label {
                width: parent.width
                font.bold: true
                text: qsTr("Host")
            }

            MyTextField {
                id: hostField

                width: parent.width
                text: settings.networkProxyHost
                onTextChanged: settings.networkProxyHost = text
                onAccepted: platformCloseSoftwareInputPanel()
            }

            Label {
                width: parent.width
                font.bold: true
                text: qsTr("Port")
            }

            MyTextField {
                id: portField

                width: parent.width
                text: settings.networkProxyPort
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: settings.networkProxyPort = text
                onAccepted: platformCloseSoftwareInputPanel()
            }

            Label {
                width: parent.width
                font.bold: true
                text: qsTr("Username")
            }

            MyTextField {
                id: userField

                width: parent.width
                text: settings.networkProxyUsername
                onTextChanged: settings.networkProxyUsername = text
                onAccepted: platformCloseSoftwareInputPanel()
            }

            Label {
                width: parent.width
                font.bold: true
                text: qsTr("Password")
            }

            MyTextField {
                id: passwordField

                width: parent.width
                echoMode: TextInput.Password
                text: settings.networkProxyPassword
                onTextChanged: settings.networkProxyPassword = text
                onAccepted: platformCloseSoftwareInputPanel()
            }
        }
    }

    ScrollDecorator {
        flickableItem: flicker
    }
}
