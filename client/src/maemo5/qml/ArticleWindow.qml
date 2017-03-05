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
import org.hildon.webkit 1.0
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
            action: copyAction
        }
        
        MenuItem {
            action: openAction
        }
        
        MenuItem {
            action: downloadAction
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
        id: copyAction
        
        text: qsTr("Copy URL")
        autoRepeat: false
        shortcut: settings.copyShortcut
        onTriggered: clipboard.text = article.url
    }
    
    Action {
        id: openAction
        
        text: qsTr("Open externally")
        autoRepeat: false
        shortcut: settings.openExternallyShortcut
        onTriggered: urlopener.open(article.url)
    }
    
    Action {
        id: downloadAction
        
        text: qsTr("Download")
        autoRepeat: false
        shortcut: settings.downloadShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root, {url: article.url})
    }
        
    Action {
        id: readAction
        
        autoRepeat: false
        shortcut: settings.toggleArticleReadShortcut
        onTriggered: article.markRead(!article.read)
    }
    
    Action {
        id: favouriteAction
        
        autoRepeat: false
        shortcut: settings.toggleArticleFavouriteShortcut
        onTriggered: article.markFavourite(!article.favourite)
    }
    
    Action {
        id: enclosuresAction
        
        text: qsTr("Enclosures")
        autoRepeat: false
        shortcut: settings.showArticleEnclosuresShortcut
        enabled: (article != null) && (article.hasEnclosures)
        onTriggered: popupManager.open(enclosuresDialog, root)
    }
    
    Action {
        id: deleteAction
        
        text: qsTr("Delete")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        onTriggered: popupManager.open(deleteDialog, root)
    }
    
    Action {
        id: nextAction
        
        text: qsTr("Next article")
        autoRepeat: false
        shortcut: settings.nextArticleShortcut
        onTriggered: root.next()
    }
    
    Action {
        id: nextUnreadAction
        
        text: qsTr("Next unread article")
        autoRepeat: false
        shortcut: settings.nextUnreadArticleShortcut
        onTriggered: root.nextUnread()
    }
    
    Action {
        id: previousAction
        
        text: qsTr("Previous article")
        autoRepeat: false
        shortcut: settings.previousArticleShortcut
        onTriggered: root.previous()
    }
    
    Flickable {
        id: flickable
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: view.height
        pressDelay: 100000
        
        WebView {
            id: view
            
            preferredWidth: flickable.width
            contextMenuPolicy: Qt.NoContextMenu
            linkDelegationPolicy: WebPage.DelegateAllLinks
            settings.userStyleSheetUrl: {
                return "data:text/css;charset=utf-8;base64,"
                + Qt.btoa("html { font-family: " + platformStyle.fontFamily + "; font-size: "
                + platformStyle.fontSizeMedium + "pt; background-color: " + platformStyle.defaultBackgroundColor
                + "; color: " + platformStyle.defaultTextColor
                + "; } img { max-width: 100%; } iframe { max-width: 100%; } .title { font-size: "
                + platformStyle.fontSizeLarge + "pt; } .separator { height: 1px; background-color: "
                + platformStyle.secondaryTextColor + "; }");
            }
            onLinkClicked: popupManager.open(urlMenu, root, {url: link})
        }
    }
    
    Component {
        id: urlMenu
        
        Menu {
            id: menu
            
            property string url
            
            MenuItem {
                text: qsTr("Copy URL")
                onTriggered: clipboard.text = menu.url
            }
            
            MenuItem {
                text: qsTr("Open externally")
                onTriggered: urlopener.open(menu.url)
            }

            MenuItem {
                text: qsTr("Download")
                onTriggered: popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root, {url: menu.url})
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
            view.html = "<p class='title'>" + title + "</p><div class='separator'></div><p>"
            + qsTr("Author") + ": " + (article.author ? article.author : qsTr("Unknown")) + "</br>"
            + qsTr("Date") + ": " + (article.dateString ? article.dateString : qsTr("Unknown")) + "</br>"
            + qsTr("Categories") + ": " + (article.categories.length > 0 ? article.categories.join(", ") : qsTr("None"))
            + "</br><div class='separator'></div><p>" + article.body + "</p>";
            
            if (!article.read) {
                article.markRead(true);
            }
        }
    }
}
