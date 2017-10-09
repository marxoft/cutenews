/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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
    
    property alias url: textField.text
    
    title: qsTr("Fetch article")
    height: textField.height + platformStyle.paddingMedium

    TextField {
        id: textField
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
        }
        placeholderText: qsTr("URL")
        onAccepted: if (text) root.accept();
    }

    Button {
        id: button
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        style: DialogButtonStyle {}
        text: qsTr("Done")
        enabled: textField.text != ""
        onClicked: textField.accepted()
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: textField
            anchors.right: parent.right
        }

        PropertyChanges {
            target: textField
            anchors.rightMargin: 0
        }

        PropertyChanges {
            target: button
            width: parent.width
        }

        PropertyChanges {
            target: root
            height: textField.height + button.height + platformStyle.paddingMedium * 2
        }
    }
    
    onStatusChanged: if (status == DialogStatus.Open) textField.forceActiveFocus();
    onAccepted: {
        if (plugins.articleIsSupported(url)) {
            windowStack.push(Qt.resolvedUrl("ArticleRequestWindow.qml"), {url: url});
        }
        else {
            informationBox.information(qsTr("No plugin found for this article"));
        }
    }
}
