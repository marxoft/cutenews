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

#include "activecolormodel.h"
#include "article.h"
#include "articlemodel.h"
#include "cutenews.h"
#include "database.h"
#include "definitions.h"
#include "eventfeed.h"
#include "maskeditem.h"
#include "networkproxytypemodel.h"
#include "screenorientationmodel.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionmodel.h"
#include "subscriptionplugins.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "transfers.h"
#include "utils.h"
#include <MDeclarativeCache>
#include <QApplication>
#include <QSqlQuery>
#include <QSsl>
#include <QSslConfiguration>
#include <QThread>
#include <qdeclarative.h>

void registerTypes() {
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
    qmlRegisterType<ActiveColorModel>("cuteNews", 1, 0, "ActiveColorModel");
    qmlRegisterType<Article>("cuteNews", 1, 0, "Article");
    qmlRegisterType<ArticleModel>("cuteNews", 1, 0, "ArticleModel");
    qmlRegisterType<MaskedItem>("cuteNews", 1, 0, "MaskedItem");
    qmlRegisterType<NetworkProxyTypeModel>("cuteNews", 1, 0, "NetworkProxyTypeModel");
    qmlRegisterType<ScreenOrientationModel>("cuteNews", 1, 0, "ScreenOrientationModel");
    qmlRegisterType<SelectionModel>("cuteNews", 1, 0, "SelectionModel");
    qmlRegisterType<Subscription>("cuteNews", 1, 0, "Subscription");
    qmlRegisterType<SubscriptionModel>("cuteNews", 1, 0, "SubscriptionModel");
    qmlRegisterType<SubscriptionSourceTypeModel>("cuteNews", 1, 0, "SubscriptionSourceTypeModel");
    qmlRegisterType<Transfer>("cuteNews", 1, 0, "Transfer");
    qmlRegisterUncreatableType<Subscriptions>("cuteNews", 1, 0, "Subscriptions", "");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QScopedPointer<QApplication> app(MDeclarativeCache::qApplication(argc, argv));
    app.data()->setOrganizationName("cuteNews");
    app.data()->setApplicationName("cuteNews");
    app.data()->setApplicationVersion(VERSION_NUMBER);
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
    
    QScopedPointer<Database> database(Database::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<Transfers> transfers(Transfers::instance());
    QScopedPointer<EventFeed> eventfeed(EventFeed::instance());
    
    CuteNews cutenews;
    
    Database::init();
    Settings::instance()->setNetworkProxy();
    Transfers::instance()->load();
    SubscriptionPlugins::load();
    
    QThread thread;
    Database::instance()->moveToThread(&thread);
    thread.start();
    
    registerTypes();
    
    QObject::connect(app.data(), SIGNAL(aboutToQuit()), &thread, SLOT(quit()));
    QObject::connect(app.data(), SIGNAL(aboutToQuit()), Transfers::instance(), SLOT(save()));
    
    const QStringList args = app.data()->arguments();
    
    if (args.contains("--window")) {
        cutenews.showWindow();
    }
    
    return app.data()->exec();
}
