/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import cuteNews 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MyPage {
    id: root
    
    function showArticle(articleId) {
        appWindow.pageStack.pop(null, true);
        appWindow.pageStack.push(Qt.resolvedUrl("ArticlePage.qml"));
        var article = Qt.createQmlObject("import cuteNews 1.0; Article {}", appWindow.pageStack.currentPage);
        article.load(articleId);
        appWindow.pageStack.currentPage.article = article;
    }
    
    title: "cuteNews"
    tools: ToolBarLayout {
        MyToolIcon {
            platformIconId: "toolbar-add"
            onClicked: dialogs.showSubscriptionTypeDialog()
        }
        
        MyToolIcon {
            platformIconId: subscriptions.status == Subscriptions.Active ? "toolbar-cancle" : "toolbar-refresh"
            onClicked: subscriptions.status == Subscriptions.Active ? subscriptions.cancel() : subscriptions.updateAll()
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
                text: qsTr("Import from OPML")
                onClicked: dialogs.showImportDialog()
            }
            
            MenuItem {
                text: qsTr("Settings")
                onClicked: appWindow.pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }
    }
    
    PageHeader {
        id: header
                
        MyTextField {
            id: searchField

            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                margins: UI.PADDING_DOUBLE
            }
            placeholderText: qsTr("Search")
            clearButtonEnabled: true
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            onAccepted: {
                if (text) {
                    platformCloseSoftwareInputPanel();
                    appWindow.pageStack.push(Qt.resolvedUrl("SearchPage.qml"), {title: qsTr("Search") + " - " + text,
                                                                                query: text.substring(0, 6) == "WHERE "
                                                                                ? text : "WHERE title LIKE '%" + text + "%'"});
                    clear();
                }
            }
        }
    }
    
    ListView {
        id: subscriptionView
        
        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        model: SubscriptionModel {
            id: subscriptionModel
        }
        delegate: SubscriptionDelegate {
            onClicked: {
                switch (index) {
                case 0:
                    appWindow.pageStack.push(Qt.resolvedUrl("AllArticlesPage.qml"), {title: title});
                    break;
                case 1:
                    appWindow.pageStack.push(Qt.resolvedUrl("FavouritesPage.qml"), {title: title});
                    break;
                default:
                    appWindow.pageStack.push(Qt.resolvedUrl("ArticlesPage.qml"), {title: title,
                                             subscription: subscriptionModel.get(index)});
                    break;
                }
            }
            onPressAndHold: contextMenu.open()
        }
    }
    
    ScrollDecorator {
        flickableItem: subscriptionView
    }
    
    ContextMenu {
        id: contextMenu
        
        MenuLayout {
            MenuItem {
                text: qsTr("Update")
                onClicked: subscriptions.update(subscriptionModel.data(subscriptionView.currentIndex, "id"))
            }
            
            MenuItem {
                text: qsTr("Mark all as read")
                onClicked: database.markSubscriptionRead(subscriptionModel.data(subscriptionView.currentIndex, "id"), true)
            }
        
            MenuItem {
                text: qsTr("Edit")
                onClicked: {
                    var sourceType = subscriptionModel.data(subscriptionView.currentIndex, "sourceType");
                
                    switch (sourceType) {
                    case Subscription.Plugin:
                        dialogs.showPluginDialog(subscriptionModel.data(subscriptionView.currentIndex, "id"));
                        break;
                    default:
                        dialogs.showSubscriptionDialog(subscriptionModel.data(subscriptionView.currentIndex, "id"),
                                                       sourceType);
                        break;
                    }
                }
            }
        
            MenuItem {
                text: qsTr("Unsubscribe")
                onClicked: database.deleteSubscription(subscriptionModel.data(subscriptionView.currentIndex, "id"))
            }
        }
    }
    
    QtObject {
        id: dialogs
        
        property FileBrowserDialog importDialog
        property MySelectionDialog subscriptionTypeDialog
        property SubscriptionDialog subscriptionDialog
        property PluginDialog pluginDialog
        
        function showImportDialog() {
            if (!importDialog) {
                importDialog = importDialogComponent.createObject(root);
            }
            
            importDialog.open();
        }
        
        function showSubscriptionTypeDialog() {
            if (!subscriptionTypeDialog) {
                subscriptionTypeDialog = subscriptionTypeDialogComponent.createObject(root);
            }
            
            subscriptionTypeDialog.open();
        }
        
        function showSubscriptionDialog(subscriptionId, sourceType) {
            if (!subscriptionDialog) {
                subscriptionDialog = subscriptionDialogComponent.createObject(root);
            }
            
            subscriptionDialog.subscriptionId = (subscriptionId ? subscriptionId : -1);
            subscriptionDialog.sourceType = (sourceType ? sourceType : Subscription.Url);
            subscriptionDialog.open();
        }
        
        function showPluginDialog(subscriptionId, pluginName) {
            if (!pluginDialog) {
                pluginDialog = pluginDialogComponent.createObject(root);
            }
            
            pluginDialog.subscriptionId = (subscriptionId ? subscriptionId : -1);
            pluginDialog.pluginName = (pluginName ? pluginName : "");
            pluginDialog.open();
        }
    }
    
    Component {
        id: importDialogComponent
        
        FileBrowserDialog {
            showFiles: true
            startFolder: "/home/user/MyDocs/"
            onFileChosen: subscriptions.importFromOpml(filePath)
        }
    }
    
    Component {
        id: subscriptionTypeDialogComponent
        
        MySelectionDialog {
            titleText: qsTr("Subscription type")
            model: SubscriptionSourceTypeModel {}
            onAccepted: {
                var sourceType = model.data(selectedIndex, "value");
                
                switch (sourceType) {
                case Subscription.Plugin:
                    dialogs.showPluginDialog(-1, model.data(selectedIndex, "name"));
                    break;
                default:
                    dialogs.showSubscriptionDialog(-1, sourceType);
                    break;
                }
            }
            onStatusChanged: if (status == DialogStatus.Closed) selectedIndex = -1;
        }
    }
    
    Component {
        id: subscriptionDialogComponent
        
        SubscriptionDialog {}
    }
    
    Component {
        id: pluginDialogComponent
        
        PluginDialog {}
    }
    
    Connections {
        target: cutenews
        onArticleRequested: showArticle(articleId)
    }
    
    Component.onCompleted: subscriptionModel.load()
}
                