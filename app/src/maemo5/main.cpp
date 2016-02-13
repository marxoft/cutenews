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

#include "cutenews.h"
#include "database.h"
#include "definitions.h"
#include "eventfeed.h"
#include "settings.h"
#include "subscriptionplugins.h"
#include "subscriptions.h"
#include "transfers.h"
#include <QThread>
#include <QApplication>
#include <QSsl>
#include <QSslConfiguration>

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cuteNews");
    app.setApplicationName("cuteNews");
    app.setApplicationVersion(VERSION_NUMBER);
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
    
    QScopedPointer<CuteNews> cutenews(CuteNews::instance());
    QScopedPointer<Database> database(Database::instance());
    QScopedPointer<EventFeed> eventfeed(EventFeed::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<Transfers> transfers(Transfers::instance());    
    
    Database::init();
    Settings::setNetworkProxy();
    SubscriptionPlugins::load();
    Transfers::instance()->load();
    
    QThread thread;
    Database::instance()->moveToThread(&thread);
    thread.start();
    
    QObject::connect(&app, SIGNAL(aboutToQuit()), &thread, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), Transfers::instance(), SLOT(save()));
    
    const QStringList args = app.arguments();
    
    if (args.contains("--window")) {
        cutenews.data()->showWindow();
    }
    
    if (args.contains("--widget")) {
        cutenews.data()->showWidget();
    }
    
    return app.exec();
}
