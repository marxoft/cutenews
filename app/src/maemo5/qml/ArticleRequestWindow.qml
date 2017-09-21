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

import QtQuick 1.0
import org.hildon.components 1.0
import org.hildon.utils 1.0
import org.hildon.webkit 1.0
import cuteNews 1.0

Window {
    id: root

    property string url
    property QtObject article

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
            action: enclosuresAction
        }
    }
    
    Action {
        id: copyAction
        
        text: qsTr("Copy URL")
        autoRepeat: false
        shortcut: settings.copyShortcut
        onTriggered: clipboard.text = article.resultUrl
    }
    
    Action {
        id: openAction
        
        text: qsTr("Open externally")
        autoRepeat: false
        shortcut: settings.openExternallyShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("OpenDialog.qml"), root, {url: article.resultUrl})
    }
    
    Action {
        id: downloadAction
        
        text: qsTr("Download")
        autoRepeat: false
        shortcut: settings.downloadShortcut
        onTriggered: popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root, {url: article.resultUrl})
    }
    
    Action {
        id: enclosuresAction
        
        text: qsTr("Enclosures")
        autoRepeat: false
        shortcut: settings.showArticleEnclosuresShortcut
        enabled: (article != null) && (article.resultHasEnclosures)
        onTriggered: popupManager.open(enclosuresDialog, root)
    }
       
    Flickable {
        id: flickable
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: view.height
        pressDelay: 100000
        focus: true
        
        WebView {
            id: view
            
            preferredWidth: flickable.width
            contextMenuPolicy: Qt.CustomContextMenu
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
            onCustomContextMenuRequested: {
                var result = hitTestContent(menuX, menuY);
                var link = result.linkUrl.toString();

                if (link) {
                    popupManager.open(urlMenu, root, {url: link});
                }
            }
            onLinkClicked: popupManager.open(Qt.resolvedUrl("OpenDialog.qml"), root, {url: link})
            onStatusChanged: root.showProgressIndicator = (status == WebView.Loading)
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
                text: qsTr("Open article")
                enabled: (menu.url) && (plugins.articleIsSupported(menu.url))
                onTriggered: windowStack.push(Qt.resolvedUrl("ArticleRequestWindow.qml"), {url: menu.url})
            }
            
            MenuItem {
                text: qsTr("Open externally")
                onTriggered: popupManager.open(Qt.resolvedUrl("OpenDialog.qml"), root, {url: menu.url})
            }

            MenuItem {
                text: qsTr("Download")
                onTriggered: popupManager.open(Qt.resolvedUrl("DownloadDialog.qml"), root, {url: menu.url})
            }            
        }
    }
       
    Component {
        id: enclosuresDialog
        
        EnclosuresDialog {
            enclosures: article.resultEnclosures
        }
    }

    Connections {
        target: article
        onFinished: {
            if (article.status == ArticleRequest.Ready) {
                flickable.contentY = 0;
                title = article.resultTitle || qsTr("Article");
                view.html = "<p class='title'>" + title + "</p><div class='separator'></div><p>"
                + qsTr("Author") + ": " + (article.resultAuthor || qsTr("Unknown")) + "</br>"
                + qsTr("Date") + ": " + (article.resultDateString || qsTr("Unknown")) + "</br>"
                + qsTr("Categories") + ": " + (article.resultCategories.length > 0
                ? article.resultCategories.join(", ") : qsTr("None")) + "</p><div class='separator'></div><p>"
                + utils.replaceSrcPaths(article.resultBody, CACHE_AUTHORITY + TEMPORARY_CACHE_PATH) + "</p>";
            }
            else if (article.status == ArticleRequest.Error) {
                informationBox.information(article.errorString);
            }
        }
        onStatusChanged: root.showProgressIndicator = (article.status == ArticleRequest.Active)
    }

    PluginSettings {
        id: pluginSettings
    }

    onUrlChanged: {
        var config = plugins.getConfigForArticle(url);

        if (!config) {
            informationBox.information(qsTr("No plugin found for this article URL"));
            return;
        }

        pluginSettings.pluginId = config.id;

        if (article) {
            article.destroy();
        }

        article = plugins.articleRequest(url, root);
        article.getArticle(url, pluginSettings.values);
    }
}
