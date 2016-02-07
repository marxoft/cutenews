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
    property string pluginName
    
    acceptButtonText: subscriptionId > 0 ? qsTr("Save") : qsTr("Subscribe")
    rejectButtonText: qsTr("Cancel")
    
    Flickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: container.height + UI.PADDING_DOUBLE
        
        Item {
            id: container
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: UI.PADDING_DOUBLE
            }
            height: column.height + enclosuresSwitch.height + homescreenSwitch.height + UI.PADDING_DOUBLE * 2
            
            Column {
                id: column
            
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
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
                
                Repeater {
                    id: repeater
                
                    Loader {
                        width: column.width
                        height: item ? item.height : 0
                    }
                }
            }
            
            MySwitch {
                id: enclosuresSwitch
        
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: homescreenSwitch.top
                    bottomMargin: UI.PADDING_DOUBLE
                }
                text: qsTr("Download enclosures")
            }
            
            MySwitch {
                id: homescreenSwitch
        
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                text: qsTr("Show feed on home screen")
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    Component {
        id: booleanComponent
        
        MySwitch {
            id: switchItem
            
            property alias title: switchItem.text
            property string key
            property alias value: switchItem.checked
            
            onCheckedChanged: internal.setParam(key, value)
        }
    }
    
    Component {
        id: integerComponent
        
        Item {
            property alias title: label.text
            property string key
            property alias min: slider.minimumValue
            property alias max: slider.maximumValue
            property alias step: slider.stepSize
            property alias value: slider.value
            
            width: parent.width
            height: label.height + slider.height + UI.PADDING_DOUBLE
            
            Label {
                id: label
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                elide: Text.ElideRight
                font.pixelSize: UI.FONT_SMALL
                font.family: UI.FONT_FAMILY_LIGHT
                text: parent.title
            }
            
            MySlider {
                id: slider
                
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                orientation: Qt.Horizontal
                valueIndicatorVisible: true
                onValueChanged: internal.setParam(key, value)
            }
        }
    }
    
    Component {
        id: listComponent
        
        ValueSelector {
            property string key
            
            x: -UI.PADDING_DOUBLE
            width: parent.width + UI.PADDING_DOUBLE * 4
            model: SelectionModel {}
            onAccepted: internal.setParam(key, model.data(selectedIndex, "value"))
        }
    }
    
    Component {
        id: textComponent
        
        Item {
            property alias title: label.text
            property string key
            property alias value: textField.text
            
            width: parent.width
            height: label.height + textField.height + UI.PADDING_DOUBLE
            
            Label {
                id: label
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                elide: Text.ElideRight
                font.pixelSize: UI.FONT_SMALL
                font.family: UI.FONT_FAMILY_LIGHT
                text: parent.title
            }
            
            MyTextField {
                id: textField
                
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: internal.setParam(key, value)
                onAccepted: platformCloseSoftwareInputPanel()
            }
        }
    }
    
    Subscription {
        id: subscription
        
        onStatusChanged: {
            if (status == Subscription.Ready) {
                pluginName = source.pluginName;
                enclosuresSwitch.checked = downloadEnclosures
            
                if (source.params) {
                    internal.params = source.params;                
                }
            
                if (plugins.hasParams(pluginName)) {
                    internal.loadParams(plugins.filePath(pluginName));
                }
            }
        }
    }
    
    QtObject {
        id: internal
        
        property variant params
        
        function getParam(key, defaultValue) {
            if ((params) && (params.hasOwnProperty(key))) {
                return params[key];
            }
            
            return defaultValue;
        }
        
        function setParam(key, value) {
            var p = params ? params : {};
            p[key] = value;
            params = p;
        }
                
        function stringifySource() {
            var s = {};
            s["pluginName"] = pluginName;
            s["params"] = params;
            return JSON.stringify(s);
        }
        
        function loadParams(fileName) {
            var request = new XMLHttpRequest();
            request.onreadystatechange = function() {
                if (request.readyState === XMLHttpRequest.DONE) {
                    var paramsNode = request.responseXML.documentElement.childNodes[1];
                    var count = paramsNode.childNodes.length;
                    
                    if (count == 0) {
                        return;
                    }
                    
                    repeater.model = count - 1;
                                        
                    for (var i = 0; i < count; i++) {
                        var node = paramsNode.childNodes[i];
                        var loader = repeater.itemAt(i);
                    
                        if (node.nodeName === "boolean") {
                            loader.sourceComponent = booleanComponent;
                            loader.item.title = findAttribute(node, "title");
                            loader.item.key = findAttribute(node, "name");
                            loader.item.value = getParam(loader.item.key, findAttribute(node, "default") == "true");
                        }
                        else if (node.nodeName === "integer") {
                            loader.sourceComponent = integerComponent;
                            loader.item.title = findAttribute(node, "title");
                            loader.item.key = findAttribute(node, "name");
                            loader.item.min = findAttribute(node, "min", 0);
                            loader.item.max = findAttribute(node, "max", 100);
                            loader.item.step = findAttribute(node, "step", 1);
                            loader.item.value = getParam(loader.item.key, findAttribute(node, "default", 20));
                        }
                        else if (node.nodeName === "list") {
                            loader.sourceComponent = listComponent;
                            loader.item.title = findAttribute(node, "title");                            
                            loader.item.key = findAttribute(node, "name");
                        
                            for (var j = 0; j < node.childNodes.length; j++) {
                                var listNode = node.childNodes[j];

                                if (listNode.nodeName === "element") {
                                    loader.item.model.append(findAttribute(listNode, "name"),
                                                             findAttribute(listNode, "value"));
                                }
                            }
                        
                            loader.item.selectedIndex = loader.item.model.match("value",
                            getParam(loader.item.key, findAttribute(node, "default")));
                        }
                        else if (node.nodeName === "text") {
                            loader.sourceComponent = textComponent;
                            loader.item.title = findAttribute(node, "title");
                            loader.item.key = findAttribute(node, "name");
                            loader.item.value = getParam(loader.item.key, findAttribute(node, "default"));
                        }
                    }
                }
            }

            request.open("GET", fileName);
            request.send();
        }

        function findAttribute(node, name, defaultValue) {
            for (var i = 0; i < node.attributes.length; i++) {
                var att = node.attributes[i];

                if (att.name === name) {
                    return att.value;
                }
            }

            return defaultValue == undefined ? "" : defaultValue;
        }
    }
    
    onStatusChanged: {
        if (status == DialogStatus.Opening) {
            internal.params = null;
            
            if (subscriptionId > 0) {
                subscription.load(subscriptionId);
                homescreenSwitch.checked = eventfeed.subscriptionIsPublished(subscriptionId);
            }
            else if ((pluginName) && (plugins.hasParams(pluginName))) {
                internal.loadParams(plugins.filePath(pluginName));
            }
        }
        else if (status == DialogStatus.Closed) {
            repeater.model = null;
        }
    }
    onAccepted: {
        eventfeed.setSubscriptionPublished(subscriptionId, homescreenSwitch.checked);
        
        if (subscriptionId > 0) {
            database.updateSubscription(subscriptionId, {source: internal.stringifySource(),
                                        downloadEnclosures: enclosuresSwitch.checked ? 1 : 0});
        }
        else {
            subscriptions.create(internal.stringifySource(), Subscription.Plugin, enclosuresSwitch.checked);
        }
                
        enclosuresSwitch.checked = false;
        homescreenSwitch.checked = false;
    }
    onRejected: {
        enclosuresSwitch.checked = false;
        homescreenSwitch.checked = false;
    }
}
    