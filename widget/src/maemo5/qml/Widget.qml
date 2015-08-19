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
import org.hildon.desktop 1.0
import cuteNews 1.0

HomescreenWidget {
    id: root
    
    property Dialog settingsDialog
    
    width: 300
    height: 332
    pluginId: "cutenews.desktop-0"
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
        highlightRangeMode: ListView.StrictlyEnforceRange
        model: ArticleModel {        
            limit: 10
        
            Component.onCompleted: load(ALL_ARTICLES_SUBSCRIPTION_ID)
        }
        delegate: Item {
            width: view.width
            height: 70
            
            Rectangle {
                anchors.fill: parent
                color: platformStyle.selectionColor
                visible: mouseArea.pressed
            }   
            
            Label {
                id: titleLabel
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: platformStyle.paddingMedium
                }
                elide: Text.ElideRight
                font.pointSize: platformStyle.fontSizeSmall
                color: (read) || (mouseArea.pressed) ? platformStyle.defaultTextColor : platformStyle.activeTextColor
                text: title
            }
    
            Label {
                id: dateLabel
                
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    margins: platformStyle.paddingMedium
                }
                verticalAlignment: Text.AlignBottom
                elide: Text.ElideRight
                font.pointSize: platformStyle.fontSizeSmall
                color: (read) || (mouseArea.pressed) ? platformStyle.secondaryTextColor : platformStyle.activeTextColor
                text: Qt.formatDateTime(date, "dd/MM/yyyy HH:mm")
            }
            
            MouseArea {
                id: mouseArea
                
                anchors.fill: parent
                onClicked: {
                    if (settings.value("Widget/openArticlesExternally", false) == true) {
                        if (!urlopener.open(url)) {
                            Qt.openUrlExternally(url);
                        }
                    }
                    else {
                        cutenews.showArticle(id);
                    }
                }
            }
        }
        
        Label {
            anchors.centerIn: parent
            text: qsTr("No articles")
            visible: (view.model.status == ArticleModel.Ready) && (view.model.count == 0)
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
            onClicked: view.currentIndex = Math.max(0, view.currentIndex - 4)
            
            Component.onCompleted: style = buttonStyle
        }
        
        ToolButton {
            id: reloadButton
            
            width: buttonRow.buttonWidth
            height: 48
            iconName: subscriptions.status == Subscriptions.Active ? "general_stop" : "general_refresh"
            onClicked: subscriptions.status == Subscriptions.Active ? subscriptions.cancel() : subscriptions.updateAll()
            
            Component.onCompleted: style = buttonStyle
        }
        
        ToolButton {
            id: downButton
            
            width: buttonRow.buttonWidth
            height: 48
            iconName: "rss_reader_move_down"
            onClicked: view.currentIndex = Math.min(view.count - 1, view.currentIndex + 4)
            
            Component.onCompleted: style = buttonStyle
        }
    }
    
    Component {
        id: settingsDialogComponent
        
        Dialog {
            id: dialog
            
            height: row.height + platformStyle.paddingMedium
            title: qsTr("Settings")
            
            Row {
                id: row
                
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                spacing: platformStyle.paddingMedium
                
                CheckBox {
                    id: checkbox
                    
                    width: parent.width - button.width - parent.spacing
                    text: qsTr("Open articles externally")
                    checked: settings.value("Widget/openArticlesExternally", false) == true
                    onClicked: settings.setValue("Widget/openArticlesExternally", checked)
                }
                
                Button {
                    id: button
                    
                    style: DialogButtonStyle {}
                    text: qsTr("Done")
                    onClicked: dialog.accept()
                }
            }
        }
    }
    
    onSettingsRequested: {
        if (!settingsDialog) {
            settingsDialog = settingsDialogComponent.createObject(root);
        }
        
        settingsDialog.open();
    }
}
