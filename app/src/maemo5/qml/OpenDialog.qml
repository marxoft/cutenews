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

Dialog {
    id: root
    
    property string url
    
    title: qsTr("Open externally")
    height: flow.height + platformStyle.paddingMedium
    
    Flow {
        id: flow
        
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        spacing: platformStyle.paddingMedium
        
        Button {            
            id: directButton

            width: Math.floor((parent.width / 2) - (parent.spacing / 2))
            text: qsTr("Open directly")
            onClicked: {
                urlopener.open(root.url);
                root.accept();
            }
        }
        
        Button {
            id: pluginButton

            width: Math.floor((parent.width / 2) - (parent.spacing / 2))
            text: qsTr("Open with plugin")
            enabled: plugins.enclosureIsSupported(root.url)
            onClicked: {
                urlopener.openWithPlugin(root.url);
                root.accept();
            }
        }
    }

    contentItem.states: State {
        name: "Portrait"
        when: screen.currentOrientation == Qt.WA_Maemo5PortraitOrientation

        PropertyChanges {
            target: directButton
            width: parent.width
        }

        PropertyChanges {
            target: pluginButton
            width: parent.width
        }
    }
}
