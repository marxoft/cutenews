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

    property alias name: nameField.text
    property alias regExp: regexpField.text
    property alias command: commandField.text

    height: column.height + platformStyle.paddingMedium
    title: qsTr("Add URL opener")
    
    Column {
        id: column

        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            bottom: parent.bottom
        }
        spacing: platformStyle.paddingMedium

        Label {
            width: parent.width
            text: qsTr("Name")
        }

        TextField {
            id: nameField
            
            width: parent.width
        }

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
            text: qsTr("Command") + " (" + qsTr("replace URL with") + " '%URL%')"
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
        enabled: (nameField.text != "") && (regexpField.text != "") && (commandField.text != "")
        onClicked: {
            var i = urlopener.match("name", nameField.text);
            
            if (i >= 0) {
                urlopener.setItemData(i, {value: {name: nameField.text, regExp: regexpField.text,
                                          command: commandField.text}});
            }
            else {
                urlopener.append(nameField.text, {name: nameField.text, regExp: regexpField.text,
                                 command: commandField.text});
            }
            
            urlopener.save();
            root.accept();
        }
    }

    onStatusChanged: {
        switch (status) {
        case DialogStatus.Open:
            nameField.forceActiveFocus();
            break;
        case DialogStatus.Closed: {
            nameField.clear();
            regexpField.clear();
            commandField.clear();
            break;
        }
        default:
            break;
        }
    }
}
