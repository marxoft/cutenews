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
import org.hildon.utils 1.0
import cuteNews 1.0

Window {
    id: root
    
    function load(subscriptionId) {
        articleModel.load(subscriptionId);
    }
    
    function search(query) {
        articleModel.search(query);
    }
    
    title: qsTr("Articles")
    menuBar: MenuBar {
        MenuItem {
            action: reloadAction
        }
        
        MenuItem {
            action: readAllAction
        }
    }
    
    Action {
        id: reloadAction
        
        text: qsTr("Reload")
        autoRepeat: false
        shortcut: settings.reloadShortcut
        onTriggered: articleModel.reload()
    }
    
    Action {
        id: readAllAction
        
        text: qsTr("Mark as read")
        autoRepeat: false
        shortcut: settings.markAllArticlesReadShortcut
        enabled: (articleModel.subscriptionId != ALL_ARTICLES_SUBSCRIPTION_ID)
        && (articleModel.subscriptionId != FAVOURITES_SUBSCRIPTION_ID)
        onTriggered: database.markSubscriptionRead(articleModel.subscriptionId, true)
    }

    Action {
        id: copyAction
        
        text: qsTr("Copy URL")
        autoRepeat: false
        shortcut: settings.copyShortcut
        onTriggered: if (articleView.currentIndex >= 0)
        clipboard.text = articleModel.data(articleView.currentIndex, "url");
    }
    
    Action {
        id: openAction
        
        text: qsTr("Open")
        autoRepeat: false
        shortcut: settings.openExternallyShortcut
        onTriggered: if (articleView.currentIndex >= 0) popupManager.open(Qt.resolvedUrl("OpenDialog.qml"), root,
        {url: articleModel.data(articleView.currentIndex, "url")});
    }
    
    Action {
        id: downloadAction
        
        text: qsTr("Download")
        autoRepeat: false
        shortcut: settings.downloadShortcut
        onTriggered: if (articleView.currentIndex >= 0) popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root,
        {url: articleModel.data(articleView.currentIndex, "url")});
    }
        
    Action {
        id: readAction
        
        autoRepeat: false
        shortcut: settings.toggleArticleReadShortcut
        onTriggered: if (articleView.currentIndex >= 0) articleModel.setData(articleView.currentIndex,
        !articleModel.data(articleView.currentIndex, "read"), "read");
    }
    
    Action {
        id: favouriteAction
        
        autoRepeat: false
        shortcut: settings.toggleArticleFavouriteShortcut
        onTriggered: if (articleView.currentIndex >= 0) articleModel.setData(articleView.currentIndex,
        !articleModel.data(articleView.currentIndex, "favourite"), "favourite");
    }

    Action {
        id: enclosuresAction
        
        text: qsTr("Enclosures")
        autoRepeat: false
        shortcut: settings.showArticleEnclosuresShortcut
        onTriggered: {
            if (articleView.currentIndex >= 0) {
                if (articleModel.data(articleView.currentIndex, "hasEnclosures")) {
                    popupManager.open(Qt.resolvedUrl("EnclosuresDialog.qml"), root,
                    {enclosures: articleModel.data(articleView.currentIndex, "enclosures")});
                }
                else {
                    informationBox.information(qsTr("Article has no enclosures"));
                }
            }
        }
    }
    
    Action {
        id: deleteAction
        
        text: qsTr("Delete")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        onTriggered: if (articleView.currentIndex >= 0) popupManager.open(deleteDialog, root);
    }

    ListView {
        id: articleView
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: ArticleModel {
            id: articleModel
            
            limit: 20
            onStatusChanged: {
                switch (status) {
                case ArticleModel.Active: {
                    root.showProgressIndicator = true;
                    reloadAction.enabled = false;
                    label.visible = false;
                    break;
                }
                default: {
                    root.showProgressIndicator = false;
                    reloadAction.enabled = true;
                    label.visible = (count == 0);
                    break;
                }
                }
            }
        }
        delegate: ArticleDelegate {
            onClicked: windowStack.push(articleWindow)
            onPressAndHold: popupManager.open(contextMenu, root)
        }
    }
    
    Label {
        id: label
        
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No articles")
        visible: false
    }
    
    Component {
        id: contextMenu
        
        Menu {
            MenuItem {
                action: copyAction
            }
            
            MenuItem {
                action: openAction
            }
            
            MenuItem {
                action: downloadAction
            }
                        
            MenuItem {
                text: articleModel.data(articleView.currentIndex, "read") ? qsTr("Mark as unread")
                : qsTr("Mark as read")
                action: readAction
            }
            
            MenuItem {
                text: articleModel.data(articleView.currentIndex, "favourite") ? qsTr("Unfavourite") : qsTr("Favourite")
                action: favouriteAction
            }
            
            MenuItem {
                action: enclosuresAction
            }
            
            MenuItem {
                action: deleteAction
            }
        }
    }
    
    Component {
        id: articleWindow
        
        ArticleWindow {
            id: window
            
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
                target: window
                property: "article"
                value: articleModel.get(articleView.currentIndex)
                when: window.status == WindowStatus.Active
            }
        }
    }

    Component {
        id: deleteDialog
        
        MessageBox {
            text: qsTr("Do you want to delete") + " '" + articleModel.data(articleView.currentIndex, "title") + "'?"
            onAccepted: articleModel.remove(articleView.currentIndex)
        }
    }
}
