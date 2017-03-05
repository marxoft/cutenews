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
    height: 360
    
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
            onClicked: popupManager.open(urlOpenerDialog, root, {regExp: name, command: value})
            onPressAndHold: popupManager.open(contextMenu, root)
        }
    }
    
    Label {
        anchors.centerIn: view
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No URL openers")
        visible: urlopener.count == 0
    }
        
    Button {
        id: button
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        style: DialogButtonStyle {}
        text: qsTr("New")
        shortcut: settings.newContentShortcut
        onClicked: popupManager.open(urlOpenerDialog, root)
    }
    
    Action {
        id: editAction
        
        text: qsTr("Edit")
        autoRepeat: false
        shortcut: settings.editShortcut
        enabled: view.currentIndex >= 0
        onTriggered: popupManager.open(urlOpenerDialog, root, {regExp: urlopener.data(view.currentIndex, "name"),
        command: urlopener.data(view.currentIndex, "value")})
    }
    
    Action {
        id: removeAction
        
        text: qsTr("Remove")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        enabled: view.currentIndex >= 0
        onTriggered: {
            urlopener.remove(view.currentIndex);
            urlopener.save();
        }
    }
    
    Component {
        id: contextMenu
        
        Menu {        
            MenuItem {
                action: editAction
            }
            
            MenuItem {
                action: removeAction
            }
        }
    }
    
    Component {
        id: urlOpenerDialog
        
        UrlOpenerDialog {
            onAccepted: {
                urlopener.append(regExp, command);
                urlopener.save();
            }
        }
    }    
}
