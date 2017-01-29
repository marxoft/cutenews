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
    
    title: qsTr("Delete read articles")
    height: column.height + platformStyle.paddingMedium
    
    Column {
        id: column
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
        }
        spacing: platformStyle.paddingMedium
        
        Label {
            width: parent.width
            text: qsTr("Delete articles read before")
        }
        
        DateSelectorButton {
            id: dateSelector
            
            width: parent.width
            text: qsTr("Date")
        }
        
        TimeSelectorButton {
            id: timeSelector
            
            width: parent.width
            text: qsTr("Time")
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
        onClicked: {
            var date = dateTime.dateTime(dateSelector.year, dateSelector.month, dateSelector.day, timeSelector.hour,
            timeSelector.minute);
            var secs = utils.dateTimeToSecs(date);
            settings.readArticleExpiry = utils.dateTimeToSecs(dateTime.currentDateTime()) - secs;
            database.deleteReadArticles(secs);
        }
    }
    
    Connections {
        target: database
        onStatusChanged: {
            switch (database.status) {
            case DBConnection.Active:
                root.showProgressIndicator = true;
                return;
            case DBConnection.Ready:
                root.accept();
                break;
            default:
                root.reject();
                break;
            }
            
            root.showProgressIndicator - false;
        }
    }
    
    onStatusChanged: {
        if (status == DialogStatus.Open) {
            var date = utils.dateTimeFromSecs(Date.now() / 1000 - settings.readArticleExpiry);
            dateSelector.year = dateTime.year(date);
            dateSelector.month = dateTime.month(date);
            dateSelector.day = dateTime.day(date);
            timeSelector.hour = dateTime.hour(date);
            timeSelector.minute = dateTime.minute(date);
        }
    }
}
