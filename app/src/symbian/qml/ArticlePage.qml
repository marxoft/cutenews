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

    property Article article

    signal next
    signal nextUnread
    signal previous

    showProgressIndicator: (article != null) && (article.status == Article.Active)
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "toolbar-previous"
            toolTip: qsTr("Previous article")
            onClicked: root.previous()
        }
        
        MyToolButton {
            iconSource: "toolbar-next"
            toolTip: qsTr("Next article")
            onClicked: root.next()
        }
        
        MyToolButton {
            iconSource: "toolbar-mediacontrol-forward"
            toolTip: qsTr("Next unread article")
            onClicked: root.nextUnread()
        }
        
        MyToolButton {
            iconSource: "toolbar-view-menu"
            toolTip: qsTr("Options")
            enabled: article != null
            onClicked: popups.open(menu, root)
        }
    }
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: column.height + platformStyle.paddingLarge
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: platformStyle.paddingLarge
            }

            Label {
                id: titleLabel

                width: parent.width
                wrapMode: Text.Wrap
                font.pixelSize: platformStyle.fontSizeLarge
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }

            HeaderLabel {
                id: detailsHeader

                width: parent.width
                text: qsTr("Details")
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            Label {
                id: authorLabel
                
                width: parent.width
                wrapMode: Text.Wrap
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            Label {
                id: dateLabel
                
                width: parent.width
                wrapMode: Text.Wrap
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            Label {
                id: categoriesLabel
                
                width: parent.width
                wrapMode: Text.Wrap
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            HeaderLabel {
                id: bodyHeader
                
                width: parent.width
                text: qsTr("Content")
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            Label {
                id: bodyLabel
                
                width: parent.width
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                clip: true
                onLinkActivated: {
                    var m = popups.load(urlMenu, root)
                    m.url = link;
                    m.open();
                }
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            HeaderLabel {
                id: enclosuresHeader
                
                width: parent.width
                text: qsTr("Enclosures")
                visible: enclosuresRepeater.count > 0
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            Repeater {
                id: enclosuresRepeater
                
                property int currentIndex
                
                EnclosureDelegate {
                    x: -platformStyle.paddingLarge
                    width: column.width + platformStyle.paddingLarge * 2
                    flickableMode: true
                    onActivated: appWindow.pageStack.push(Qt.resolvedUrl("DownloadPage.qml"),
                    {url: modelData.url, subscriptionId: article.subscriptionId})
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("DownloadPage.qml"),
                    {url: modelData.url, subscriptionId: article.subscriptionId})
                    onPressAndHold: {
                        enclosuresRepeater.currentIndex = index;
                        popups.open(enclosureMenu, root);
                    }
                }
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    Component {
        id: menu
        
        MyMenu {
            focusItem: flickable
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Copy URL")
                    onClicked: clipboard.text = article.url
                }
                
                MenuItem {
                    text: qsTr("Open externally")
                    onClicked: Qt.openUrlExternally(article.url)
                }
                
                MenuItem {
                    text: qsTr("Download")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("DownloadPage.qml"), {url: article.url})
                }
                
                MenuItem {
                    text: article.read ? qsTr("Mark as unread") : qsTr("Mark as read")
                    onClicked: article.markRead(!article.read)
                }
                
                MenuItem {
                    text: article.favourite? qsTr("Unfavourite") : qsTr("Favourite")
                    onClicked: article.markFavourite(!article.favourite)
                }
                
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: popups.open(deleteDialog, root)
                }
            }
        }
    }

    Component {
        id: urlMenu

        MyContextMenu {
            id: menu

            property string url

            focusItem: flickable

            MenuLayout {
                MenuItem {
                    text: qsTr("Copy URL")
                    onClicked: clipboard.text = menu.url
                }
                
                MenuItem {
                    text: qsTr("Open externally")
                    onClicked: Qt.openUrlExternally(menu.url)
                }
                
                MenuItem {
                    text: qsTr("Download")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("DownloadPage.qml"), {url: menu.url})
                }
            }
        }
    }
    
    Component {
        id: enclosureMenu
        
        MyContextMenu {
            focusItem: flickable
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Copy URL")
                    onClicked: clipboard.text = article.enclosures[enclosuresRepeater.currentIndex].url
                }
                
                MenuItem {
                    text: qsTr("Open externally")
                    onClicked: Qt.openUrlExternally(article.enclosures[enclosuresRepeater.currentIndex].url)
                }

                MenuItem {
                    text: qsTr("Download")
                    onClicked: appWindow.pageStack.push(Qt.resolvedUrl("DownloadPage.qml"),
                        {url: article.enclosures[enclosuresRepeater.currentIndex].url})
                }
            }
        }
    }
    
    Component {
        id: deleteDialog
        
        MyQueryDialog {
            focusItem: flickable
            titleText: qsTr("Delete?")
            message: qsTr("Do you want to delete") + " '" + article.title + "'?"
            onAccepted: article.remove()
        }
    }
    
    onArticleChanged: {
        if (article) {
            flickable.contentY = 0;
            title = article.title ? article.title : qsTr("Article");
            titleLabel.text = title;
            authorLabel.text = qsTr("Author") + ": " + (article.author ? article.author : qsTr("Unknown"));
            dateLabel.text = qsTr("Date") + ": " + (article.dateString ? article.dateString : qsTr("Unknown"));
            categoriesLabel.text = qsTr("Categories") + ": "
            + (article.categories.length > 0 ? article.categories.join(", ") : qsTr("None"));
            bodyLabel.text = article.body;
            enclosuresRepeater.model = article.enclosures;

            if (!article.read) {
                article.markRead(true);
            }
        }
    }
}
