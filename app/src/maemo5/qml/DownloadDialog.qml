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
    
    property string url
    property alias command: commandField.text
    property alias commandOverrideEnabled: overrideCheckBox.checked
    property alias category: categorySelector.value
    property alias priority: prioritySelector.value
    property alias usePlugin: pluginCheckBox.checked
    
    title: qsTr("Download")
    height: Math.min(360, column.height + platformStyle.paddingMedium)
    
    Flickable {
        id: flickable

        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
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
                wrapMode: Text.WordWrap
                text: qsTr("Custom command (%f for filename)")
            }

            TextField {
                id: commandField

                width: parent.width
            }

            CheckBox {
                id: overrideCheckBox

                width: parent.width
                text: qsTr("Override global custom command")
            }
            
            ListSelectorButton {
                id: categorySelector
                
                width: parent.width
                text: qsTr("Category")
                model: CategoryNameModel {}
                value: settings.defaultCategory
                onSelected: settings.defaultCategory = value
            }
            
            ListSelectorButton {
                id: prioritySelector
                
                width: parent.width
                text: qsTr("Priority")
                model: TransferPriorityModel {}
                value: Transfer.NormalPriority
            }
            
            CheckBox {
                id: pluginCheckBox
                
                width: parent.width
                text: qsTr("Use plugin")
            }
        }
    }
    
    Button {
        id: button
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        style: DialogButtonStyle {}
        text: qsTr("Done")
        enabled: root.url != ""
        onClicked: root.accept()
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        AnchorChanges {
            target: flickable
            anchors.right: parent.right
            anchors.bottom: button.top
        }

        PropertyChanges {
            target: flickable
            anchors.rightMargin: 0
            anchors.bottomMargin: platformStyle.paddingMedium
            clip: true
        }

        PropertyChanges {
            target: button
            width: parent.width
        }

        PropertyChanges {
            target: root
            height: Math.min(680, column.height + button.height + platformStyle.paddingMedium * 2)
        }
    }

    onUrlChanged: {
        var config = plugins.getConfigForEnclosure(url);

        if (config) {
            pluginCheckBox.text = qsTr("Use") + " " + config.displayName + " " + qsTr("plugin");
            pluginCheckBox.visible = true;
            pluginCheckBox.checked = true;
        }
        else {
            pluginCheckBox.visible = false;
        }
    }
    
    onAccepted: transfers.addEnclosureDownload(url, command, commandOverrideEnabled, category, priority, usePlugin)
}
