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
    
    title: qsTr("Keyboard shortcuts")
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

            CheckBox {
                width: parent.width
                text: qsTr("Use volume keys to change articles")
                checked: settings.volumeKeysEnabled
                onCheckedChanged: settings.volumeKeysEnabled = checked
            }
            
            Label {
                width: parent.width
                text: qsTr("Update subscription")
            }
            
            TextField {
                width: parent.width
                text: settings.updateSubscriptionShortcut
                onTextChanged: settings.updateSubscriptionShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Update all subscriptions")
            }
            
            TextField {
                width: parent.width
                text: settings.updateAllSubscriptionsShortcut
                onTextChanged: settings.updateAllSubscriptionsShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Import subscriptions")
            }
            
            TextField {
                width: parent.width
                text: settings.importSubscriptionsShortcut
                onTextChanged: settings.importSubscriptionsShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Mark subscription read")
            }
            
            TextField {
                width: parent.width
                text: settings.markSubscriptionReadShortcut
                onTextChanged: settings.markSubscriptionReadShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Mark all subscriptions read")
            }
            
            TextField {
                width: parent.width
                text: settings.markAllSubscriptionsReadShortcut
                onTextChanged: settings.markAllSubscriptionsReadShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Mark all articles read")
            }
            
            TextField {
                width: parent.width
                text: settings.markAllArticlesReadShortcut
                onTextChanged: settings.markAllArticlesReadShortcut = text
            }
                        
            Label {
                width: parent.width
                text: qsTr("Toggle article read")
            }
            
            TextField {
                width: parent.width
                text: settings.toggleArticleReadShortcut
                onTextChanged: settings.toggleArticleReadShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Toggle article favourite")
            }
            
            TextField {
                width: parent.width
                text: settings.toggleArticleFavouriteShortcut
                onTextChanged: settings.toggleArticleFavouriteShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Show article enclosures")
            }
            
            TextField {
                width: parent.width
                text: settings.showArticleEnclosuresShortcut
                onTextChanged: settings.showArticleEnclosuresShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Show previous article")
            }
            
            TextField {
                width: parent.width
                text: settings.previousArticleShortcut
                onTextChanged: settings.previousArticleShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Show next article")
            }
            
            TextField {
                width: parent.width
                text: settings.nextArticleShortcut
                onTextChanged: settings.nextArticleShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Show next unread article")
            }
            
            TextField {
                width: parent.width
                text: settings.nextUnreadArticleShortcut
                onTextChanged: settings.nextUnreadArticleShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Show downloads")
            }
            
            TextField {
                width: parent.width
                text: settings.transfersShortcut
                onTextChanged: settings.transfersShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Start download")
            }
            
            TextField {
                width: parent.width
                text: settings.startTransferShortcut
                onTextChanged: settings.startTransferShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Start all downloads")
            }
            
            TextField {
                width: parent.width
                text: settings.startAllTransfersShortcut
                onTextChanged: settings.startAllTransfersShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Pause download")
            }
            
            TextField {
                width: parent.width
                text: settings.pauseTransferShortcut
                onTextChanged: settings.pauseTransferShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Pause all downloads")
            }
            
            TextField {
                width: parent.width
                text: settings.pauseAllTransfersShortcut
                onTextChanged: settings.pauseAllTransfersShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Set download category")
            }
            
            TextField {
                width: parent.width
                text: settings.transferCategoryShortcut
                onTextChanged: settings.transferCategoryShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Set download priority")
            }
            
            TextField {
                width: parent.width
                text: settings.transferPriorityShortcut
                onTextChanged: settings.transferPriorityShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Search")
            }
            
            TextField {
                width: parent.width
                text: settings.searchShortcut
                onTextChanged: settings.searchShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Settings")
            }
            
            TextField {
                width: parent.width
                text: settings.settingsShortcut
                onTextChanged: settings.settingsShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Copy content URL")
            }
            
            TextField {
                width: parent.width
                text: settings.copyShortcut
                onTextChanged: settings.copyShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Open content externally")
            }
            
            TextField {
                width: parent.width
                text: settings.openExternallyShortcut
                onTextChanged: settings.openExternallyShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Download content")
            }
            
            TextField {
                width: parent.width
                text: settings.downloadShortcut
                onTextChanged: settings.downloadShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("New content")
            }
            
            TextField {
                width: parent.width
                text: settings.newContentShortcut
                onTextChanged: settings.newContentShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Edit content")
            }
            
            TextField {
                width: parent.width
                text: settings.editShortcut
                onTextChanged: settings.editShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Delete content")
            }
            
            TextField {
                width: parent.width
                text: settings.deleteShortcut
                onTextChanged: settings.deleteShortcut = text
            }
            
            Label {
                width: parent.width
                text: qsTr("Reload content")
            }
            
            TextField {
                width: parent.width
                text: settings.reloadShortcut
                onTextChanged: settings.reloadShortcut = text
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
