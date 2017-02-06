/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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
import com.nokia.symbian 1.1
import cuteNews 1.0

MyPage {
    id: root

    function load(subscriptionId) {
        internal.subscriptionId = subscriptionId;

        if (status == PageStatus.Active) {
            internal.load();
        }
    }
    
    function search(query) {
        internal.query = query;

        if (status == PageStatus.Active) {
            internal.load();
        }
    }
    
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "images/read.svg"
            toolTip: qsTr("Mark as read")
            enabled: (articleModel.subscriptionId != ALL_ARTICLES_SUBSCRIPTION_ID)
            && (articleModel.subscriptionId != FAVOURITES_SUBSCRIPTION_ID)
            onClicked: database.markSubscriptionRead(articleModel.subscriptionId)
        }
    }
    
    MyListView {
        id: articleView
        
        anchors.fill: parent
        model: ArticleModel {
            id: articleModel

            limit: 20
            onStatusChanged: {
                switch (status) {
                case ArticleModel.Active: {
                    root.showProgressIndicator = true;
                    label.visible = false;
                    break;
                }
                default: {
                    root.showProgressIndicator = false;
                    label.visible = (count == 0);
                    break;
                }
                }
            }
        }
        delegate: ArticleDelegate {
            onActivated: appWindow.pageStack.push(articlePage)
            onClicked: appWindow.pageStack.push(articlePage)
            onPressAndHold: popups.open(contextMenu, root)
        }
    }
    
    ScrollDecorator {
        flickableItem: articleView
    }
    
    Label {
        id: label
        
        anchors {
            fill: parent
            margins: platformStyle.paddingLarge
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        font.pixelSize: 32
        color: platformStyle.colorNormalMid
        text: qsTr("No articles")
        visible: false
    }
    
    Component {
        id: contextMenu
        
        MyContextMenu {
            focusItem: articleView
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Copy URL")
                    onClicked: clipboard.text = articleModel.data(articleView.currentIndex, "url")
                }
                
                MenuItem {
                    text: qsTr("Open externally")
                    onClicked: Qt.openUrlExternally(articleModel.data(articleView.currentIndex, "url"))
                }
                
                MenuItem {
                    text: qsTr("Download")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("DownloadPage.qml"),
                    {url: articleModel.data(articleView.currentIndex, "url")})
                }

                MenuItem {
                    text: articleModel.data(articleView.currentIndex, "read") ? qsTr("Mark as unread")
                    : qsTr("Mark as read")
                    onClicked: articleModel.setData(articleView.currentIndex,
                    !articleModel.data(articleView.currentIndex, "read"), "read")
                }
                
                MenuItem {
                    text: articleModel.data(articleView.currentIndex, "favourite") ? qsTr("Unfavourite")
                    : qsTr("Favourite")
                    onClicked: articleModel.setData(articleView.currentIndex,
                    !articleModel.data(articleView.currentIndex, "favourite"), "favourite")
                }
                
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: popups.open(deleteDialog, root)
                }
            }
        }
    }

    Component {
        id: articlePage

        ArticlePage {
            id: page

            onNext: articleView.incrementCurrentIndex()
            onNextUnread: {
                var index = articleModel.match(Math.min(articleView.currentIndex + 1, articleView.count - 1),
                "read", false);
                
                if (index != -1) {
                    articleView.currentIndex = index;
                }
            }
            onPrevious: articleView.decrementCurrentIndex()

            Binding {
                target: page
                property: "article"
                value: articleModel.get(articleView.currentIndex)
                when: page.status == PageStatus.Active
            }
        }
    }
    
    Component {
        id: deleteDialog
        
        MyQueryDialog {
            focusItem: articleView
            titleText: qsTr("Delete?")
            message: qsTr("Do you want to delete") + " '" + articleModel.data(articleView.currentIndex, "title") + "'?"
            onAccepted: articleModel.remove(articleView.currentIndex)
        }
    }

    QtObject {
        id: internal

        property string subscriptionId
        property string query
        property bool loaded: false

        function load() {
            if (query) {
                articleModel.search(query);
            }
            else {
                articleModel.load(subscriptionId);
            }

            loaded = true;
        }

        onSubscriptionIdChanged: loaded = false
        onQueryChanged: loaded = false
    }

    onStatusChanged: if ((status == PageStatus.Active) && (!internal.loaded)) internal.load();
}
