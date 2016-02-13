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
    
    title: qsTr("Settings")
    height: Math.min(360, column.height + platformStyle.paddingMedium)
    
    Flickable {
        id: flickable
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: column.height
        
        Column {
            id: column
    
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: platformStyle.paddingMedium
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("User interface")
                model: UserInterfaceModel {}
                value: settings.userInterface
                onSelected: settings.userInterface = value
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Article view mode")
                model: ViewModeModel {}
                value: settings.viewMode
                onSelected: settings.viewMode = value
            }
        
            ValueButton {
                width: parent.width
                text: qsTr("Download path")
                valueText: settings.downloadPath
                onClicked: popupLoader.open(fileDialog, root)
            }
        
            CheckBox {
                width: parent.width
                text: qsTr("Start downloads automatically")
                checked: settings.startTransfersAutomatically
                onClicked: settings.startTransfersAutomatically = checked
            }
        
            Button {
                width: parent.width
                text: qsTr("Network proxy")
                onClicked: popupLoader.open(proxyDialog, root)
            }
        
            Button {
                width: parent.width
                text: qsTr("Url openers")
                onClicked: popupLoader.open(urlOpenersDialog, root)
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
        text: qsTr("Done")
        onClicked: root.accept()
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: fileDialog
        
        FileDialog {
            selectFolder: true
            folder: settings.downloadPath
            onAccepted: settings.downloadPath = folder
        }
    }
    
    Component {
        id: proxyDialog
        
        NetworkProxyDialog {}
    }
    
    Component {
        id: urlOpenersDialog
        
        UrlOpenersDialog {}
    }    
}
