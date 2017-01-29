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
    
    title: "cuteNews"
    tools: ToolBarLayout {
        BackToolButton {}
                
        MyToolButton {
            id: subscriptionButton

            iconSource: "toolbar-add"
            toolTip: qsTr("New subscription")
            onClicked: popups.open(sourceTypeDialog, root)
        }
        
        MyToolButton {
            id: updateButton

            iconSource: subscriptions.status == Subscriptions.Active ? "toolbar-mediacontrol-stop" : "toolbar-refresh"
            toolTip: subscriptions.status == Subscriptions.Active ? qsTr("Cancel updates") : qsTr("Update all")
            onClicked: subscriptions.status == Subscriptions.Active ? subscriptions.cancel() : subscriptions.updateAll()
        }
        
        MyToolButton {
            iconSource: "toolbar-view-menu"
            toolTip: qsTr("Options")
            onClicked: popups.open(menu, root)
        }
    }
    
    MySearchBox {
        id: searchBox
        
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        onAccepted: {
            subscriptionView.forceActiveFocus();
            appWindow.pageStack.push(Qt.resolvedUrl("ArticlesPage.qml"),
            {title: qsTr("Search") + " '" + searchText + "'"}).search(searchText);
            searchText = "";
        }
    }
    
    MyListView {
        id: subscriptionView
        
        anchors {
            left: parent.left
            right: parent.right
            top: searchBox.bottom
            bottom: parent.bottom
        }
        model: SubscriptionModel {
            id: subscriptionModel
        }
        delegate: SubscriptionDelegate {
            onActivated: appWindow.pageStack.push(Qt.resolvedUrl("ArticlesPage.qml"), {title: title}).load(id)
            onClicked: appWindow.pageStack.push(Qt.resolvedUrl("ArticlesPage.qml"), {title: title}).load(id)
            onPressAndHold: if (index > 1) popups.open(contextMenu, root);
        }
    }
    
    ScrollDecorator {
        flickableItem: subscriptionView
    }
    
    Component {
        id: menu
        
        MyMenu {
            focusItem: subscriptionView
                        
            MenuLayout {
                MenuItem {
                    text: qsTr("Reload")
                    enabled: settings.serverAddress != ""
                    onClicked: {
                        subscriptions.getStatus(Subscriptions.DefaultStatusInterval);
                        serversettings.load();
                        plugins.load();
                        subscriptionModel.load();
                    }
                }

                MenuItem {
                    text: qsTr("Mark all as read")
                    enabled: settings.serverAddress != ""
                    onClicked: database.markAllSubscriptionsRead()
                }
                
                MenuItem {
                    text: qsTr("Import from OPML")
                    enabled: settings.serverAddress != ""
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("ImportPage.qml"))
                }
                
                MenuItem {
                    text: qsTr("Downloads")
                    enabled: settings.serverAddress != ""
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("TransfersPage.qml"))
                }
                
                MenuItem {
                    text: qsTr("Settings")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
                }
            }
        }
    }
    
    Component {
        id: contextMenu
        
        MyContextMenu {
            focusItem: subscriptionView
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Update")
                    onClicked: subscriptions.update(subscriptionModel.data(subscriptionView.currentIndex, "id"))
                }
                
                MenuItem {
                    text: qsTr("Mark as read")
                    onClicked: subscriptionModel.setData(subscriptionView.currentIndex, true, "read")
                }
                
                MenuItem {
                    text: qsTr("Properties")
                    onClicked: {
                        var sourceType = subscriptionModel.data(subscriptionView.currentIndex, "sourceType");
                        var id = subscriptionModel.data(subscriptionView.currentIndex, "id");
                        
                        switch (sourceType) {
                        case Subscription.Url:
                            appWindow.pageStack.push(Qt.resolvedUrl("UrlSubscriptionPage.qml"),
                            {subscriptionId: id});
                            break;
                        case Subscription.LocalFile:
                            appWindow.pageStack.push(Qt.resolvedUrl("LocalFileSubscriptionPage.qml"),
                            {subscriptionId: id});
                            break;
                        case Subscription.Command:
                            appWindow.pageStack.push(Qt.resolvedUrl("CommandSubscriptionPage.qml"),
                            {subscriptionId: id});
                            break;
                        default:
                            appWindow.pageStack.push(Qt.resolvedUrl("PluginSubscriptionPage.qml"),
                            {subscriptionId: id});
                            break;
                        }
                    }
                }
                
                MenuItem {
                    text: qsTr("Unsubscribe")
                    onClicked: popups.open(unsubscribeDialog, root)
                }
            }
        }
    }
    
    Component {
        id: sourceTypeDialog
        
        ValueDialog {
            focusItem: subscriptionView
            titleText: qsTr("Source type")
            highlightSelectedIndex: false
            model: SubscriptionSourceTypeModel {}
            onAccepted: {
                switch (value) {
                case Subscription.Url:
                    appWindow.pageStack.push(Qt.resolvedUrl("UrlSubscriptionPage.qml"));
                    break;
                case Subscription.LocalFile:
                    appWindow.pageStack.push(Qt.resolvedUrl("LocalFileSubscriptionPage.qml"));
                    break;
                case Subscription.Command:
                    appWindow.pageStack.push(Qt.resolvedUrl("CommandSubscriptionPage.qml"));
                    break;
                default:
                    appWindow.pageStack.push(Qt.resolvedUrl("PluginSubscriptionPage.qml"), {pluginId: value});
                    break;
                }
            }
        }
    }
    
    Component {
        id: unsubscribeDialog
        
        MyQueryDialog {
            focusItem: subscriptionView
            titleText: qsTr("Unsubscribe?")
            message: qsTr("Do you want to unsubscribe to") + " '"
            + subscriptionModel.data(subscriptionView.currentIndex, "title") + "'?"
            onAccepted: subscriptionModel.remove(subscriptionView.currentIndex)
        }
    }
    
    Connections {
        target: settings
        onServerAddressChanged: {
            if (address) {
                subscriptionButton.enabled = true;
                updateButton.enabled = true;
                searchBox.enabled = true;
                subscriptions.getStatus(Subscriptions.DefaultStatusInterval);
                serversettings.load();
                plugins.load();
                subscriptionModel.load();
            }
            else {
                subscriptionButton.enabled = false;
                updateButton.enabled = false;
                searchBox.enabled = false;
            }
        }
    }

    Component.onCompleted: {
        if (settings.serverAddress) {
            subscriptionButton.enabled = true;
            updateButton.enabled = true;
            searchBox.enabled = true;
            subscriptions.getStatus(Subscriptions.DefaultStatusInterval);
            serversettings.load();
            plugins.load();
            subscriptionModel.load();
        }
        else {
            subscriptionButton.enabled = false;
            updateButton.enabled = false;
            searchBox.enabled = false;
        }
    }
}
