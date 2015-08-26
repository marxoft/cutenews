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
    property string pluginName
        
    title: qsTr("Subscription properties")
    height: Math.min(350, container.height + platformStyle.paddingMedium)
    
    Flickable {
        id: flickable
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        contentHeight: container.height
        
        Item {
            id: container
            
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: column.height + enclosuresCheckBox.height + platformStyle.paddingMedium
            
            Column {
                id: column
            
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                spacing: platformStyle.paddingMedium
                
                Repeater {
                    id: repeater
                
                    Loader {
                        width: column.width
                        height: item ? item.height : 0
                    }
                }
            }
            
            CheckBox {
                id: enclosuresCheckBox
        
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                text: qsTr("Download enclosures automatically")
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
        onClicked: root.accept()
    }
    
    Component {
        id: booleanComponent
        
        CheckBox {
            id: checkBox
            
            property alias title: checkBox.text
            property string key
            property alias value: checkBox.checked
            
            onClicked: internal.setParam(key, value)
        }
    }
    
    Component {
        id: integerComponent
        
        Item {
            property alias title: label.text
            property string key
            property alias min: spinbox.minimum
            property alias max: spinbox.maximum
            property alias step: spinbox.singleStep
            property alias value: spinbox.value
            
            width: parent.width
            height: label.height + spinbox.height + platformStyle.paddingMedium
            
            Label {
                id: label
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                elide: Text.ElideRight
                text: parent.title
            }
            
            SpinBox {
                id: spinbox
                
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                onValueChanged: internal.setParam(key, value)
            }
        }
    }
    
    Component {
        id: listButtonComponent
        
        ValueButton {
            id: valueButton
            
            property alias title: valueButton.text            
        }
    }
    
    Component {
        id: listSelectorComponent
        
        ListPickSelector {
            property string key
            
            model: SelectionModel {}
            textRole: "name"
            onSelected: internal.setParam(key, model.data(currentIndex, "value"))
        }
    }
    
    Component {
        id: textComponent
        
        Item {
            property alias title: label.text
            property string key
            property alias value: textField.text
            
            width: parent.width
            height: label.height + textField.height + platformStyle.paddingMedium
            
            Label {
                id: label
                
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                elide: Text.ElideRight
                text: parent.title
            }
            
            TextField {
                id: textField
                
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                onTextChanged: internal.setParam(key, value)
            }
        }
    }
    
    Subscription {
        id: subscription
        
        onStatusChanged: {
            if (status == Subscription.Ready) {
                pluginName = source.pluginName;
                enclosuresCheckBox.checked = downloadEnclosures
            
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
                            loader.item.value = getParam(loader.item.key, findAttribute(node, "default", 0));
                        }
                        else if (node.nodeName === "list") {
                            loader.sourceComponent = listButtonComponent;
                            loader.item.title = findAttribute(node, "title");                            
                            loader.item.pickSelector = listSelectorComponent.createObject(root,
                                                       {key: findAttribute(node, "name")});
                        
                            for (var j = 0; j < node.childNodes.length; j++) {
                                var listNode = node.childNodes[j];

                                if (listNode.nodeName === "element") {
                                    loader.item.pickSelector.model.append(findAttribute(listNode, "name"),
                                                                          findAttribute(listNode, "value"));
                                }
                            }
                        
                            loader.item.pickSelector.currentIndex = loader.item.pickSelector.model.match("value",
                            getParam(loader.item.pickSelector.key, findAttribute(node, "default")));
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
        if (status == DialogStatus.Open) {
            internal.params = null;
            
            if (subscriptionId > 0) {
                subscription.load(subscriptionId);
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
        if (subscriptionId > 0) {
            database.updateSubscription(subscriptionId, {source: internal.stringifySource(),
                                        downloadEnclosures: enclosuresCheckBox.checked ? 1 : 0});
        }
        else {
            subscriptions.create(internal.stringifySource(), Subscription.Plugin, enclosuresCheckBox.checked);
        }
        
        enclosuresCheckBox.checked = false;
    }
    onRejected: enclosuresCheckBox.checked = false
}
    