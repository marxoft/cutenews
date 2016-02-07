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
    
    title: qsTr("Favourites")
    tools: ToolBarLayout {
        BackToolIcon {}
    }

    ListView {
        id: articleView
        
        anchors.fill: parent
        model: ArticleModel {
            id: articleModel
            
            limit: 20
            onStatusChanged: if (status == ArticleModel.Error) infoBanner.displayMessage(errorString);
        }
        header: ArticlesHeader {
            title: root.title
        }
        delegate: ArticleDelegate {
            onClicked: {
                appWindow.pageStack.push(Qt.resolvedUrl("ArticlePage.qml"));
                appWindow.pageStack.currentPage.article = articleModel.get(index);
            }
            onPressAndHold: contextMenu.open()
        }
    }
    
    ScrollDecorator {
        flickableItem: articleView
    }
    
    Label {
        anchors.centerIn: parent
        font.pixelSize: 48
        color: UI.COLOR_INVERTED_SECONDARY_FOREGROUND
        text: qsTr("No articles")
        visible: (articleModel.status == ArticleModel.Ready) && (articleModel.count == 0)
    }
    
    ContextMenu {
        id: contextMenu
        
        MenuLayout {
            MenuItem {
                text: qsTr("Open externally")
                onClicked: {
                    var url = articleModel.data(articleView.currentIndex, "url");
                
                    if (!urlopener.open(url)) {
                        Qt.openUrlExternally(url);
                    }
                }
            }
        
            MenuItem {
                text: articleModel.data(articleView.currentIndex, "read") ? qsTr("Mark as unread") : qsTr("Mark as read")
                onClicked: database.markArticleRead(articleModel.data(articleView.currentIndex, "id"),
                                                    !articleModel.data(articleView.currentIndex, "read"))
            }
        
            MenuItem {
                text: qsTr("Unfavourite")
                onClicked: database.markArticleFavourite(articleModel.data(articleView.currentIndex, "id"), false)
            }
            
            MenuItem {
                text: qsTr("Share")
                onClicked: shareui.shareArticle(articleModel.get(articleView.currentIndex))
            }
        }
    }
    
    Component.onCompleted: articleModel.load(FAVOURITES_SUBSCRIPTION_ID)
}
