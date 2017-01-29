/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import com.nokia.extras 1.1

ValueListItem {
    id: root

    property int year: 0
    property int month: 1
    property int day: 1
    property int minimumYear: 0
    property int maximumYear: 0
    property Item focusItem: null
    property bool _ready: false
    
    signal accepted
    signal rejected
    
    function _updateSubTitle() {
        subTitle = Qt.formatDateTime(new Date(year, month - 1, day - 1), "dd MMM yyyy");
    }

    Loader {
        id: loader
    }

    Component {
        id: dialog

        DatePickerDialog {
            titleText: root.title
            acceptButtonText: qsTr("Done")
            year: root.year
            month: root.month
            day: root.day
            minimumYear: root.minimumYear
            maximumYear: root.maximumYear
            onClickedOutside: reject()
            onAccepted: {
                root.year = year;
                root.month = month;
                root.day = day;
                root.accepted();
            }
            onRejected: root.rejected()
            onStatusChanged: if ((status == DialogStatus.Closed) && (root.focusItem)) root.focusItem.forceActiveFocus();
        }
    }

    onYearChanged: if (_ready) _updateSubTitle();
    onMonthChanged: if (_ready) _updateSubTitle();
    onDayChanged: if (_ready) _updateSubTitle();
    onClicked: {
        loader.sourceComponent = dialog;
        loader.item.open();
    }

    Component.onCompleted: {
        _updateSubTitle();
        _ready = true;
    }
}
