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

ApplicationWindow {
    id: appWindow
        
    visible: true
    title: "cuteNews"
    showProgressIndicator: subscriptions.status == Subscriptions.Active
    menuBar: MenuBar {
        MenuItem {
            action: readAllAction
        }
        
        MenuItem {
            action: importAction
        }
        
        MenuItem {
            text: qsTr("Downloads") + " (" + transfers.count + ")"
            onTriggered: windowStack.push(Qt.resolvedUrl("TransfersWindow.qml"))
        }
        
        MenuItem {
            action: searchAction
        }
        
        MenuItem {
            text: qsTr("Settings")
            onTriggered: popups.open(settingsDialog, appWindow)
        }
        
        MenuItem {
            text: qsTr("About")
            onTriggered: popups.open(aboutDialog, appWindow)
        }
    }
    
    Action {
        id: readAllAction
        
        text: qsTr("Mark all as read")
        autoRepeat: false
        shortcut: qsTr("Ctrl+R")
        onTriggered: database.markAllSubscriptionsRead()
    }
    
    Action {
        id: importAction
        
        text: qsTr("Import from OPML")
        autoRepeat: false
        shortcut: qsTr("Ctrl+O")
        onTriggered: popups.open(importDialog, appWindow)
    }
    
    Action {
        id: searchAction
        
        text: qsTr("Search")
        autoRepeat: false
        shortcut: qsTr("Ctrl+F")
        onTriggered: popups.open(searchDialog, appWindow)
    }
    
    Action {
        id: updateAction
        
        text: qsTr("Update")
        autoRepeat: false
        shortcut: qsTr("u")
        enabled: subscriptionView.currentIndex > 1
        onTriggered: subscriptions.update(subscriptionModel.data(subscriptionView.currentIndex, "id"))
    }
    
    Action {
        id: readAction
        
        text: qsTr("Mark as read")
        autoRepeat: false
        shortcut: qsTr("r")
        enabled: subscriptionView.currentIndex > 1
        onTriggered: subscriptionModel.setData(subscriptionView.currentIndex, true, "read")
    }
    
    Action {
        id: propertiesAction
        
        text: qsTr("Properties")
        autoRepeat: false
        shortcut: qsTr("p")
        enabled: subscriptionView.currentIndex > 1
        onTriggered: {
            var subscription = subscriptionModel.itemData(subscriptionView.currentIndex);
            
            switch (subscription.sourceType) {
                case Subscription.Plugin: {
                    var dialog = popups.load(pluginDialog, appWindow);
                    dialog.subscriptionId = subscription.id;
                    dialog.open();
                    break;
                }
                default: {
                    var dialog = popups.load(subscriptionDialog, appWindow);
                    dialog.subscriptionId = subscription.id;
                    dialog.sourceType = subscription.sourceType;
                    dialog.open();
                    break;
                }
            }
        }
    }
    
    Action {
        id: unsubscribeAction
        
        text: qsTr("Unsubscribe")
        autoRepeat: false
        shortcut: qsTr("d")
        enabled: subscriptionView.currentIndex > 1
        onTriggered: popups.open(unsubscribeDialog, appWindow)
    }
    
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
        activeFocusOnPress: false
        autoRepeat: false
        text: qsTr("New subscription")
        iconName: "general_add"
        shortcut: qsTr("Ctrl+N")
        onClicked: popups.open(subscriptionTypeDialog, appWindow)
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
        activeFocusOnPress: false
        autoRepeat: false
        text: subscriptions.status == Subscriptions.Active ? qsTr("Cancel updates") : qsTr("Update all")
        iconName: subscriptions.status == Subscriptions.Active ? "general_stop" : "general_refresh"
        shortcut: qsTr("Ctrl+U")
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
        }
        delegate: SubscriptionDelegate {
            onClicked: windowStack.push(Qt.resolvedUrl("ArticlesWindow.qml"), {title: title}).load(id)
            onPressAndHold: if (index > 1) popups.open(contextMenu, appWindow);
        }
    }
    
    InformationBox {
        id: informationBox
        
        function information(message) {
            informationLabel.text = message;
            open();
        }
        
        height: informationLabel.height + platformStyle.paddingLarge
        
        Label {
            id: informationLabel
            
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                margins: platformStyle.paddingLarge
            }
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: platformStyle.reversedTextColor
        }
    }
    
    PopupLoader {
        id: popups
    }
    
    Component {
        id: contextMenu
        
        Menu {
            MenuItem {
                action: updateAction
            }
            
            MenuItem {
                action: readAction
            }
            
            MenuItem {
                action: propertiesAction
            }
            
            MenuItem {
                action: unsubscribeAction
            }
        }
    }
    
    Component {
        id: unsubscribeDialog
        
        MessageBox {
            text: qsTr("Unsubscribe from") + " '" + subscriptionModel.data(subscriptionView.currentIndex, "title")
            + "'?"
            onAccepted: subscriptionModel.remove(subscriptionView.currentIndex)
        }
    }
    
    Component {
        id: importDialog
        
        FileDialog {
            nameFilters: ["*.opml"]
            onAccepted: subscriptions.importFromOpml(filePath)
        }
    }
    
    Component {
        id: searchDialog
        
        SearchDialog {}
    }
        
    Component {
        id: settingsDialog
        
        SettingsDialog {}
    }
    
    Component {
        id: aboutDialog
        
        AboutDialog {}
    }
    
    Component {
        id: subscriptionTypeDialog
        
        ListPickSelector {
            title: qsTr("Subscription type")
            model: SubscriptionSourceTypeModel {}
            textRole: "name"
            onSelected: {
                var value = model.data(currentIndex, "value");
                
                switch (value) {
                case Subscription.Url:
                case Subscription.LocalFile:
                case Subscription.Command: {
                    var dialog = popups.load(subscriptionDialog, appWindow);
                    dialog.sourceType = value;
                    dialog.open();
                    break;
                }
                default: {
                    var dialog = popups.load(pluginDialog, appWindow);
                    dialog.pluginId = value;
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
    
    Component {
        id: articleComponent
        
        Article {
            id: article
            
            autoUpdate: true
            onFinished: parent.article = article
        }
    }
    
    Connections {
        target: cutenews
        onArticleRequested: {
            windowStack.clear();
            windowStack.push(Qt.resolvedUrl("ArticleWindow.qml"));
            var article = articleComponent.createObject(windowStack.currentWindow);
            article.load(articleId);
        }
    }
    
    Connections {
        target: notifier
        onError: informationBox.information(errorString)
        onReadArticlesDeleted: informationBox.information(count + " " + qsTr("read articles deleted"));
    }
    
    Connections {
        target: transfers
        onTransferAdded: informationBox.information(qsTr("Enclosure added to downloads"))
    }
    
    Component.onCompleted: subscriptionModel.load()
}
