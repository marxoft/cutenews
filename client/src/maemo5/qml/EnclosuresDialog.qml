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
import org.hildon.utils 1.0
import cuteNews 1.0

Dialog {
    id: root
    
    property variant enclosures
    
    title: qsTr("Enclosures")
    height: Math.min(360, view.count * 70 + platformStyle.paddingMedium)
    
    Action {
        id: openAction
        
        text: qsTr("Open externally")
        autoRepeat: false
        shortcut: qsTr("o")
        enabled: view.currentIndex >= 0
        onTriggered: {
            var url = enclosures[view.currentIndex].url;
            
            if (!urlopener.open(url)) {
                Qt.openUrlExternally(url);
            }            
        }
    }
    
    Action {
        id: copyAction
        
        text: qsTr("Copy URL")
        autoRepeat: false
        shortcut: qsTr("c")
        enabled: view.currentIndex >= 0
        onTriggered: clipboard.text = enclosures[view.currentIndex].url
    }
    
    Action {
        id: downloadAction
        
        text: qsTr("Download")
        autoRepeat: false
        shortcut: qsTr("d")
        enabled: view.currentIndex >= 0
        onTriggered: popups.open(downloadDialog, root)
    }
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: enclosures
        delegate: EnclosureDelegate {
            onClicked: if (!urlopener.open(modelData.url)) Qt.openUrlExternally(modelData.url);
            onPressAndHold: popups.open(contextMenu, root)
        }
    }
    
    Component {
        id: contextMenu
        
        Menu {        
            MenuItem {
                action: openAction
            }
            
            MenuItem {
                action: copyAction
            }
            
            MenuItem {
                action: downloadAction
            }
        }
    }
    
    Component {
        id: downloadDialog
        
        DownloadDialog {
            url: enclosures[view.currentIndex].url
        }
    }
}
