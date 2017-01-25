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

    property int hour: 0
    property int minute: 0
    property Item focusItem: null
    property bool _ready: false
    
    signal accepted
    signal rejected
    
    function _updateSubTitle() {
        subTitle = (hour > 9 ? hour : "0" + hour) + ":" + (minute > 9 ? minute : "0" + minute);
    }

    Loader {
        id: loader
    }

    Component {
        id: dialog

        TimePickerDialog {
            titleText: root.title
            acceptButtonText: qsTr("Done")
            hour: root.hour
            minute: root.minute
            onClickedOutside: reject()
            onAccepted: {
                root.hour = hour;
                root.minute = minute;
                root.accepted();
            }
            onRejected: root.rejected()
            onStatusChanged: if ((status == DialogStatus.Closed) && (root.focusItem)) root.focusItem.forceActiveFocus();
        }
    }

    onHourChanged: if (_ready) _updateSubTitle();
    onMinuteChanged: if (_ready) _updateSubTitle();
    onClicked: {
        loader.sourceComponent = dialog;
        loader.item.open();
    }

    Component.onCompleted: {
        _updateSubTitle();
        _ready = true;
    }
}
