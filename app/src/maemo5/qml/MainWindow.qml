/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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
            onTriggered: dialogs.showImportDialog()
        }
        
        MenuItem {
            text: qsTr("Downloads") + " (" + downloads.count + ")"
            onTriggered: windowStack.push(Qt.resolvedUrl("DownloadsWindow.qml"))
        }
        
        MenuItem {
            text: qsTr("Search")
            enabled: settings.userInterface == "touch"
            onTriggered: dialogs.showSearchDialog()
        }
        
        MenuItem {
            text: qsTr("Settings")
            onTriggered: dialogs.showSettingsDialog()
        }
        
        MenuItem {
            text: qsTr("About")
            onTriggered: dialogs.showAboutDialog()
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
    
    QtObject {
        id: dialogs
        
        property FileDialog importDialog
        property SubscriptionDialog subscriptionDialog
        property SearchDialog searchDialog
        property SettingsDialog settingsDialog
        property AboutDialog aboutDialog
        
        function showImportDialog() {
            if (!importDialog) {
                importDialog = importDialogComponent.createObject(appWindow);
            }
            
            importDialog.open();
        }
        
        function showSubscriptionDialog(subscriptionId) {
            if (!subscriptionDialog) {
                subscriptionDialog = subscriptionDialogComponent.createObject(appWindow);
            }
            
            subscriptionDialog.subscriptionId = subscriptionId;
            subscriptionDialog.open();
        }
        
        function showSearchDialog() {
            if (!searchDialog) {
                searchDialog = searchDialogComponent.createObject(appWindow);
            }
            
            searchDialog.open();
        }
        
        function showSettingsDialog() {
            if (!settingsDialog) {
                settingsDialog = settingsDialogComponent.createObject(appWindow);
            }
            
            settingsDialog.open();
        }
        
        function showAboutDialog() {
            if (!aboutDialog) {
                aboutDialog = aboutDialogComponent.createObject(appWindow);
            }
            
            aboutDialog.open();
        }
    }
    
    Component {
        id: importDialogComponent
        
        FileDialog {
            nameFilters: ["*.opml"]
            onAccepted: subscriptions.importFromOpml(filePath)
        }
    }
    
    Component {
        id: subscriptionDialogComponent
        
        SubscriptionDialog {}
    }
    
    Component {
        id: searchDialogComponent
        
        SearchDialog {
            onAccepted: windowStack.push(Qt.resolvedUrl("SearchWindow.qml"), {title: qsTr("Search") + " - " + query,
                                         query: query.substring(0, 6) == "WHERE " ? query
                                         : "WHERE title LIKE '%" + query + "%'"})
        }
    }
    
    Component {
        id: settingsDialogComponent
        
        SettingsDialog {}
    }
    
    Component {
        id: aboutDialogComponent
        
        AboutDialog {}
    }
    
    Connections {
        target: database
        onError: informationBox.information(qsTr("Database error") + ": " + errorString)
    }
    
    Connections {
        target: settings
        onUserInterfaceChanged: loader.loadUi()
    }
    
    Component.onCompleted: loader.loadUi();
}
                