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
    
    property alias pluginId: plugin.pluginId
    property alias pluginSettings: repeater.model
        
    title: qsTr("Enclosure settings")
    height: Math.min(360, column.height + platformStyle.paddingMedium)
    
    PluginSettings {
        id: plugin
    }
    
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
        contentHeight: column.height
        
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
                checked = plugin.value(key, modelData.value) === true;
            }

            width: column.width
            onCheckedChanged: plugin.setValue(key, checked)
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
                
                field.value = plugin.value(field.key, modelData.value) || 0;
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
                onValueChanged: plugin.setValue(key, value)
            }
        }
    }
    
    Component {
        id: passwordField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = plugin.value(field.key, modelData.value) || "";
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
                onTextChanged: plugin.setValue(key, text)
            }
        }
    }
    
    Component {
        id: textField
        
        Column {
            function init(modelData, group) {
                label.text = modelData.label;
                field.key = (group ? group + "/" : "") + modelData.key;
                field.text = plugin.value(field.key, modelData.value) || "";
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
                onTextChanged: plugin.setValue(key, text)
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

                value = plugin.value(key, modelData.value) || model.data(0, "value");
            }

            width: column.width
            model: SelectionModel {}
            onSelected: plugin.setValue(key, value)
        }
    }
}
