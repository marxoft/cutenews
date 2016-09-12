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
    
    property QtObject transfer
    
    title: qsTr("Download properties")
    height: 360
    
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
                elide: Text.ElideRight
                text: transfer ? transfer.name : ""
            }
            
            Label {
                width: parent.width
                text: qsTr("Custom command (%f for filename)")
            }
            
            TextField {
                width: parent.width
                text: transfer ? transfer.customCommand : ""
                onTextChanged: transfer.customCommand = text
            }
            
            CheckBox {
                width: parent.width
                text: qsTr("Override global custom command")
                checked: (transfer) && (transfer.customCommandOverrideEnabled)
                onCheckedChanged: transfer.customCommandOverrideEnabled = checked
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Category")
                model: CategoryNameModel {}
                value: transfer ? transfer.category : ""
                onSelected: transfer.category = value
            }
            
            ListSelectorButton {
                width: parent.width
                text: qsTr("Priority")
                model: TransferPriorityModel {}
                value: transfer ? transfer.priority : ""
                onSelected: transfer.priority = value
            }

            ProgressBar {
                width: parent.width
                value: transfer ? transfer.progress : 0
                textVisible: true
                text: (transfer ? transfer.progress : 0) + "%"
            }
        
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: transfer ? transfer.progressString : ""
            }
            
            Label {
                width: parent.width
                wrapMode: Text.Wrap
                color: (transfer) && (transfer.status == Transfer.Failed) ? platformStyle.attentionColor
                                                                          : platformStyle.defaultTextColor
                text: transfer ? transfer.statusString : ""
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
            text: (transfer) && (transfer.status == Transfer.Downloading) ? qsTr("Pause") : qsTr("Start")
            onClicked: {
                if (transfer) {
                    if (transfer.status == Transfer.Downloading) {
                        transfer.pause();
                    }
                    else {
                        transfer.start();
                    }
                }
            }
        }
        
        Button {            
            style: DialogButtonStyle {}
            text: qsTr("Remove")
            onClicked: {
                if (transfer) {
                    transfer.cancel();
                    root.close();
                }
            }
        }
    }    
}
