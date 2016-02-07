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

import QtQuick 1.1
import com.nokia.meego 1.0
import cuteNews 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MyPage {
    id: root
    
    property Article article
    
    title: article ? article.title : qsTr("Article")
    tools: ToolBarLayout {
        BackToolIcon {            
            Component.onCompleted: {
                if (appWindow.pageStack.depth > 1) {
                    platformIconId = "toolbar-back";
                }
                else {
                    iconSource = "images/icon-view-main.png";
                }
            }
        }
                
        MyToolIcon {
            platformIconId: (article) && (article.favourite) ? "toolbar-favorite-mark" : "toolbar-favorite-unmark"
            onClicked: database.markArticleFavourite(article.id, !article.favourite)
        }
        
        MyToolIcon {
            platformIconId: "toolbar-view-menu"
            onClicked: menu.open()
        }
    }
    
    Menu {
        id: menu
        
        MenuLayout {
            MenuItem {
                text: (article) && (article.read) ? qsTr("Mark as unread") : qsTr("Mark as read")
                onClicked: database.markArticleRead(article.id, !article.read)
            }
            
            MenuItem {
                text: qsTr("Share")
                onClicked: shareui.shareArticle(article)
            }
        }
    }
    
    Flickable {
        id: flickable
        
        anchors.fill: parent       
        contentHeight: column.height + UI.PADDING_DOUBLE * 2
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: UI.PADDING_DOUBLE
            }
            spacing: UI.PADDING_DOUBLE
            
            Label {
                width: parent.width
                text: article ? article.title : ""
                font.pixelSize: UI.FONT_XLARGE
            }
            
            Label {
                width: parent.width
                font.pixelSize: UI.FONT_SMALL
                font.family: UI.FONT_FAMILY_LIGHT
                text: (article) && (article.author) ? article.author : qsTr("Unknown author")
            }
            
            Label {
                width: parent.width
                font.pixelSize: UI.FONT_SMALL
                font.family: UI.FONT_FAMILY_LIGHT
                text: article ? Qt.formatDateTime(article.date, "dd/MM/yyyy HH:mm") : ""
            }
            
            Image {
                width: parent.width
                source: "image://theme/meegotouch-groupheader-inverted-background"
            }
            
            Label {        
                width: parent.width
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                text: article ? article.body : ""
                onLinkActivated: if (!urlopener.open(link)) Qt.openUrlExternally(link);
            }
            
            MyButton {
                x: Math.floor((parent.width - width) / 2)
                width: 250
                text: qsTr("Open externally")
                onClicked: if (!urlopener.open(article.url)) Qt.openUrlExternally(article.url);
            }
            
            Item {
                width: parent.width
                height: visible ? enclosuresColumn.height : 0
                visible: (article) && (article.hasEnclosures) ? true : false
                
                Column {
                    id: enclosuresColumn
                    
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                    }
                    
                    SeparatorLabel {
                        width: parent.width
                        text: qsTr("Enclosures")
                    }
            
                    Item {
                        width: parent.width
                        height: UI.PADDING_DOUBLE
                    }
            
                    Repeater {
                        id: repeater
                
                        property int currentIndex: -1
                
                        model: article ? article.enclosures : null
                
                        EnclosureDelegate {
                            x: -UI.PADDING_DOUBLE
                            width: enclosuresColumn.width + UI.PADDING_DOUBLE * 2
                            onClicked: if (!urlopener.open(modelData.url)) Qt.openUrlExternally(modelData.url);
                            onPressAndHold: {
                                repeater.currentIndex = -1;
                                repeater.currentIndex = index;
                                contextMenu.open();
                            }
                        }
                    }
                }
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    ContextMenu {
        id: contextMenu
        
        MenuLayout {
            MenuItem {
                text: qsTr("Open externally")
                onClicked: {
                    var url = repeater.model[repeater.currentIndex].url;
                    
                    if (!urlopener.open(url)) {
                        Qt.openUrlExternally(url);
                    }
                }
            }
            
            MenuItem {
                text: qsTr("Download")
                onClicked: {
                    downloads.addDownloadTransfer(repeater.model[repeater.currentIndex].url, article.subscriptionId);
                    infoBanner.displayMessage(qsTr("Download added"));
                }
            }
        }
    }
    
    onArticleChanged: if ((article) && (!article.read)) database.markArticleRead(article.id, true);
}
    