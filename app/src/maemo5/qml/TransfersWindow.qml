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
            onTriggered: transfers.start()
        }
        
        MenuItem {
            text: qsTr("Pause all downloads")
            onTriggered: transfers.pause()
        }
    }
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: TransferModel {
            id: transferModel
        }
        delegate: TransferDelegate {
            onClicked: popupLoader.open(propertiesDialog, root)
            onPressAndHold: contextMenu.popup()
        }
    }
    
    Label {
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No downloads")
        visible: transfers.count == 0
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: transferModel.data(view.currentIndex, "status") == Transfer.Downloading ? qsTr("Pause")
                                                                                          : qsTr("Start")
            onTriggered: transferModel.data(view.currentIndex, "status") == Transfer.Downloading
                         ? transfers.get(view.currentIndex).pause() : transfers.get(view.currentIndex).queue()
        }
        
        MenuItem {
            text: qsTr("Remove")
            onTriggered: transfers.get(view.currentIndex).cancel()
        }
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: propertiesDialog
        
        TransferPropertiesDialog {
            transfer: transfers.get(view.currentIndex);
        }
    }
}
