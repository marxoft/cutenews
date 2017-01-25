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

AppWindow {
    id: appWindow

    showStatusBar: true
    showToolBar: true
    initialPage: MainPage {
        id: mainPage
    }

    MyInfoBanner {
        id: infoBanner
    }
    
    PopupLoader {
        id: popups
    }

    Component.onCompleted: {
        plugins.load();
        transfers.load();
        
        if ((settings.updateSubscriptionsOnStartup) && (!settings.offlineModeEnabled)) {
            subscriptions.updateAll();
        };
        
        notifier.error.connect(function(errorString) {
            infoBanner.information(errorString);
        });

        notifier.readArticlesDeleted.connect(function(count) {
            infoBanner.information(count + " " + qsTr("read articles deleted"));
        });

        transfers.transferAdded.connect(function() {
            infoBanner.information(qsTr("Enclosure added to downloads"));
        });
    }
}
