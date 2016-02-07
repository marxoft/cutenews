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
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MyPage {
    id: root

    title: qsTr("General")
    tools: ToolBarLayout {

        BackToolIcon {}
    }
    
    PageHeader {
        id: header
        
        title: root.title
    }

    Flickable {
        id: flicker

        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
        }
        contentHeight: column.height + UI.PADDING_DOUBLE

        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            spacing: UI.PADDING_DOUBLE
            
            ValueListItem {
                width: parent.width
                title: qsTr("Download path")
                subTitle: settings.downloadPath
                onClicked: {
                    loader.sourceComponent = pathDialog;
                    loader.item.open();
                }
            }

            MySwitch {
                x: UI.PADDING_DOUBLE
                width: parent.width - UI.PADDING_DOUBLE * 2
                text: qsTr("Start downloads automatically")
                checked: settings.startTransfersAutomatically
                onCheckedChanged: settings.startTransfersAutomatically = checked
            }
        }
    }

    ScrollDecorator {
        flickableItem: flicker
    }
    
    Loader {
        id: loader
    }

    Component {
        id: pathDialog

        FileBrowserDialog {
            onFileChosen: settings.downloadPath = filePath
        }
    }
}
