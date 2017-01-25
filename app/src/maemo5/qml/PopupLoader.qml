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

import QtQuick 1.0
import org.hildon.components 1.0

QtObject {    
    function load(popup, parent) {
        var obj = popup.createObject(parent);
        
        if ((obj) && (obj.hasOwnProperty("status"))) {
            obj.statusChanged.connect(function () { if (obj.status == DialogStatus.Closed) obj.destroy(); });
        }
        
        return obj
    }
    
    function open(popup, parent) {
        var obj = load(popup, parent);
        
        if (obj) {
            if (obj.hasOwnProperty("popup")) {
                obj.popup();
            }
            else {
                obj.open();
            }
        }
        
        return obj;
    }
}
