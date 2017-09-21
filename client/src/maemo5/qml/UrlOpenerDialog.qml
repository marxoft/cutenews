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

Dialog {
    id: root

    property alias regExp: regexpField.text
    property alias command: commandField.text

    height: column.height + platformStyle.paddingMedium
    title: qsTr("URL opener")
    
    Column {
        id: column

        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
        }
        spacing: platformStyle.paddingMedium

        Label {
            width: parent.width
            text: qsTr("Regular expression")
        }

        TextField {
            id: regexpField
            
            width: parent.width
        }

        Label {
            width: parent.width
            text: qsTr("Command (%u for URL)")
        }

        TextField {
            id: commandField
            
            width: parent.width
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
        enabled: (regexpField.text != "") && (commandField.text != "")
        onClicked: root.accept()
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: column
            anchors.right: parent.right
        }

        PropertyChanges {
            target: column
            anchors.rightMargin: 0
        }

        PropertyChanges {
            target: button
            width: parent.width
        }

        PropertyChanges {
            target: root
            height: column.height + button.height + platformStyle.paddingMedium * 2
        }
    }

    onStatusChanged: if (status == DialogStatus.Open) regexpField.forceActiveFocus();
}
