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

Window {
    id: root
    
    title: qsTr("Downloads")
    menuBar: MenuBar {
        MenuItem {
            text: qsTr("Start all downloads")
            onTriggered: downloads.start()
        }
        
        MenuItem {
            text: qsTr("Pause all downloads")
            onTriggered: downloads.pause()
        }
    }
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: TransferModel {
            id: downloadModel
        }
        delegate: DownloadDelegate {
            onClicked: dialogs.showPropertiesDialog()
            onPressAndHold: contextMenu.popup()
        }
    }
    
    Label {
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.secondaryTextColor
        text: qsTr("No downloads")
        visible: downloads.count == 0
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: downloadModel.data(view.currentIndex, "status") == Transfer.Downloading ? qsTr("Pause")
                                                                                          : qsTr("Start")
            onTriggered: downloadModel.data(view.currentIndex, "status") == Transfer.Downloading
                         ? downloads.get(view.currentIndex).pause() : downloads.get(view.currentIndex).queue()
        }
        
        MenuItem {
            text: qsTr("Remove")
            onTriggered: downloads.get(view.currentIndex).cancel()
        }
    }
    
    QtObject {
        id: dialogs
        
        property DownloadPropertiesDialog propertiesDialog
        
        function showPropertiesDialog() {
            if (!propertiesDialog) {
                propertiesDialog = propertiesDialogComponent.createObject(root);
            }
            
            propertiesDialog.open();
        }
    }
    
    Component {
        id: propertiesDialogComponent
        
        DownloadPropertiesDialog {
            onStatusChanged: {
                switch (status) {
                case DialogStatus.Opening:
                    download = downloads.get(view.currentIndex);
                    break;
                case DialogStatus.Closed:
                    download = null;
                    break;
                }
            }
        }
    }
}
