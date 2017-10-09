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
    height: 360
    
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
            
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
                text: qsTr("General")
            }
            
            CheckBox {
                width: parent.width
                text: qsTr("Update on startup")
                checked: settings.updateSubscriptionsOnStartup
                onCheckedChanged: settings.updateSubscriptionsOnStartup = checked
            }
            
            CheckBox {
                width: parent.width
                text: qsTr("Work offline")
                checked: settings.offlineModeEnabled
                onCheckedChanged: settings.offlineModeEnabled = checked
            }

            CheckBox {
                width: parent.width
                text: qsTr("Enable JavaScript in browser")
                checked: settings.enableJavaScriptInBrowser
                onCheckedChanged: settings.enableJavaScriptInBrowser = checked
            }
            
            Button {
                width: parent.width
                text: qsTr("Delete read articles")
                onClicked: popupManager.open(Qt.resolvedUrl("DeleteDialog.qml"), root)
            }
            
            Button {
                width: parent.width
                text: qsTr("Network proxy")
                onClicked: popupManager.open(Qt.resolvedUrl("NetworkProxyDialog.qml"), root)
            }
            
            Button {
                width: parent.width
                text: qsTr("Keyboard shortcuts")
                onClicked: popupManager.open(Qt.resolvedUrl("ShortcutsDialog.qml"), root)
            }
        
            Button {
                width: parent.width
                text: qsTr("URL openers")
                onClicked: popupManager.open(Qt.resolvedUrl("UrlOpenersDialog.qml"), root)
            }

            Button {
                width: parent.width
                text: qsTr("Plugins - Articles")
                onClicked: popupManager.open(articleDialog, root)
            }
            
            Button {
                width: parent.width
                text: qsTr("Plugins - Enclosures")
                onClicked: popupManager.open(enclosureDialog, root)
            }

            ListSelectorButton {
                width: parent.width
                text: qsTr("Screen orientation")
                model: ScreenOrientationModel {}
                value: settings.screenOrientation
                onSelected: settings.screenOrientation = value
            }
            
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
                text: qsTr("Downloads")
            }
        
            ValueButton {
                width: parent.width
                text: qsTr("Download path")
                valueText: settings.downloadPath
                onClicked: popupManager.open(fileDialog, root)
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Maximum concurrent downloads")
                model: ConcurrentTransfersModel {}
                value: settings.maximumConcurrentTransfers
                onSelected: settings.maximumConcurrentTransfers = value
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Custom download command (%f for filename)")
            }

            TextField {
                width: parent.width
                text: settings.customTransferCommand
                onTextChanged: settings.customTransferCommand = text
            }

            CheckBox {
                width: parent.width
                text: qsTr("Enable custom download command")
                checked: settings.customTransferCommandEnabled
                onCheckedChanged: settings.customTransferCommandEnabled = checked
            }
        
            CheckBox {
                width: parent.width
                text: qsTr("Start downloads automatically")
                checked: settings.startTransfersAutomatically
                onCheckedChanged: settings.startTransfersAutomatically = checked
            }
            
            Button {
                width: parent.width
                text: qsTr("Categories")
                onClicked: popupManager.open(Qt.resolvedUrl("CategoriesDialog.qml"), root)
            }
            
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
                text: qsTr("Logging")
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Logging verbosity")
                model: LoggerVerbosityModel {}
                value: settings.loggerVerbosity
                onSelected: settings.loggerVerbosity = value
            }
            
            Button {
                width: parent.width
                text: qsTr("View log")
                onClicked: popupManager.open(Qt.resolvedUrl("LogDialog.qml"), root)
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
    
    Component {
        id: fileDialog
        
        FileDialog {
            selectFolder: true
            folder: settings.downloadPath
            onAccepted: settings.downloadPath = folder
        }
    }

    Component {
        id: articleDialog
        
        ListPickSelector {
            title: qsTr("Plugins - Articles")
            model: PluginConfigModel {}
            textRole: "displayName"
            onSelected: {
                var plugin = model.itemData(currentIndex);
                
                if ((plugin.supportsArticles) && (plugin.articleSettings)) {
                    popupManager.open(Qt.resolvedUrl("ArticleSettingsDialog.qml"), root, {title: plugin.displayName,
                    pluginId: plugin.id, pluginSettings: plugin.articleSettings});
                }
                else {
                    informationBox.information(qsTr("No article settings for this plugin"));
                }
            }
        }
    }
    
    Component {
        id: enclosureDialog
        
        ListPickSelector {
            title: qsTr("Plugins - Enclosures")
            model: PluginConfigModel {}
            textRole: "displayName"
            onSelected: {
                var plugin = model.itemData(currentIndex);
                
                if ((plugin.supportsEnclosures) && (plugin.enclosureSettings)) {
                    popupManager.open(Qt.resolvedUrl("EnclosureSettingsDialog.qml"), root, {title: plugin.displayName,
                    pluginId: plugin.id, pluginSettings: plugin.enclosureSettings});
                }
                else {
                    informationBox.information(qsTr("No enclosure settings for this plugin"));
                }
            }
        }
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: flickable
            anchors.right: parent.right
            anchors.bottom: button.top
        }

        PropertyChanges {
            target: flickable
            anchors.rightMargin: 0
            anchors.bottomMargin: platformStyle.paddingMedium
            clip: true
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
