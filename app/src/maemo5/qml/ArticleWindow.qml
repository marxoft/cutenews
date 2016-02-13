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

Window {
    id: root
    
    property Article article
    
    showProgressIndicator: subscriptions.status == Subscriptions.Active
    title: article ? article.title : qsTr("Article")
    menuBar: MenuBar {
        MenuItem {
            text: qsTr("Open externally")
            onTriggered: if (!urlopener.open(article.url)) Qt.openUrlExternally(article.url);
        }
        
        MenuItem {
            text: (article) && (article.read) ? qsTr("Mark as unread") : qsTr("Mark as read")
            onTriggered: database.markArticleRead(article.id, !article.read)
        }
        
        MenuItem {
            text: (article) && (article.favourite) ? qsTr("Unfavourite") : qsTr("Favourite")
            onTriggered: database.markArticleFavourite(article.id, !article.favourite)
        }
        
        MenuItem {
            text: qsTr("Enclosures") + (article ? " (" + article.enclosures.length + ")" : "")
            enabled: (article != null) && (article.enclosures.length > 0)
            onTriggered: popupLoader.open(enclosuresDialog, root)
        }
    }
    
    TextAreaStyle {
        id: textAreaStyle
    }
    
    Flickable {
        id: flickable
        
        anchors.fill: parent
        focus: true
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: background.height
        
        BorderImage {
            id: background
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: Math.max(flickable.height, label.height + textAreaStyle.paddingTop + textAreaStyle.paddingBottom)
            border {
                left: textAreaStyle.backgroundCornerMargin
                right: textAreaStyle.backgroundCornerMargin
                top: textAreaStyle.backgroundCornerMargin
                bottom: textAreaStyle.backgroundCornerMargin
            }
            source: flickable.activeFocus ? textAreaStyle.backgroundSelected : textAreaStyle.background
            smooth: true

            Label {
                id: label
                
                anchors {
                    left: parent.left
                    leftMargin: textAreaStyle.paddingLeft
                    right: parent.right
                    rightMargin: textAreaStyle.paddingRight
                    top: parent.top
                    topMargin: textAreaStyle.paddingTop
                }
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                color: textAreaStyle.textColor
                text: article == null ? "" : qsTr("Author") + ": " + (article.author ? article.author : qsTr("Unknown"))
                + "<br>" + qsTr("Date") + ": " + Qt.formatDateTime(article.date, "dd/MM/yyyy HH:mm") + "<br>"
                + qsTr("Categories") + ": " + article.categories.join(", ") + "<br><br>" + article.body
                
                onLinkActivated: if (!urlopener.open(link)) Qt.openUrlExternally(link);
            }
        }
        
        Keys.onLeftPressed: {
            if (event.modifiers == Qt.NoModifier) {
                articleView.decrementCurrentIndex();
                article = articleModel.get(articleView.currentIndex);
            }
        }
        
        Keys.onRightPressed: {
            if (event.modifiers == Qt.NoModifier) {
                articleView.incrementCurrentIndex();
                article = articleModel.get(articleView.currentIndex);
            }
        }
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: enclosuresDialog
        
        EnclosuresDialog {
            article: root.article
        }
    }
    
    StateGroup {
        states: State {
            name: "LightOnDark"
            when: settings.viewMode == "dark"
        
            PropertyChanges {
                target: textAreaStyle
                background: ""
                backgroundSelected: ""
                backgroundDisabled: ""
                textColor: platformStyle.defaultTextColor
            }
        }
    }
    
    onArticleChanged: {
        flickable.contentY = 0;
        
        if ((article) && (!article.read)) {
            database.markArticleRead(article.id, true);
        }
    }
}
    
