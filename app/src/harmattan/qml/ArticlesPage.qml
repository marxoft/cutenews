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
    
    property Subscription subscription
    
    title: qsTr("Articles")
    tools: ToolBarLayout {
        BackToolIcon {}
        
        MyToolIcon {
            platformIconId: subscriptions.status == Subscriptions.Active ? "toolbar-cancle" : "toolbar-refresh"
            onClicked: subscriptions.status == Subscriptions.Active ? subscriptions.cancel() : subscriptions.update(subscription.id)
        }
        
        MyToolIcon {
            platformIconId: "toolbar-done"
            enabled: articleModel.count > 0
            onClicked: database.markSubscriptionRead(subscription.id, true)
        }
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
            count: subscription ? subscription.unreadArticles : 0
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
                text: articleModel.data(articleView.currentIndex, "favourite") ? qsTr("Unfavourite") : qsTr("Favourite")
                onClicked: database.markArticleFavourite(articleModel.data(articleView.currentIndex, "id"),
                                                         !articleModel.data(articleView.currentIndex, "favourite"))
            }
            
            MenuItem {
                text: qsTr("Share")
                onClicked: shareui.shareArticle(articleModel.get(articleView.currentIndex))
            }
        }
    }
    
    onSubscriptionChanged: if (subscription) articleModel.load(subscription.id);
}
