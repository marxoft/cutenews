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

Dialog {
    id: root
    
    property Transfer download
    
    title: qsTr("Download properties")
    height: column.height + platformStyle.paddingMedium
    
    
    Flickable {
        id: flickable
        
        anchors {
            left: parent.left
            right: buttonColumn.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: column.height
        
        Column {
            id: column
    
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: platformStyle.paddingMedium
        
            Label {
                width: parent.width
                color: platformStyle.secondaryTextColor
                text: qsTr("Filename")
            }
        
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: download ? download.fileName : qsTr("Unknown")
            }
        
            Label {
                width: parent.width
                color: platformStyle.secondaryTextColor
                text: qsTr("Status")
            }
        
            Label {
                width: parent.width
                wrapMode: Text.Wrap
                color: (download) && (download.status == Transfer.Failed) ? platformStyle.attentionColor
                                                                          : platformStyle.defaultTextColor
                text: download == null ? "" : download.statusString
                      + (download.status == Transfer.Failed ? ": " + download.errorString : "")
            }
        
            Label {
                width: parent.width
                color: platformStyle.secondaryTextColor
                text: qsTr("Progress")
            }
        
            ProgressBar {
                width: parent.width
                value: download ? download.progress : 0
                textVisible: true
                text: (download ? download.progress : 0) + "%"
            }
        
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: download ? (utils.formatBytes(download.bytesTransferred) + " / "
                               + (download.size > 0 ? utils.formatBytes(download.size) : qsTr("Unknown")))
                               : ""
            }        
        }
    }
    
    Column {
        id: buttonColumn
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        
        Button {            
            style: DialogButtonStyle {}
            text: (download) && (download.status == Transfer.Downloading) ? qsTr("Pause") : qsTr("Start")
            onClicked: {
                if (download) {
                    if (download.status == Transfer.Downloading) {
                        download.pause();
                    }
                    else {
                        download.start();
                    }
                }
            }
        }
        
        Button {            
            style: DialogButtonStyle {}
            text: qsTr("Cancel")
            onClicked: {
                if (download) {
                    download.cancel();
                    root.close();
                }
            }
        }
    }
    
    onStatusChanged: if (status == DialogStatus.Open) flickable.contentY = 0;
}
