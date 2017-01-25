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

MyPage {
    id: root
    
    title: qsTr("Categories")
    tools: ToolBarLayout {
        BackToolButton {}
        
        MyToolButton {
            iconSource: "toolbar-add"
            toolTip: qsTr("New category")
            onClicked: appWindow.pageStack.push(categoryPage)
        }
    }
    
    MyListView {
        id: view
        
        anchors.fill: parent
        model: CategoryModel {
            id: categoryModel
        }
        delegate: DualTextDelegate {
            text: name
            subText: value
            onClicked: appWindow.pageStack.push(categoryPage, {name: name, path: value})
            onPressAndHold: popups.open(contextMenu, root)
        }
    }

    ScrollDecorator {
        flickableItem: view
    }

    Label {
        id: label

        anchors {
            fill: parent
            margins: platformStyle.paddingLarge
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        color: platformStyle.colorNormalMid
        font.bold: true
        font.pixelSize: 32
        text: qsTr("No Categories")
        visible: categoryModel.count === 0
    }
    
    Component {
        id: contextMenu
        
        MyContextMenu {
            focusItem: view
            
            MenuLayout {
                MenuItem {
                    text: qsTr("Edit")
                    onClicked: appWindow.pageStack.push(categoryPage, {name: categoryModel.data(view.currentIndex, "name"),
                                                            path: categoryModel.data(view.currentIndex, "value")});
                }
                
                MenuItem {
                    text: qsTr("Remove")
                    onClicked: {
                        categoryModel.remove(view.currentIndex);
                        categoryModel.save();
                    }
                }
            }
        }
    }

    Component {
        id: categoryPage

        EditCategoryPage {
            onAccepted: {
                categoryModel.append(name, path);
                categoryModel.save();
                appWindow.pageStack.pop();
            }
        }
    }
}
