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
                text: qsTr("Subscriptions")
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
            
            Button {
                width: parent.width
                text: qsTr("Delete read articles")
                onClicked: popups.open(deleteDialog, root)
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
                onClicked: popups.open(fileDialog, root)
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Maximum concurrent downloads")
                model: ConcurrentTransfersModel {}
                value: settings.maximumConcurrentTransfers
                onSelected: settings.maximumConcurrentTransfers = value
            }
        
            CheckBox {
                width: parent.width
                text: qsTr("Start downloads automatically")
                checked: settings.startTransfersAutomatically
                onClicked: settings.startTransfersAutomatically = checked
            }
            
            Button {
                width: parent.width
                text: qsTr("Categories")
                onClicked: popups.open(categoriesDialog, root)
            }
            
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
                text: qsTr("Other")
            }
        
            Button {
                width: parent.width
                text: qsTr("Network proxy")
                onClicked: popups.open(proxyDialog, root)
            }
        
            Button {
                width: parent.width
                text: qsTr("Url openers")
                onClicked: popups.open(urlOpenersDialog, root)
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
                onClicked: popups.open(logDialog, root)
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
        id: deleteDialog
        
        DeleteDialog {}
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
        id: categoriesDialog
        
        CategoriesDialog {}
    }
    
    Component {
        id: proxyDialog
        
        NetworkProxyDialog {}
    }
    
    Component {
        id: urlOpenersDialog
        
        UrlOpenersDialog {}
    }
    
    Component {
        id: logDialog
        
        LogDialog {}
    }
}
