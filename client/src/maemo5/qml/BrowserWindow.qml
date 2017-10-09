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
import org.hildon.utils 1.0
import org.hildon.webkit 1.0

Window {
    id: root

    property alias url: view.url
    
    title: view.title ? view.title : qsTr("Browser")
    showProgressIndicator: view.status == WebView.Loading
    menuBar: MenuBar {
        MenuItem {
            action: reloadAction
        }

        MenuItem {
            action: copyAction
        }

        MenuItem {
            action: openAction
        }
        
        MenuItem {
            action: downloadAction
        }
    }

    Action {
        id: reloadAction

        text: qsTr("Reload")
        autoRepeat: false
        shortcut: settings.reloadShortcut
        enabled: view.status != WebView.Loading
        onTriggered: view.reload()
    }
    
    Action {
        id: copyAction
        
        text: qsTr("Copy URL")
        autoRepeat: false
        shortcut: settings.copyShortcut
        onTriggered: clipboard.text = view.url
    }
    
    Action {
        id: openAction
        
        text: qsTr("Open")
        autoRepeat: false
        shortcut: settings.openExternallyShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("OpenDialog.qml"), root, {url: view.url})
    }
    
    Action {
        id: downloadAction
        
        text: qsTr("Download")
        autoRepeat: false
        shortcut: settings.downloadShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root, {url: view.url})
    }
        
    Flickable {
        id: flickable
        
        anchors.fill: parent
        contentWidth: view.width
        contentHeight: view.height
        pressDelay: 100000
        focus: true
        
        WebView {
            id: view
            
            preferredWidth: flickable.width
            contextMenuPolicy: Qt.CustomContextMenu
            settings.javascriptEnabled: settings.enableJavaScriptInBrowser
            onCustomContextMenuRequested: {
                var result = hitTestContent(menuX, menuY);
                var link = result.linkUrl.toString();

                if (link) {
                    popupManager.open(urlMenu, root, {url: link});
                }
            }
        }

        Keys.onPressed: {
            if (!event.isAutoRepeat) {
                switch (event.key) {
                    case Qt.Key_F7:
                        view.zoomFactor += 0.1;
                        break;
                    case Qt.Key_F8:
                        view.zoomFactor -= 0.1;
                        break;
                    default:
                        return;
                }

                event.accepted = true;
            }
        }
    }
    
    Component {
        id: urlMenu
        
        Menu {
            id: menu
            
            property string url
            
            MenuItem {
                text: qsTr("Copy URL")
                onTriggered: clipboard.text = menu.url
            }

            MenuItem {
                text: qsTr("Open")
                onTriggered: popupManager.open(Qt.resolvedUrl("OpenDialog.qml"), root, {url: menu.url})
            }

            MenuItem {
                text: qsTr("Download")
                onTriggered: popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root, {url: menu.url})
            }
        }
    }
    
    VolumeKeys.enabled: settings.volumeKeysEnabled
}
