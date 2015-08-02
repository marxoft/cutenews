/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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
    
    anchors.fill: parent
    
    ListView {
        id: subscriptionView
        
        anchors {
            left: parent.left
            top: parent.top
            bottom: toolBar.top
        }
        width: 200
        visible: viewButton.checked
        model: SubscriptionModel {
            id: subscriptionModel
            
            onStatusChanged: if (status == SubscriptionModel.Error) informationBox.information(errorString);
        }
        delegate: ListItem {
            height: 32
            style: ListItemStyle {
                background: ""
                backgroundPressed: ""
                backgroundSelected: ""
            }
            
            Rectangle {
                anchors.fill: parent
                color: parent.ListView.isCurrentItem ? platformStyle.selectionColor : "transparent"
            }
            
            Image {
                id: icon
                
                anchors {
                    left: parent.left
                    leftMargin: platformStyle.paddingSmall
                    verticalCenter: parent.verticalCenter
                }
                width: 24
                height: 24
                smooth: true
                source: iconPath ? iconPath : "image://icon/general_rss"
            }
            
            Label {
                anchors {
                    left: icon.right
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    margins: platformStyle.paddingSmall
                }
                elide: Text.ElideRight
                color: (subscriptions.activeSubscription == id)
                       && (!parent.ListView.isCurrentItem) ? platformStyle.activeTextColor
                                                           : platformStyle.defaultTextColor
                text: subscriptions.activeSubscription == id ? qsTr("Updating...") : title
            }
            
            onClicked: {
                appWindow.windowTitle = "cuteNews - " + title;
                searchField.clear();
                subscriptionView.forceActiveFocus();
                articleModel.load(id);
            }
            onPressAndHold: if (index > 1) subscriptionMenu.popup();
        }
    }
    
    Menu {
        id: subscriptionMenu
        
        MenuItem {
            text: qsTr("Update")
            onTriggered: subscriptions.update(subscriptionModel.data(subscriptionView.currentIndex, "id"))
        }
    }

    ListView {
        id: articleView
        
        anchors {
            left: subscriptionView.visible ? subscriptionView.right : parent.left
            leftMargin: subscriptionView.visible ? platformStyle.paddingMedium : 0
            right: parent.right
            top: parent.top
            bottom: toolBar.top
        }
        cacheBuffer: height * 2
        model: ArticleModel {
            id: articleModel
            
            limit: 5
            onStatusChanged: if (status == ArticleModel.Error) informationBox.information(errorString);
        }
        delegate: Item {
            width: parent.width
            height: column.height + platformStyle.paddingMedium * 2
        
            Column {
                id: column
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: platformStyle.paddingMedium
                }
                spacing: platformStyle.paddingMedium
                
                Label {            
                    width: parent.width
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                    color: settings.viewMode == "dark" ? platformStyle.defaultTextColor : platformStyle.reversedTextColor
                    text: "<a href='" + url + "'>" + title + "</a><br><br>"
                          + qsTr("Author") + ": " + (author ? author : qsTr("Unknown")) + "<br>" + qsTr("Date") + ": "
                          + Qt.formatDateTime(date, "dd/MM/yyyy HH:mm") + "<br>" + qsTr("Categories") + ": "
                          + categories.join(", ") + "<br><br>" + body
        
                    onLinkActivated: if (!urlopener.open(link)) Qt.openUrlExternally(link);
                }
                            
                CheckBox {                
                    width: parent.width
                    height: 26
                    style: CheckBoxStyle {
                        background: ""
                        backgroundChecked: ""
                        backgroundDisabled: ""
                        backgroundPressed: ""
                        iconWidth: 26
                        iconHeight: 26
                        paddingLeft: 0
                        textColor: platformStyle.activeTextColor
                        pressedTextColor: platformStyle.activeTextColor
                        disabledTextColor: platformStyle.buttonTextDisabledColor
                        checkedTextColor: platformStyle.activeTextColor
                    }
                    text: " " + qsTr("Favourite")
                    iconSource: "image://theme/qgn_plat_check_box" + (checked ? "_selected" : "_empty")
                                + (enabled ? "_normal" : "_disabled")
                    checked: favourite
                    onClicked: database.markArticleFavourite(id, checked)
                }
                
                Repeater {
                    model: enclosures
                
                    Row {
                        width: column.width
                        spacing: platformStyle.paddingMedium
                        
                        Label {
                            id: enclosureUrlLabel
                            
                            width: parent.width - enclosureInfoLabel.width - parent.spacing
                            elide: Text.ElideRight
                            color: platformStyle.activeTextColor
                            text: modelData.url
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: if (!urlopener.open(modelData.url)) Qt.openUrlExternally(modelData.url);
                                onPressAndHold: {
                                    enclosureMenu.enclosureUrl = modelData.url;
                                    enclosureMenu.popup();
                                }
                            }
                        }
                        
                        Label {
                            id: enclosureInfoLabel
                            
                            color: settings.viewMode == "dark" ? platformStyle.defaultTextColor
                                                               : platformStyle.reversedTextColor
                            text: utils.formatBytes(modelData.length) + " (" + modelData.type + ")"
                        }
                    }
                }
                
                Rectangle {
                    width: parent.width
                    height: 1
                    color: platformStyle.secondaryTextColor
                }
            }
        }
        
        Rectangle {
            anchors {
                fill: parent
                rightMargin: platformStyle.paddingMedium
            }
            z: -1
            color: settings.viewMode == "dark" ? "transparent" : "#fff"
        }        
    }
    
    Menu {
        id: enclosureMenu
        
        property url enclosureUrl
        
        MenuItem {
            text: qsTr("Open externally")
            onTriggered: if (!urlopener.open(enclosureMenu.enclosureUrl))
                            Qt.openUrlExternally(enclosureMenu.enclosureUrl);
        }
        
        MenuItem {
            text: qsTr("Download")
            onTriggered: {
                downloads.addDownloadTransfer(enclosureMenu.enclosureUrl);
                informationBox.information(qsTr("Download added"));
            }
        }
    }
    
    ToolBar {
        id: toolBar
        
        anchors.bottom: parent.bottom
        
        ToolButton {
            id: updateButton
            
            iconName: "general_refresh"
            enabled: (subscriptionModel.count > 0) && (subscriptions.status != Subscriptions.Active)
            onClicked: {
                for (var i = 2; i < subscriptionModel.count; i++) {
                    subscriptions.update(subscriptionModel.data(i, "id"));
                }
            }
        }
        
        ToolButton {
            id: subscriptionButton
            
            iconName: "general_add"
            onClicked: dialogs.showSubscriptionDialog(-1)
        }
        
        ToolButton {
            id: propertiesButton
            
            iconName: "general_information"
            enabled: subscriptionView.currentIndex > 1
            onClicked: dialogs.showSubscriptionDialog(subscriptionModel.data(subscriptionView.currentIndex, "id"))
        }
        
        ToolButton {
            id: deleteButton
            
            iconName: "general_delete"
            enabled: subscriptionView.currentIndex > 1
            onClicked: {
                database.deleteSubscription(subscriptionModel.data(subscriptionView.currentIndex, "id"));
                appWindow.title = "cuteNews";
            }
        }
        
        ToolButton {
            id: viewButton
            
            iconName: "general_foldertree"
            checkable: true
            checked: true
        }
        
        TextField {
            id: searchField
            
            width: parent.width - 420
            onAccepted: {
                if (text) {
                    subscriptionView.currentIndex = 0;
                    appWindow.windowTitle = "cuteNews - " + qsTr("Search");
                    subscriptionView.forceActiveFocus();
                    articleModel.search(text.substring(0, 6) == "WHERE " ? text
                                        : "WHERE title LIKE '%" + text + "%'");
                }
                else {
                    informationBox.information(qsTr("Enter keywords"));
                }
            }
        }
        
        ToolButton {
            id: searchButton
            
            iconName: "rss_reader_goto"
            onClicked: searchField.accepted()
        }
    }
    
    Component.onCompleted: {
        database.init();
        subscriptionModel.load();
        subscriptionView.forceActiveFocus();
    }
}
