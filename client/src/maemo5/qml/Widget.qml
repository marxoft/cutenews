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
import org.hildon.desktop 1.0
import cuteNews 1.0

HomescreenWidget {
    id: root
        
    width: 300
    height: 332
    pluginId: "cutenews-client.desktop-0"
    settingsAvailable: true
    
    Rectangle {
        id: background
        
        anchors {
            fill: parent
            rightMargin: 1
            bottomMargin: 1
        }
        color: platformStyle.defaultBackgroundColor
        opacity: 0.8
        border {
            width: 1
            color: platformStyle.selectionColor
        }
    }
    
    ListView {
        id: view
                
        function incrementCurrentIndexInstantly() {
            highlightMoveDuration = 1;
            incrementCurrentIndex();
            highlightMoveDuration = -1;
        }
        
        function decrementCurrentIndexInstantly() {
            highlightMoveDuration = 1;
            decrementCurrentIndex();
            highlightMoveDuration = -1;
        }
        
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: buttonRow.top
            margins: 2
        }
        clip: true
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        preferredHighlightBegin: 0
        preferredHighlightEnd: 70
        highlightMoveDuration: -1
        highlightMoveSpeed: 25
        highlightRangeMode: ListView.StrictlyEnforceRange
        model: ArticleModel {
            limit: 20
            onStatusChanged: {
                switch (status) {
                    case ArticleModel.Active: {
                        reloadButton.enabled = false;
                        label.visible = false;
                        break;
                    }
                    default: {
                        reloadButton.enabled = true;
                        label.visible = (count == 0);
                        view.currentIndex = 0;
                        break;
                    }
                }
            }

            Component.onCompleted: load(ALL_ARTICLES_SUBSCRIPTION_ID)
        }
        delegate: ListItem {
            id: listItem

            style: OssoListItemStyle {
                background: ""
                backgroundSelected: ""
                itemHeight: 70
            }
            
            Loader {
                id: unreadLoader
                
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                    margins: platformStyle.paddingMedium
                }
                width: platformStyle.paddingMedium
                sourceComponent: !read ? unreadIndicator : undefined
            }

            Label {
                id: titleLabel
                
                anchors {
                    left: unreadLoader.right
                    right: parent.right
                    top: parent.top
                    margins: platformStyle.paddingMedium
                }
                elide: Text.ElideRight
                font.pointSize: platformStyle.fontSizeSmall
                text: title
            }
            
            Label {
                id: dateLabel
                
                anchors {
                    left: titleLabel.left
                    right: titleLabel.right
                    bottom: parent.bottom
                    bottomMargin: platformStyle.paddingMedium
                }
                verticalAlignment: Text.AlignBottom
                elide: Text.ElideRight
                font.pointSize: platformStyle.fontSizeSmall
                color: platformStyle.secondaryTextColor
                text: dateString
            }
            
            Component {
                id: unreadIndicator
                
                Rectangle {
                    anchors.fill: parent
                    color: listItem.pressed ? platformStyle.defaultTextColor : platformStyle.activeTextColor
                }
            }
           
            onClicked: settings.openArticlesExternallyFromWidget ? urlopener.open(url) : cutenews.showArticle(id);
        }
        
        Label {
            id: label

            anchors.centerIn: parent
            text: qsTr("No articles")
            visible: false
        }
    }
    
    ToolButtonStyle {
        id: buttonStyle
        
        background: ""
        backgroundDisabled: ""
        backgroundMarginLeft: 10
        backgroundMarginRight: 10
        backgroundMarginTop: 10
        backgroundMarginBottom: 10
    }
    
    Row {
        id: buttonRow
        
        property int buttonWidth: Math.floor(width / 3)
        
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 2
        }
        
        ToolButton {
            id: upButton
            
            width: buttonRow.buttonWidth
            height: 48
            iconName: "rss_reader_move_up"
            style: buttonStyle
            enabled: view.currentIndex > 0
            onClicked: view.decrementCurrentIndexInstantly()
        }
        
        ToolButton {
            id: reloadButton
            
            width: buttonRow.buttonWidth
            height: 48
            iconName: "general_refresh"
            style: buttonStyle
            onClicked: view.model.reload()
        }
        
        ToolButton {
            id: downButton
            
            width: buttonRow.buttonWidth
            height: 48
            iconName: "rss_reader_move_down"
            style: buttonStyle
            enabled: view.currentIndex < view.count - 4
            onClicked: view.incrementCurrentIndexInstantly()
        }
    }
    
    Timer {
        id: scrollTimer
        
        interval: 6000
        repeat: true
        running: (root.isOnCurrentHomescreen) && (settings.enableAutomaticScrollingInWidget)
                 && (view.currentIndex < view.count - 4) && (!upButton.pressed) && (!downButton.pressed)
        onTriggered: view.incrementCurrentIndex()
    }
    
    Component {
        id: settingsDialog
        
        Dialog {
            id: dialog
            
            height: column.height + platformStyle.paddingMedium
            title: qsTr("Settings")
            
            Column {
                id: column
                
                anchors {
                    left: parent.left
                    right: button.left
                    rightMargin: platformStyle.paddingMedium
                    top: parent.top
                }
                spacing: platformStyle.paddingMedium
                
                CheckBox {                    
                    width: parent.width
                    text: qsTr("Automatic scrolling")
                    checked: settings.enableAutomaticScrollingInWidget
                    onClicked: settings.enableAutomaticScrollingInWidget = checked
                }
                
                CheckBox {                    
                    width: parent.width
                    text: qsTr("Open articles externally")
                    checked: settings.openArticlesExternallyFromWidget
                    onClicked: settings.openArticlesExternallyFromWidget = checked
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
                onClicked: dialog.accept()
            }

            contentItem.states: State {
                name: "Portrait"
                when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

                AnchorChanges {
                    target: column
                    anchors.right: parent.right
                }

                PropertyChanges {
                    target: column
                    anchors.rightMargin: 0
                }

                PropertyChanges {
                    target: button
                    width: parent.width
                }

                PropertyChanges {
                    target: dialog
                    height: column.height + button.height + platformStyle.paddingMedium * 2
                }
            }
        }
    }
    
    onSettingsRequested: popupManager.open(settingsDialog, null)
}
