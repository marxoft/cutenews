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
    
    property string subscriptionId
    
    title: qsTr("Subscription properties")
    acceptable: sourceField.text != ""
    
    Subscription {
        id: subscription
        
        onStatusChanged: {
            switch (status) {
            case Subscription.Active:
                root.showProgressIndicator = true;
                return;
            case Subscription.Ready:
                internal.load();
                break;
            default:
                break;
            }
            
            root.showProgressIndicator = false;
        }
    }
    
    QtObject {
        id: internal
        
        function load() {
            sourceField.text = subscription.source;
            enclosuresSwitch.checked = subscription.downloadEnclosures;
            var interval = subscription.updateInterval;
            
            if (interval <= 0) {
                return;
            }
            
            if (interval % 1440 == 0) {
                interval /= 1440;
                daysButton.clicked();
            }
            else if (interval % 60 == 0) {
                interval /= 60;
                hoursButton.clicked();
            }
            else {
                minutesButton.clicked();
            }
            
            updateField.text = interval;
        }
    }
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: inputContext.visible ? height : flow.height + platformStyle.paddingLarge * 2
        
        Flow {
            id: flow
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: platformStyle.paddingLarge
            }
            spacing: platformStyle.paddingLarge
            
            Label {
                width: parent.width
                elide: Text.ElideRight
                text: qsTr("Source")
                visible: sourceField.visible
            }
            
            MyTextField {
                id: sourceField
                
                width: parent.width
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                visible: (!inputContext.visible) || (focus)
            }
            
            TextDelegate {
                id: browseButton
                
                width: parent.width
                flickableMode: true
                subItemIndicator: true
                text: qsTr("Browse")
                visible: (!inputContext.visible)
                onClicked: appWindow.pageStack.push(browserPage)
            }
            
            MySwitch {
                id: enclosuresSwitch
                
                width: parent.width
                text: qsTr("Download enclosures automatically")
                visible: !inputContext.visible
            }

            HeaderLabel {
                width: parent.width
                text: qsTr("Updates")
                visible: !inputContext.visible
            }
            
            Label {
                width: parent.width - updateField.width - parent.spacing
                height: updateField.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: qsTr("Update every")
                visible: updateField.visible
            }
            
            MyTextField {
                id: updateField
                
                inputMethodHints: Qt.ImhDigitsOnly
                visible: (!inputContext.visible) || (focus)
                text: "0"
            }
            
            ButtonRow {
                id: updateButtons
                
                width: parent.width
                visible: !inputContext.visible
                exclusive: true
                
                Button {
                    id: minutesButton
                    
                    property int mins: 1
                    
                    text: qsTr("Minutes")
                }
                
                Button {
                    id: hoursButton
                    
                    property int mins: 60
                    
                    text: qsTr("Hours")
                }
                
                Button {
                    id: daysButton
                    
                    property int mins: 1440
                    
                    text: qsTr("Days")
                }
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }

    Component {
        id: browserPage

        FileBrowserPage {
            title: qsTr("Source")
            showFiles: true
            startFolder: sourceField.text ? sourceField.text.substring(0, sourceField.text.lastIndexOf("/")) : "E:/"
            onAccepted: {
                sourceField.text = chosenUrl;
                appWindow.pageStack.pop();
            }
        }
    }
    
    onSubscriptionIdChanged: subscription.load(subscriptionId)
    onAccepted: {
        var source = sourceField.text;
        var downloadEnclosures = enclosuresSwitch.checked;
        var updateInterval = parseInt(updateField.text) * updateButtons.checkedButton.mins;
        
        if (subscription.id) {
            subscription.update({"source": source, "downloadEnclosures": downloadEnclosures ? 1 : 0,
                "updateInterval": updateInterval});
        }
        else {
            subscriptions.create(source, Subscription.LocalFile, downloadEnclosures, updateInterval);
        }
        
        appWindow.pageStack.pop();
    }
}
