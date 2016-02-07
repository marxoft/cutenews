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

Dialog {
    id: root
    
    property Article article
    
    title: qsTr("Enclosures")
    height: Math.min(350, view.count * 70 + platformStyle.paddingMedium)
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: article ? article.enclosures : null
        delegate: EnclosureDelegate {
            onClicked: {
                if (!urlopener.open(modelData.url)) {
                    Qt.openUrlExternally(modelData.url);
                }
                
                root.accept();
            }
            onPressAndHold: contextMenu.popup()
        }
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: qsTr("Open externally")
            onTriggered: {
                var url = article.enclosures[view.currentIndex].url;
                
                if (!urlopener.open(url)) {
                    Qt.openUrlExternally(url);
                }
                
                root.accept();
            }
        }
        
        MenuItem {
            text: qsTr("Download")
            onTriggered: {
                downloads.addDownloadTransfer(article.enclosures[view.currentIndex].url, article.subscriptionId);
                informationBox.information(qsTr("Download added"));
                root.accept();
            }
        }
    }
    
    onStatusChanged: if (status == DialogStatus.Open) view.positionViewAtIndex(0, ListView.Beginning);
}
