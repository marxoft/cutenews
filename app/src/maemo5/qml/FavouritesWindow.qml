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

Window {
    id: root
    
    title: qsTr("Favourites")
    showProgressIndicator: (articleModel.status == ArticleModel.Active)
                           || (subscriptions.status == Subscriptions.Active)
    
    ListView {
        id: articleView
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: ArticleModel {
            id: articleModel
            
            limit: 20
            onStatusChanged: if (status == ArticleModel.Error) informationBox.information(errorString);
        }
        delegate: ArticleDelegate {
            onClicked: {
                windowStack.push(Qt.resolvedUrl("ArticleWindow.qml"));
                windowStack.currentWindow.article = articleModel.get(index);
            }
            onPressAndHold: contextMenu.popup()
        }
    }
    
    Label {
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No articles")
        visible: (articleModel.status == ArticleModel.Ready) && (articleModel.count == 0)
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: qsTr("Open externally")
            onTriggered: {
                var url = articleModel.data(articleView.currentIndex, "url");
                
                if (!urlopener.open(url)) {
                    Qt.openUrlExternally(url);
                }
            }
        }
        
        MenuItem {
            text: articleModel.data(articleView.currentIndex, "read") ? qsTr("Mark as unread") : qsTr("Mark as read")
            onTriggered: database.markArticleRead(articleModel.data(articleView.currentIndex, "id"),
                                                  !articleModel.data(articleView.currentIndex, "read"))
        }
        
        MenuItem {
            text: qsTr("Unfavourite")
            onTriggered: database.markArticleFavourite(articleModel.data(articleView.currentIndex, "id"), false)
        }
        
        MenuItem {
            text: qsTr("Delete")
            onTriggered: database.deleteArticle(articleModel.data(articleView.currentIndex, "id"))
        }
    }
    
    Component.onCompleted: articleModel.load(FAVOURITES_SUBSCRIPTION_ID)
}
