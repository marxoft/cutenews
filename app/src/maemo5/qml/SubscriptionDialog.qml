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
    
    property int subscriptionId: -1
    property int sourceType: Subscription.Url
    
    title: qsTr("Subscription properties")
    height: flow.height + platformStyle.paddingMedium
  
    Flow {
        id: flow
    
        anchors {
            left: parent.left
            right: parent.right
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
        
        CheckBox {
            id: enclosuresCheckBox
            
            width: parent.width - acceptButton.width - parent.spacing
            text: qsTr("Download enclosures automatically")
        }
        
        Button {
            id: acceptButton
            
            style: DialogButtonStyle {}
            text: qsTr("Done")
            enabled: sourceField.text != ""
            onClicked: root.accept()
        }
    }

    Subscription {
        id: subscription
        
        onStatusChanged: {
            if (status == Subscription.Ready) {
                sourceField.text = source;
                root.sourceType = sourceType;
                enclosuresCheckBox.checked = downloadEnclosures;
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
            
            if (subscriptionId > 0) {
                subscription.load(subscriptionId);
            }
        }
    }
    onAccepted: {
        if (subscriptionId > 0) {
            database.updateSubscription(subscriptionId, {source: sourceField.text,
                                        downloadEnclosures: enclosuresCheckBox.checked ? 1 : 0});
        }
        else {
            subscriptions.create(sourceField.text, sourceType, enclosuresCheckBox.checked);
        }
    }
}
