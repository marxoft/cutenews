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

ApplicationWindow {
    id: appWindow
        
    visible: true
    title: "cuteNews"
    showProgressIndicator: subscriptions.status == Subscriptions.Active
    menuBar: MenuBar {
        MenuItem {
            text: qsTr("Import from OPML")
            onTriggered: popupLoader.open(importDialog, appWindow)
        }
        
        MenuItem {
            text: qsTr("Downloads") + " (" + transfers.count + ")"
            onTriggered: windowStack.push(Qt.resolvedUrl("TransfersWindow.qml"))
        }
        
        MenuItem {
            text: qsTr("Search")
            enabled: settings.userInterface == "touch"
            onTriggered: popupLoader.open(searchDialog, appWindow)
        }
        
        MenuItem {
            text: qsTr("Settings")
            onTriggered: popupLoader.open(settingsDialog, appWindow)
        }
        
        MenuItem {
            text: qsTr("About")
            onTriggered: popupLoader.open(aboutDialog, appWindow)
        }
    }
    
    Loader {
        id: loader
        
        function loadUi() {
            if (settings.userInterface == "osso") {
                source = Qt.resolvedUrl("OssoView.qml");
            }
            else {
                source = Qt.resolvedUrl("TouchView.qml");
            }
        }
        
        anchors.fill: parent
    }
    
    InformationBox {
        id: informationBox
        
        function information(message) {
            informationLabel.text = message;
            open();
        }
        
        height: informationLabel.height + platformStyle.paddingLarge
        
        Label {
            id: informationLabel
            
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                margins: platformStyle.paddingLarge
            }
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: platformStyle.reversedTextColor
        }
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: importDialog
        
        FileDialog {
            nameFilters: ["*.opml"]
            onAccepted: subscriptions.importFromOpml(filePath)
        }
    }
        
    Component {
        id: searchDialog
        
        SearchDialog {
            onAccepted: windowStack.push(Qt.resolvedUrl("SearchWindow.qml"), {title: qsTr("Search") + " - " + query,
                                         query: query.substring(0, 6) == "WHERE " ? query
                                         : "WHERE title LIKE '%" + query + "%'"})
        }
    }
    
    Component {
        id: settingsDialog
        
        SettingsDialog {}
    }
    
    Component {
        id: aboutDialog
        
        AboutDialog {}
    }
    
    Connections {
        target: database
        onFinished: {
            if (database.status == DBConnection.Error) {
                informationBox.information(database.errorString);
            }
        }
    }
    
    Connections {
        target: settings
        onUserInterfaceChanged: loader.loadUi()
    }
    
    Component.onCompleted: {
        loader.loadUi();
        urlopener.load();
    }
}
                
