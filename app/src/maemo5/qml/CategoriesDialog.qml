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
    
    title: qsTr("Categories")
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
        model: CategoryModel {
            id: categoryModel
        }
        delegate: CategoryDelegate {
            onClicked: popupManager.open(categoryDialog, root, {name: name, path: value})
            onPressAndHold: popupManager.open(contextMenu, root)
        }
    }
    
    Label {
        anchors.centerIn: view
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No categories")
        visible: categoryModel.count == 0
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
        onClicked: popupManager.open(categoryDialog, root)
    }
    
    Action {
        id: editAction
        
        text: qsTr("Edit")
        autoRepeat: false
        shortcut: settings.editShortcut
        enabled: view.currentIndex >= 0
        onTriggered: popupManager.open(categoryDialog, root, {name: categoryModel.data(view.currentIndex, "name"),
        path: categoryModel.data(view.currentIndex, "value")})
    }
    
    Action {
        id: removeAction
        
        text: qsTr("Remove")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        enabled: view.currentIndex >= 0
        onTriggered: {
            categoryModel.remove(view.currentIndex);
            categoryModel.save();
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
        id: categoryDialog
        
        CategoryDialog {
            onAccepted: {
                categoryModel.append(name, path);
                categoryModel.save();
            }
        }
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: view
            anchors.right: parent.right
            anchors.bottom: button.top
        }

        PropertyChanges {
            target: view
            anchors.rightMargin: 0
            anchors.bottomMargin: platformStyle.paddingMedium
        }

        PropertyChanges {
            target: button
            width: parent.width
        }

        PropertyChanges {
            target: root
            height: 680
        }
    }
}
