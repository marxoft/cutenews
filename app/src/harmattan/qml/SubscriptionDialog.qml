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

import QtQuick 1.1
import com.nokia.meego 1.0
import cuteNews 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MySheet {
    id: root
    
    property int subscriptionId: -1
    property int sourceType: Subscription.Url
    
    acceptButtonText: sourceField.text ? subscriptionId > 0 ? qsTr("Save") : qsTr("Subscribe") : ""
    rejectButtonText: qsTr("Cancel")
    
    Flickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: column.height + UI.PADDING_DOUBLE
          
        Column {
            id: column
    
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: UI.PADDING_DOUBLE
            }
            spacing: UI.PADDING_DOUBLE
            
            Label {
                width: parent.width
                font.pixelSize: UI.FONT_XLARGE
                text: subscriptionId > 0 ? qsTr("Edit feed") : qsTr("Subscribe to feed")
            }
            
            Image {
                width: parent.width
                source: "image://theme/meegotouch-groupheader-inverted-background"
            }
        
            Label {
                width: parent.width
                font.pixelSize: UI.FONT_SMALL
                font.family: UI.FONT_FAMILY_LIGHT
                text: sourceType == Subscription.LocalFile ? qsTr("File path") : sourceType == Subscription.Command
                                                           ? qsTr("Command") : qsTr("Address")
            }
            
            MyTextField {
                id: sourceField
            
                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onAccepted: platformCloseSoftwareInputPanel()
            }
        
            MySwitch {
                id: enclosuresSwitch
            
                width: parent.width
                text: qsTr("Download enclosures")
            }
            
            MySwitch {
                id: homescreenSwitch
            
                width: parent.width
                text: qsTr("Show feed on home screen")
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }

    Subscription {
        id: subscription
        
        onStatusChanged: {
            if (status == Subscription.Ready) {
                sourceField.text = source;
                root.sourceType = sourceType;
                enclosuresSwitch.checked = downloadEnclosures;
            }
        }
    }
    
    QtObject {
        id: dialogs
        
        property FileBrowserDialog fileDialog        
        
        function showFileDialog() {
            if (!fileDialog) {
                fileDialog = fileDialogComponent.createObject(root);
            }
            
            fileDialog.open();
        }
    }
    
    Component {
        id: fileDialogComponent
        
        FileBrowserDialog {
            showFiles: true
            onFileChosen: sourceField.text = filePath
        }
    }
    
    onStatusChanged: {
        if (status == DialogStatus.Opening) {
            sourceField.forceActiveFocus();
            
            if (subscriptionId > 0) {
                subscription.load(subscriptionId);
                homescreenSwitch.checked = eventfeed.subscriptionIsPublished(subscriptionId);
            }
        }
    }
    onAccepted: {
        eventfeed.setSubscriptionPublished(subscriptionId, homescreenSwitch.checked);
        
        if (subscriptionId > 0) {
            database.updateSubscription(subscriptionId, {source: sourceField.text,
                                        downloadEnclosures: enclosuresSwitch.checked ? 1 : 0});
        }
        else {
            subscriptions.create(sourceField.text, sourceType, enclosuresSwitch.checked);
        }
        
        sourceField.clear();
        sourceType = Subscription.Url;
        enclosuresSwitch.checked = false;
        homescreenSwitch.checked = false;
    }
    onRejected: {
        sourceField.clear();
        sourceType = Subscription.Url;
        enclosuresSwitch.checked = false;
        homescreenSwitch.checked = false;
    }
}
