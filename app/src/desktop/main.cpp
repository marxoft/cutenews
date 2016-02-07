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
#include "settings.h"
#include "subscriptionplugins.h"
#include "subscriptions.h"
#include "transfers.h"
#include <QApplication>
#include <QIcon>
#include <QSqlQuery>
#include <QThread>
#if QT_VERSION < 0x050000
#include <QSsl>
#include <QSslConfiguration>
#endif

void registerTypes() {
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cuteNews");
    app.setApplicationName("cuteNews");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setWindowIcon(QIcon::fromTheme("cutenews"));
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);

#if QT_VERSION < 0x050000
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
    QIcon::setThemeName("Lubuntu");
#endif  
    
    QScopedPointer<Database> database(Database::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<Transfers> transfers(Transfers::instance());
    
    CuteNews cutenews;
    
    Database::init();
    Settings::instance()->setNetworkProxy();
    Transfers::instance()->load();
    SubscriptionPlugins::load();
    
    QThread thread;
    Database::instance()->moveToThread(&thread);
    thread.start();
    
    registerTypes();
        
    QObject::connect(&app, SIGNAL(aboutToQuit()), &thread, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), Transfers::instance(), SLOT(save()));
    
    const QStringList args = app.arguments();
    
    if (args.contains("--window")) {
        cutenews.showWindow();
    }
    
    return app.exec();
}
