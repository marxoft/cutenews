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
    property alias category: categorySelector.value
    property alias priority: prioritySelector.value
    
    title: qsTr("Download")
    height: column.height + platformStyle.paddingMedium
    
    Column {
        id: column
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
        }
        spacing: platformStyle.paddingMedium
        
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
    }
    
    Button {
        id: button
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        style: DialogButtonStyle {}
        text: qsTr("Done")
        enabled: (root.url) && (root.subscriptionId)
        onClicked: root.accept()
    }
    
    onAccepted: transfers.addEnclosureDownload(url, category, priority)
}
