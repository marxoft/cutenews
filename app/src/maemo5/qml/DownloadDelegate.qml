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

ListItem {
    id: root
    
    Label {
        anchors {
            left: parent.left
            right: progressBar.left
            top: parent.top
            margins: platformStyle.paddingMedium
        }
        elide: Text.ElideRight
        text: fileName
    }
    
    Label {
        anchors {
            left: parent.left
            right: progressBar.left
            bottom: parent.bottom
            margins: platformStyle.paddingMedium
        }
        verticalAlignment: Text.AlignBottom
        elide: Text.ElideRight
        font.pointSize: platformStyle.fontSizeSmall
        color: status == Transfer.Failed ? platformStyle.attentionColor : platformStyle.secondaryTextColor
        text: statusString + ": " + (status == Transfer.Failed ? errorString : utils.formatBytes(bytesTransferred)
              + " / " + (size > 0 ? utils.formatBytes(size) : qsTr("unknown")))
    }
    
    ProgressBar {
        id: progressBar
        
        anchors {
            right: parent.right
            rightMargin: platformStyle.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        width: 150
        value: progress
        textVisible: true
        text: progress + "%"
    }
}
        