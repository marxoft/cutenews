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
    
    property alias query: searchField.text
    
    title: qsTr("Search")
    height: searchField.height + platformStyle.paddingMedium

    TextField {
        id: searchField
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            bottom: parent.bottom
        }
        placeholderText: qsTr("Search query")
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
        enabled: searchField.text != ""
        onClicked: searchField.accepted()
    }
    
    onStatusChanged: if (status == DialogStatus.Open) searchField.forceActiveFocus();
    onAccepted: windowStack.push(Qt.resolvedUrl("ArticlesWindow.qml"), {title: qsTr("Search") + " - " + query})
    .search(query)
}
