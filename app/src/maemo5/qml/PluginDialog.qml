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
    property string pluginId
        
    title: qsTr("Subscription properties")
    height: 360
    
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
                top: parent.top
            }
            height: column.height + flow.height + platformStyle.paddingMedium * 6
            
            Column {
                id: column
            
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: flow.top
                    bottomMargin: platformStyle.paddingMedium
                }
                spacing: platformStyle.paddingMedium
                
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
            }
            
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
        id: checkBox
        
        CheckBox {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key;
                text = modelData.label;
                checked = internal.getValue(key, modelData.value) === true;
            }

            width: column.width
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

            width: column.width
            spacing: platformStyle.paddingLarge
            
            Label {
                id: label

                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: platformStyle.secondaryTextColor
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
                
                if (modelData.minimum) {
                    field.minimum = modelData.minimum;
                }
                
                if (modelData.maximum) {
                    field.maximum = modelData.maximum;
                }
                
                if (modelData.step) {
                    field.singleStep = modelData.step;
                }
                
                field.value = internal.getValue(field.key, modelData.value) || 0;
            }

            width: column.width
            spacing: platformStyle.paddingMedium
            
            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
            }
            
            SpinBox {
                id: field

                property string key

                width: parent.width
                onValueChanged: internal.setValue(key, value)
            }
        }
    }
    
    Component {
        id: passwordField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = internal.getValue(field.key, modelData.value) || "";
            }

            width: column.width
            spacing: platformStyle.paddingMedium
            
            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
            }
            
            TextField {
                id: field

                property string key

                width: parent.width
                echoMode: TextInput.Password
                onTextChanged: internal.setValue(key, text)
            }
        }
    }
    
    Component {
        id: textField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = internal.getValue(field.key, modelData.value) || "";
            }

            width: column.width
            spacing: platformStyle.paddingMedium
            
            Label {
                id: label

                width: parent.width
                elide: Text.ElideRight
            }
            
            TextField {
                id: field

                property string key

                width: parent.width
                onTextChanged: internal.setValue(key, text)
            }
        }
    }
    
    Component {
        id: valueSelector
        
        ListSelectorButton {
            property string key

            function init(modelData, group) {
                key = (group ? group + "/" : "") + modelData.key
                text = modelData.label;

                for (var i = 0; i < modelData.options.length; i++) {
                    var option = modelData.options[i];
                    model.append(option.label, option.value);
                }

                value = internal.getValue(key, modelData.value) || model.data(0, "value");
            }

            width: column.width
            model: SelectionModel {}
            onSelected: internal.setValue(key, value)
        }
    }
    
    Subscription {
        id: subscription
        
        onStatusChanged: {
            if (status == Subscription.Ready) {
                pluginId = source.pluginId;
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
                
                internal.settings = source.settings;
                repeater.model = plugins.getConfig(pluginId).settings;
            }
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
    }
    
    onStatusChanged: {
        if (status == DialogStatus.Opening) {
            internal.settings = null;
            
            if (subscriptionId) {
                subscription.load(subscriptionId);
            }
            else if (pluginId) {
                repeater.model = plugins.getConfig(pluginId).settings;
            }
        }
    }
    onAccepted: {
        var interval = updateIntervalSpinBox.value;
        
        if (interval > 0) {
            interval *= updateIntervalModel.data(updateIntervalSelector.currentIndex, "value");
        }
        
        if (subscriptionId) {
            database.updateSubscription(subscriptionId, {source: internal.stringifySource(),
                downloadEnclosures: enclosuresCheckBox.checked ? 1 : 0, updateInterval: interval});
        }
        else {
            subscriptions.create(internal.stringifySource(), Subscription.Plugin, enclosuresCheckBox.checked, interval);
        }        
    }
}
    
