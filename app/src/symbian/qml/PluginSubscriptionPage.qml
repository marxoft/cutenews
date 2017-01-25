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
    property string pluginId

    title: qsTr("Subscription properties")
        
    Subscription {
        id: subscription
        
        onStatusChanged: {
            switch (status) {
            case Subscription.Active:
                root.showProgressIndicator = true;
                return;
            case Subscription.Ready:
                root.pluginId = source.pluginId;
                break;
            default:
                break;
            }
            
            root.showProgressIndicator = false;
        }
    }
    
    QtObject {
        id: internal
        
        property variant settings
        
        function getValue(key, defaultValue) {
            if ((settings) && (settings.hasOwnProperty(key))) {
                return settings[key];
            }
            
            return defaultValue;
        }
        
        function setValue(key, value) {
            var p = settings ? settings : {};
            p[key] = value;
            settings = p;
        }
                
        function stringifySource() {
            var s = {};
            s["pluginId"] = pluginId;
            s["settings"] = settings;
            return JSON.stringify(s);
        }
        
        function load() {
            settings = subscription.source.settings;
            repeater.model = plugins.getConfig(root.pluginId).settings;
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
            
            Repeater {
                id: repeater
                
                Loader {
                    function initSourceComponent() {
                        switch (modelData.type) {
                        case "boolean":
                            sourceComponent = checkBox;
                            break;
                        case "group":
                            sourceComponent = group;
                            break;
                        case "integer":
                            sourceComponent = integerField;
                            break;
                        case "list":
                            sourceComponent = valueSelector;
                            break;
                        case "password":
                            sourceComponent = passwordField;
                            break;
                        case "text":
                            sourceComponent = textField;
                            break;
                        default:
                            break;
                        }

                        if (item) {
                            item.init(modelData);
                        }
                    }

                    Component.onCompleted: initSourceComponent()
                }
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
        id: checkBox
        
        MySwitch {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key;
                text = modelData.label;
                checked = internal.getValue(key, modelData.value) === true;
            }

            width: flow.width
            visible: !inputContext.visible
            onCheckedChanged: internal.setValue(key, checked)
        }
    }
    
    Component {
        id: group
        
        Column {            
            function init(modelData) {
                label.text = modelData.label;
                repeater.key = modelData.key;
                repeater.model = modelData.settings;
            }

            width: flow.width
            spacing: platformStyle.paddingLarge
            
            HeaderLabel {
                id: label

                width: parent.width
                visible: !inputContext.visible
            }
            
            Repeater {
                id: repeater

                property string key
                
                Loader {
                    function initSourceComponent() {
                        switch (modelData.type) {
                        case "boolean":
                            sourceComponent = checkBox;
                            break;
                        case "integer":
                            sourceComponent = integerField;
                            break;
                        case "list":
                            sourceComponent = valueSelector;
                            break;
                        case "password":
                            sourceComponent = passwordField;
                            break;
                        case "text":
                            sourceComponent = textField;
                            break;
                        default:
                            break;
                        }

                        if (item) {
                            item.init(modelData, repeater.key);
                        }
                    }

                    Component.onCompleted: initSourceComponent()
                }
            }
        }
    }
    
    Component {
        id: integerField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.validator.bottom = Math.max(0, parseInt(modelData.minimum));
                field.validator.top = Math.max(1, parseInt(modelData.maximum));
                field.text = internal.getValue(field.key, modelData.value);
            }

            width: flow.width
            spacing: platformStyle.paddingLarge
            
            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
                visible: field.visible
            }
            
            MyTextField {
                id: field

                property string key

                width: parent.width
                validator: IntValidator {}
                inputMethodHints: Qt.ImhDigitsOnly
                visible: (!inputContext.visible) || (activeFocus)
                onTextChanged: internal.setValue(key, text)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }
    
    Component {
        id: passwordField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = internal.getValue(field.key, modelData.value);
            }

            width: flow.width
            spacing: platformStyle.paddingLarge
            
            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
                visible: field.visible
            }
            
            MyTextField {
                id: field

                property string key

                width: parent.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                visible: (!inputContext.visible) || (activeFocus)
                onTextChanged: internal.setValue(key, text)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }
    
    Component {
        id: textField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = internal.getValue(field.key, modelData.value);
            }

            width: flow.width
            spacing: platformStyle.paddingLarge
            
            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
                visible: field.visible
            }
            
            MyTextField {
                id: field

                property string key

                width: parent.width
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                visible: (!inputContext.visible) || (activeFocus)
                onTextChanged: internal.setValue(key, text)
                onAccepted: closeSoftwareInputPanel()
            }
        }
    }
    
    Component {
        id: valueSelector
        
        ValueSelector {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key
                title = modelData.label;

                for (var i = 0; i < modelData.options.length; i++) {
                    var option = modelData.options[i];
                    model.append(option.label, option.value);
                }

                value = internal.getValue(key, modelData.value);
            }

            x: -platformStyle.paddingLarge
            width: flow.width + platformStyle.paddingLarge * 2
            model: SelectionModel {}
            visible: !inputContext.visible
            onAccepted: internal.setValue(key, value)
        }
    }
    
    onSubscriptionIdChanged: subscription.load(subscriptionId)
    onPluginIdChanged: internal.load()
    onAccepted: {
        var source = internal.stringifySource();
        var downloadEnclosures = enclosuresSwitch.checked;
        var updateInterval = parseInt(updateField.text) * updateButtons.checkedButton.mins;
        
        if (subscription.id) {
            subscription.update({"source": source, "downloadEnclosures": downloadEnclosures ? 1 : 0,
            "updateInterval": updateInterval});
        }
        else {
            subscriptions.create(source, Subscription.Plugin, downloadEnclosures, updateInterval);
        }
        
        appWindow.pageStack.pop();
    }
}
