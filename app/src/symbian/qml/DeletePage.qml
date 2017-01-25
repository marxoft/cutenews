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
    
    title: qsTr("Delete read articles")
    
    KeyNavFlickable {
        id: flickable
        
        anchors.fill: parent
        contentHeight: column.height + platformStyle.paddingLarge
        
        Column {
            id: column
            
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                topMargin: platformStyle.paddingLarge
            }
            
            Label {
                x: platformStyle.paddingLarge
                width: parent.width - platformStyle.paddingLarge * 2
                elide: Text.ElideRight
                text: qsTr("Delete articles read before")
            }

            Item {
                width: parent.width
                height: platformStyle.paddingLarge
            }
            
            DateSelector {
                id: dateSelector
                
                width: parent.width
                title: qsTr("Date")
            }
            
            TimeSelector {
                id: timeSelector
                
                width: parent.width
                title: qsTr("Time")
            }
        }
    }
    
    ScrollDecorator {
        flickableItem: flickable
    }
    
    Connections {
        target: database
        onStatusChanged: {
            switch (database.status) {
            case DBConnection.Active:
                root.showProgressIndicator = true;
                return;
            default:
                break;
            }
            
            root.showProgressIndicator = false;
            appWindow.pageStack.pop();
        }
    }
    
    onStatusChanged: {
        if (status == PageStatus.Active) {
            var date = new Date(Date.now() - settings.readArticleExpiry * 1000);
            dateSelector.year = date.getFullYear();
            dateSelector.month = date.getMonth() + 1;
            dateSelector.day = date.getDate() + 1;
            timeSelector.hour = date.getHours();
            timeSelector.minute = date.getMinutes();
        }
    }
    onAccepted: {
        var date = new Date(dateSelector.year, dateSelector.month - 1, dateSelector.day - 1, timeSelector.hour,
        timeSelector.minute);
        var secs = utils.dateTimeToSecs(date);
        settings.readArticleExpiry = Date.now() / 1000 - secs;
        database.deleteReadArticles(secs);
    }
}
