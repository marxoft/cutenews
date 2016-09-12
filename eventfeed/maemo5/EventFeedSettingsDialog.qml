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
import org.hildon.settings 1.0
import org.hildon.utils 1.0

Dialog {
    id: root

    title: "cuteNews"
    showProgressIndicator: process.state == Process.RunningState
    height: row.height + platformStyle.paddingMedium
    
    Row {
        id: row
        
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        spacing: platformStyle.paddingMedium
        
        CheckBox {
            id: checkBox
            
            width: row.width - button.width - row.spacing
            text: qsTr("Publish articles to event feed")
            checked: settings.enabled
        }
        
        Button {
            id: button
            
            style: DialogButtonStyle {}
            text: qsTr("Done")
            onClicked: {
                if (checkBox.checked == settings.enabled) {
                    root.accept();
                    return;
                }
                
                settings.publishArticles = checkBox.checked;
                process.command = checkBox.checked ? "/opt/cutenews/bin/enable_events"
                                                   : "/opt/cutenews/bin/disable_events";
                process.start();
            }
        }
    }
    
    Process {
        id: process
        
        onFinished: root.accept()
    }
    
    Settings {
        id: settings
        
        property bool enabled
        
        fileName: "/home/user/.config/cutenews/eventfeed"
    }
}
