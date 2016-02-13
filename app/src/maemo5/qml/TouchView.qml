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

Item {
    id: root
    
    function showArticle(articleId) {
        windowStack.clear();
        windowStack.push(Qt.resolvedUrl("ArticleWindow.qml"));
        var article = Qt.createQmlObject("import cuteNews 1.0; Article {}", windowStack.currentWindow);
        article.load(articleId);
        windowStack.currentWindow.article = article;
    }
    
    anchors.fill: parent
    
    Button {
        id: subscriptionButton
        
        anchors {
            left: parent.left
            leftMargin: platformStyle.paddingMedium
            right: parent.horizontalCenter
            rightMargin: platformStyle.paddingMedium / 2
            top: parent.top
            topMargin: platformStyle.paddingMedium
        }
        text: qsTr("New subscription")
        iconName: "general_add"
        activeFocusOnPress: false
        onClicked: popupLoader.open(subscriptionTypeDialog, root)
    }
    
    Button {
        id: updateButton
        
        anchors {
            left: parent.horizontalCenter
            leftMargin: platformStyle.paddingMedium / 2
            right: parent.right
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            topMargin: platformStyle.paddingMedium
        }
        text: subscriptions.status == Subscriptions.Active ? qsTr("Cancel update") : qsTr("Update all")
        iconName: subscriptions.status == Subscriptions.Active ? "general_stop" : "general_refresh"
        activeFocusOnPress: false
        onClicked: subscriptions.status == Subscriptions.Active ? subscriptions.cancel() : subscriptions.updateAll()
    }
    
    ListView {
        id: subscriptionView
        
        anchors {
            left: parent.left
            right: parent.right
            top: subscriptionButton.bottom
            topMargin: platformStyle.paddingMedium
            bottom: parent.bottom
        }
        clip: true
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: SubscriptionModel {
            id: subscriptionModel
            
            onStatusChanged: if (status == SubscriptionModel.Error) informationBox.information(errorString);
        }
        delegate: SubscriptionDelegate {
            onClicked: {
                switch (index) {
                case 0:
                    windowStack.push(Qt.resolvedUrl("AllArticlesWindow.qml"), {title: title});
                    break;
                case 1:
                    windowStack.push(Qt.resolvedUrl("FavouritesWindow.qml"), {title: title});
                    break;
                default:
                    windowStack.push(Qt.resolvedUrl("ArticlesWindow.qml"), {title: title, subscriptionId: id});
                    break;
                }
            }       
            onPressAndHold: if (index > 1) contextMenu.popup();
        }
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: qsTr("Update")
            onTriggered: subscriptions.update(subscriptionModel.data(subscriptionView.currentIndex, "id"))
        }
        
        MenuItem {
            text: qsTr("Mark all as read")
            onTriggered: database.markSubscriptionRead(subscriptionModel.data(subscriptionView.currentIndex, "id"), true)
        }
        
        MenuItem {
            text: qsTr("Properties")
            onTriggered: {
                var subscription = subscriptionModel.itemData(subscriptionView.currentIndex);
                
                switch (subscription.sourceType) {
                case Subscription.Plugin: {
                    var dialog = popupLoader.load(pluginDialog, root);
                    dialog.subscriptionId = subscription.id;
                    dialog.open();
                    break;
                }
                default: {
                    var dialog = popupLoader.load(subscriptionDialog, root);
                    dialog.subscriptionId = subscription.id;
                    dialog.sourceType = subscription.sourceType;
                    dialog.open();
                    break;
                }
                }
            }
        }
        
        MenuItem {
            text: qsTr("Unsubscribe")
            onTriggered: database.deleteSubscription(subscriptionModel.data(subscriptionView.currentIndex, "id"))
        }
    }
    
    Connections {
        target: cutenews
        onArticleRequested: showArticle(articleId)
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: subscriptionTypeDialog
        
        ListPickSelector {
            title: qsTr("Subscription type")
            model: SubscriptionSourceTypeModel {}
            textRole: "name"
            onSelected: {
                var sourceType = model.data(currentIndex, "value");
                
                switch (sourceType) {
                case Subscription.Plugin: {
                    var dialog = popupLoader.load(pluginDialog, root);
                    dialog.pluginName = model.data(currentIndex, "name");
                    dialog.open();
                    break;
                }
                default: {
                    var dialog = popupLoader.load(subscriptionDialog, root);
                    dialog.sourceType = sourceType;
                    dialog.open();
                    break;
                }
                }
            }
        }
    }
    
    Component {
        id: subscriptionDialog
        
        SubscriptionDialog {}
    }
    
    Component {
        id: pluginDialog
        
        PluginDialog {}
    }
    
    Component.onCompleted: {
        appWindow.title = "cuteNews";
        subscriptionModel.load();
        subscriptionView.forceActiveFocus();
    }
}
