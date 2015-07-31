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
    
    property int subscriptionId: -1
    
    title: subscriptionId == -1 ? qsTr("New subscription") : qsTr("Subscription properties")
    height: column.height + platformStyle.paddingMedium
  
    Column {
        id: column
    
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            bottom: parent.bottom
        }
        spacing: platformStyle.paddingMedium
        
        Label {
            width: parent.width
            text: qsTr("Source")
        }
        
        Row {
            width: parent.width
            spacing: platformStyle.paddingMedium
            
            TextField {
                id: sourceField
                
                width: parent.width - sourceButton.width - parent.spacing
            }
        
            Button {
                id: sourceButton
                
                text: qsTr("Browse")
                enabled: (sourceTypeSelector) && (sourceTypeSelector.currentIndex > 0)
                onClicked: dialogs.showFileDialog() 
            }
        }
        
        ValueButton {
            id: sourceTypeButton
            
            width: parent.width
            text: qsTr("Source type")
            pickSelector: sourceTypeSelector
        }
        
        CheckBox {
            id: enclosuresCheckBox
            
            width: parent.width
            text: qsTr("Download enclosures automatically")
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
        enabled: sourceField.text != ""
        onClicked: root.accept()
    }
    
    ListPickSelector {
        id: sourceTypeSelector
        
        model: SubscriptionSourceTypeModel {
            id: sourceTypeModel
        }
        textRole: "name"
    }
    
    Subscription {
        id: subscription
        
        onSourceChanged: sourceField.text = source
        onSourceTypeChanged: sourceTypeSelector.currentIndex = sourceTypeModel.match("value", sourceType)
        onDownloadEnclosuresChanged: enclosuresCheckBox.checked = downloadEnclosures
    }
    
    QtObject {
        id: dialogs
        
        property FileDialog fileDialog        
        
        function showFileDialog() {
            if (!fileDialog) {
                fileDialog = fileDialogComponent.createObject(root);
            }
            
            fileDialog.open();
        }
    }
    
    Component {
        id: fileDialogComponent
        
        FileDialog {
            onAccepted: sourceField.text = filePath
        }
    }
    
    onStatusChanged: {
        if (status == DialogStatus.Open) {
            sourceField.forceActiveFocus();
            
            if (subscriptionId > 0) {
                subscription.load(subscriptionId);
            }
        }
    }
    onAccepted: {
        if (subscriptionId > 0) {
            database.updateSubscription(subscriptionId,
            {source: sourceField.text, sourceType: sourceTypeModel.data(sourceTypeSelector.currentIndex, "value"),
             downloadEnclosures: enclosuresCheckBox.checked ? 1 : 0});
        }
        else {
            database.addSubscription([subscriptionId > 0 ? subscriptionId : null, 0, "",
            enclosuresCheckBox.checked ? 1 : 0, "", "", sourceField.text,
            sourceTypeModel.data(sourceTypeSelector.currentIndex, "value"), qsTr("New subscription"), 0, ""]);
        }
        
        sourceField.clear();
        sourceTypeSelector.currentIndex = 0;
        enclosuresCheckBox.checked = false;
    }
    onRejected: {
        sourceField.clear();
        sourceTypeSelector.currentIndex = 0;
        enclosuresCheckBox.checked = false;
    }
}
