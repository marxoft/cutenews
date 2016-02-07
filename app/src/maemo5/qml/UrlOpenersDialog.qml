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
    
    title: qsTr("URL openers")
    height: 350
    
    ListView {
        id: view
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
        model: urlopener
        delegate: UrlOpenerDelegate {
            onClicked: dialogs.showUrlOpenerDialog(urlopener.itemData(index))
            onPressAndHold: contextMenu.popup()
        }
    }
    
    Label {
        anchors.centerIn: view
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.secondaryTextColor
        text: qsTr("No URL openers")
        visible: urlopener.count == 0
    }
        
    Menu {
        id: contextMenu
        
        MenuItem {
            text: qsTr("Edit")
            onTriggered: dialogs.showUrlOpenerDialog(urlopener.itemData(view.currentIndex))
        }
        
        MenuItem {
            text: qsTr("Delete")
            onTriggered: {
                urlopener.remove(view.currentIndex);
                urlopener.save();
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
        text: qsTr("New")
        onClicked: dialogs.showUrlOpenerDialog()
    }
    
    QtObject {
        id: dialogs
        
        property UrlOpenerDialog urlOpenerDialog
        
        function showUrlOpenerDialog(opener) {
            if (!urlOpenerDialog) {
                urlOpenerDialog = urlOpenerDialogComponent.createObject(root);
            }
            
            if (opener) {
                urlOpenerDialog.name = opener.name;
                urlOpenerDialog.regExp = opener.value.regExp;
                urlOpenerDialog.command = opener.value.command;
            }
            
            urlOpenerDialog.open();
        }
    }
    
    Component {
        id: urlOpenerDialogComponent
        
        UrlOpenerDialog {}
    }
    
    onStatusChanged: if (status == DialogStatus.Open) view.positionViewAtIndex(0, ListView.Beginning);
}
