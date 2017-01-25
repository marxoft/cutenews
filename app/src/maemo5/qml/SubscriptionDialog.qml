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
    
    property string subscriptionId
    property int sourceType: Subscription.Url
    
    title: qsTr("Subscription properties")
    height: flow.height + platformStyle.paddingMedium
    
    Flow {
        id: flow
        
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
        
        TextField {
            id: sourceField
            
            width: parent.width - sourceButton.width - parent.spacing
            onAccepted: if (text) root.accept();
        }
        
        Button {
            id: sourceButton
            
            text: qsTr("Browse")
            enabled: sourceType != Subscription.Url
            onClicked: popupLoader.open(fileDialog, root);
        }
        
        Label {
            width: parent.width
            text: qsTr("Update interval (0 to disable)")
        }
        
        SpinBox {
            id: updateIntervalSpinBox
            
            width: parent.width - updateIntervalSelector.width - parent.spacing
        }
        
        ComboBox {
            id: updateIntervalSelector
            
            model: UpdateIntervalTypeModel {
                id: updateIntervalModel
            }
            textRole: "name"
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

    Subscription {
        id: subscription
        
        onStatusChanged: {
            if (status == Subscription.Ready) {
                sourceField.text = source;
                root.sourceType = sourceType;
                enclosuresCheckBox.checked = downloadEnclosures;
                
                if (updateInterval > 0) {
                    for (var i = updateIntervalModel.count - 1; i >= 0; i--) {
                        var value = updateIntervalModel.data(i, "value");
                        
                        if ((value > 0) && (updateInterval % value == 0)) {
                            updateIntervalSpinBox.value = updateInterval / value;
                            updateIntervalSelector.currentIndex = i;
                            break;
                        }
                    }
                }                
            }
        }
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: fileDialog
        
        FileDialog {
            onAccepted: sourceField.text = filePath
        }
    }
    
    onStatusChanged: {
        if (status == DialogStatus.Open) {
            sourceField.forceActiveFocus();
            
            if (subscriptionId) {
                subscription.load(subscriptionId);
            }
        }
    }
    onAccepted: {
        var interval = updateIntervalSpinBox.value;
        
        if (interval > 0) {
            interval *= updateIntervalModel.data(updateIntervalSelector.currentIndex, "value");
        }
        
        if (subscriptionId) {
            subscription.update(subscriptionId, {source: sourceField.text,
                downloadEnclosures: enclosuresCheckBox.checked ? 1 : 0, updateInterval: interval});
        }
        else {
            subscriptions.create(sourceField.text, sourceType, enclosuresCheckBox.checked, interval);
        }
    }
}
