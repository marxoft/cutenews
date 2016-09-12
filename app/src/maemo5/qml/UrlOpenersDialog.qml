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
            onClicked: {
                var dialog = popupLoader.load(urlOpenerDialog, root);
                var opener = urlopener.itemData(index).value;
                dialog.name = opener.name;
                dialog.regExp = opener.regExp;
                dialog.command = opener.command;
                dialog.open();
            }
            onPressAndHold: contextMenu.popup()
        }
    }
    
    Label {
        anchors.centerIn: view
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No URL openers")
        visible: urlopener.count == 0
    }
        
    Menu {
        id: contextMenu
        
        MenuItem {
            text: qsTr("Edit")
            onTriggered: {
                var dialog = popupLoader.load(urlOpenerDialog, root);
                var opener = urlopener.itemData(view.currentIndex).value;
                dialog.name = opener.name;
                dialog.regExp = opener.regExp;
                dialog.command = opener.command;
                dialog.open();
            }
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
        onClicked: popupLoader.open(urlOpenerDialog, root)
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: urlOpenerDialog
        
        UrlOpenerDialog {
            onAccepted: {
                var i = urlopener.match(0, "name", name);
                
                if (i >= 0) {
                    urlopener.setItemData(i, {value: {name: name, regExp: regExp, command: command}});
                }
                else {
                    urlopener.append(name, {name: name, regExp: regExp, command: command});
                }
                
                urlopener.save();
            }
        }
    }    
}
