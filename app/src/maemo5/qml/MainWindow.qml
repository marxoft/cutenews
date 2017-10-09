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
            action: transfersAction
        }
        
        MenuItem {
            action: searchAction
        }

        MenuItem {
            action: articleAction
        }

        MenuItem {
            action: pluginsAction
        }
        
        MenuItem {
            action: settingsAction
        }
        
        MenuItem {
            text: qsTr("About")
            onTriggered: popupManager.open(Qt.resolvedUrl("AboutDialog.qml"), appWindow)
        }
    }
    
    Action {
        id: readAllAction
        
        text: qsTr("Mark all as read")
        autoRepeat: false
        shortcut: settings.markAllSubscriptionsReadShortcut
        onTriggered: database.markAllSubscriptionsRead()
    }
    
    Action {
        id: importAction
        
        text: qsTr("Import from OPML")
        autoRepeat: false
        shortcut: settings.importSubscriptionsShortcut
        onTriggered: popupManager.open(importDialog, appWindow)
    }
    
    Action {
        id: transfersAction
        
        text: qsTr("Downloads") + " (" + transfers.count + ")"
        autoRepeat: false
        shortcut: settings.transfersShortcut
        shortcutContext: Qt.ApplicationShortcut
        onTriggered: if (windowStack.currentWindow.objectName != "TransfersWindow") 
        windowStack.push(Qt.resolvedUrl("TransfersWindow.qml"));
    }
    
    Action {
        id: searchAction
        
        text: qsTr("Search articles")
        autoRepeat: false
        shortcut: settings.searchShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("SearchDialog.qml"), appWindow)
    }

    Action {
        id: articleAction
        
        text: qsTr("Fetch article")
        autoRepeat: false
        shortcut: settings.fetchArticleShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("ArticleDialog.qml"), appWindow)
    }
    
    Action {
        id: settingsAction
        
        text: qsTr("Settings")
        autoRepeat: false
        shortcut: settings.settingsShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("SettingsDialog.qml"), appWindow)
    }
    
    Action {
        id: pluginsAction
        
        text: qsTr("Load plugins")
        autoRepeat: false
        shortcut: settings.reloadShortcut
        onTriggered: {
            var count = plugins.load();
            
            if (count) {
                informationBox.information(count + " " + qsTr("new plugins loaded"));
            }
            else {
                informationBox.information(qsTr("No new plugins loaded"));
            }
        }
    }
    
    Action {
        id: updateAction
        
        text: qsTr("Update")
        autoRepeat: false
        shortcut: settings.updateSubscriptionShortcut
        enabled: subscriptionView.currentIndex > 1
        onTriggered: subscriptions.update(subscriptionModel.data(subscriptionView.currentIndex, "id"))
    }
    
    Action {
        id: readAction
        
        text: qsTr("Mark as read")
        autoRepeat: false
        shortcut: settings.markSubscriptionReadShortcut
        enabled: subscriptionView.currentIndex > 1
        onTriggered: subscriptionModel.setData(subscriptionView.currentIndex, true, "read")
    }
    
    Action {
        id: propertiesAction
        
        text: qsTr("Properties")
        autoRepeat: false
        shortcut: settings.editShortcut
        enabled: subscriptionView.currentIndex > 1
        onTriggered: {
            var subscription = subscriptionModel.itemData(subscriptionView.currentIndex);
            
            switch (subscription.sourceType) {
            case Subscription.Plugin:
                popupManager.open(Qt.resolvedUrl("PluginDialog.qml"), appWindow, {subscriptionId: subscription.id});
                break;
            default:
                popupManager.open(Qt.resolvedUrl("SubscriptionDialog.qml"), appWindow,
                {subscriptionId: subscription.id, sourceType: subscription.sourceType});
                break;
            }
        }
    }
    
    Action {
        id: unsubscribeAction
        
        text: qsTr("Unsubscribe")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        enabled: subscriptionView.currentIndex > 1
        onTriggered: popupManager.open(unsubscribeDialog, appWindow)
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
        shortcut: settings.newContentShortcut
        onClicked: popupManager.open(subscriptionTypeDialog, appWindow)
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
        shortcut: settings.updateAllSubscriptionsShortcut
        onClicked: subscriptions.status == Subscriptions.Active ? subscriptions.cancel() : subscriptions.updateAll()
    }
    
    ListView {
        id: subscriptionView
        
        anchors {
            left: parent.left
            right: parent.right
            top: updateButton.bottom
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
            onPressAndHold: if (index > 1) popupManager.open(contextMenu, appWindow);
        }
    }
    
    InformationBox {
        id: informationBox
        
        function information(message) {
            informationLabel.text = message;
            show();
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
            text: qsTr("Do you want to unsubscribe from") + " '"
            + subscriptionModel.data(subscriptionView.currentIndex, "title") + "'?"
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
                case Subscription.Command:
                    popupManager.open(Qt.resolvedUrl("SubscriptionDialog.qml"), appWindow, {sourceType: value});
                    break;
                default:
                    popupManager.open(Qt.resolvedUrl("PluginDialog.qml"), appWindow, {pluginId: value});
                    break;
                }
            }
        }
    }
    
    Component {
        id: articleComponent
        
        Article {
            id: article
            
            autoUpdate: true
            onFinished: windowStack.currentWindow.article = article
        }
    }
    
    Connections {
        target: cutenews
        onArticleRequested: {
            windowStack.clear();

            if (plugins.articleIsSupported(articleId)) {
                windowStack.push(Qt.resolvedUrl("ArticleRequestWindow.qml"), {url: articleId});
            }
            else {
                windowStack.push(Qt.resolvedUrl("ArticleWindow.qml"));
                var article = articleComponent.createObject(windowStack.currentWindow);
                article.load(articleId);
            }
        }
    }
    
    Connections {
        target: notifier
        onError: informationBox.information(errorString)
        onReadArticlesDeleted: informationBox.information(count + " " + qsTr("read articles deleted"));
    }
    
    Connections {
        target: transfers
        onTransferAdded: informationBox.information(qsTr("File added to downloads"))
    }

    Binding {
        target: screen
        property: "orientationLock"
        value: settings.screenOrientation
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: subscriptionButton
            anchors.right: parent.right
        }

        PropertyChanges {
            target: subscriptionButton
            anchors.rightMargin: platformStyle.paddingMedium
        }

        AnchorChanges {
            target: updateButton
            anchors.left: parent.left
            anchors.top: subscriptionButton.bottom
        }

        PropertyChanges {
            target: updateButton
            anchors.leftMargin: platformStyle.paddingMedium
        }
    }
    
    Component.onCompleted: subscriptionModel.load()
}
