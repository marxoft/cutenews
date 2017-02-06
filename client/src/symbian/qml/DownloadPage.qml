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

EditPage {
    id: root
    
    property string url
    property alias category: categorySelector.value
    property alias priority: prioritySelector.value
    property alias usePlugin: pluginSwitch.checked
    
    title: qsTr("Download")
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: column.height
        
        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }

            ValueSelector {
                id: categorySelector

                focusItem: flickable
                width: parent.width
                title: qsTr("Category")
                model: CategoryNameModel {
                    id: categoryModel
                }
                value: settings.defaultCategory
                onAccepted: settings.defaultCategory = value
            }

            ValueSelector {
                id: prioritySelector

                focusItem: flickable
                width: parent.width
                title: qsTr("Priority")
                model: TransferPriorityModel {
                    id: priorityModel
                }
                value: Transfer.NormalPriority
            }
            
            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }

            MySwitch {
                id: pluginSwitch

                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                text: qsTr("Use plugin")
                enabled: plugins.enclosureIsSupported(root.url)
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    onAccepted: {
        transfers.addEnclosureDownload(url, category, priority, usePlugin);
        appWindow.pageStack.pop();
    }
}
