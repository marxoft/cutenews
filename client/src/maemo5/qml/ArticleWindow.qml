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
    
    property Article article
    
    signal next
    signal nextUnread
    signal previous
    
    showProgressIndicator: (article != null) && (article.status == Article.Active)
    title: qsTr("Article")
    menuBar: MenuBar {
        MenuItem {
            action: openAction
        }
        
        MenuItem {
            action: copyAction
        }
        
        MenuItem {
            text: (article != null) && (article.read) ? qsTr("Mark as unread") : qsTr("Mark as read")
            action: readAction
        }
        
        MenuItem {
            text: (article != null) && (article.favourite) ? qsTr("Unfavourite") : qsTr("Favourite")
            action: favouriteAction
        }
        
        MenuItem {
            action: enclosuresAction
        }
        
        MenuItem {
            action: deleteAction
        }
    }
    
    Action {
        id: openAction
        
        text: qsTr("Open externally")
        autoRepeat: false
        shortcut: qsTr("o")
        onTriggered: if (!urlopener.open(article.url)) Qt.openUrlExternally(article.url);
    }
    
    Action {
        id: copyAction
        
        text: qsTr("Copy URL")
        autoRepeat: false
        shortcut: qsTr("c")
        onTriggered: clipboard.text = article.url
    }
    
    Action {
        id: readAction
        
        autoRepeat: false
        shortcut: qsTr("r")
        onTriggered: article.markRead(!article.read)
    }
    
    Action {
        id: favouriteAction
        
        autoRepeat: false
        shortcut: qsTr("f")
        onTriggered: article.markFavourite(!article.favourite)
    }
    
    Action {
        id: enclosuresAction
        
        text: qsTr("Enclosures")
        autoRepeat: false
        shortcut: qsTr("e")
        enabled: (article != null) && (article.hasEnclosures)
        onTriggered: popups.open(enclosuresDialog, root)
    }
    
    Action {
        id: deleteAction
        
        text: qsTr("Delete")
        autoRepeat: false
        shortcut: qsTr("d")
        onTriggered: popups.open(deleteDialog, root)
    }
    
    Action {
        id: nextAction
        
        text: qsTr("Next article")
        autoRepeat: false
        shortcut: qsTr("Right")
        onTriggered: root.next()
    }
    
    Action {
        id: nextUnreadAction
        
        text: qsTr("Next unread article")
        autoRepeat: false
        shortcut: qsTr("Shift+Right")
        onTriggered: root.nextUnread()
    }
    
    Action {
        id: previousAction
        
        text: qsTr("Previous article")
        autoRepeat: false
        shortcut: qsTr("Left")
        onTriggered: root.previous()
    }
    
    Flickable {
        id: flickable
        
        anchors.fill: parent
        focus: true
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: column.height + platformStyle.paddingMedium * 2
        
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
                id: authorLabel
                
                width: parent.width
                elide: Text.ElideRight
            }
            
            Label {
                id: dateLabel
                
                width: parent.width
                elide: Text.ElideRight
            }
            
            Label {
                id: categoriesLabel
                
                width: parent.width
                wrapMode: Text.Wrap
            }
            
            Rectangle {
                width: parent.width
                height: 1
                color: platformStyle.secondaryTextColor
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
                    m.popup();
                }
            }
        }
    }
    
    Component {
        id: urlMenu
        
        Menu {
            id: menu
            
            property string url
            
            MenuItem {
                text: qsTr("Open externally")
                onTriggered: if (!urlopener.open(menu.url)) Qt.openUrlExternally(menu.url);
            }
            
            MenuItem {
                text: qsTr("Copy URL")
                onTriggered: clipboard.text = menu.url
            }
        }
    }
    
    Component {
        id: deleteDialog
        
        MessageBox {
            text: qsTr("Do you want to delete") + " '" + article.title + "'?"
            onAccepted: article.remove()
        }
    }
    
    Component {
        id: enclosuresDialog
        
        EnclosuresDialog {
            enclosures: article.enclosures
        }
    }
    
    onArticleChanged: {
        if (article) {
            flickable.contentY = 0;
            title = article.title ? article.title : qsTr("Article");
            authorLabel.text = qsTr("Author") + ": " + (article.author ? article.author : qsTr("Unknown"));
            dateLabel.text = qsTr("Date") + ": " + (article.dateString ? article.dateString : qsTr("Unknown"));
            categoriesLabel.text = qsTr("Categories") + ": "
            + (article.categories.length > 0 ? article.categories.join(", ") : qsTr("None"));
            bodyLabel.text = article.body.replace(/ src="/g, " src=\"" + settings.serverAddress);
            
            if (!article.read) {
                article.markRead(true);
            }
        }
    }
}
