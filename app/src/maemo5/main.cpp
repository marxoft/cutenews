/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "article.h"
#include "articlemodel.h"
#include "database.h"
#include "definitions.h"
#include "networkproxytypemodel.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionmodel.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "transfermodel.h"
#include "transfers.h"
#include "urlopener.h"
#include "urlopenermodel.h"
#include "userinterfacemodel.h"
#include "utils.h"
#include "viewmodemodel.h"
#include <QSqlQuery>
#include <QThread>
#include <QApplication>
#include <QSsl>
#include <QSslConfiguration>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <qdeclarative.h>
#include <QDebug>

void registerTypes() {
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
    qmlRegisterType<Article>("cuteNews", 1, 0, "Article");
    qmlRegisterType<ArticleModel>("cuteNews", 1, 0, "ArticleModel");
    qmlRegisterType<NetworkProxyTypeModel>("cuteNews", 1, 0, "NetworkProxyTypeModel");
    qmlRegisterType<Subscription>("cuteNews", 1, 0, "Subscription");
    qmlRegisterType<SubscriptionModel>("cuteNews", 1, 0, "SubscriptionModel");
    qmlRegisterType<SubscriptionSourceTypeModel>("cuteNews", 1, 0, "SubscriptionSourceTypeModel");
    qmlRegisterType<Transfer>("cuteNews", 1, 0, "Transfer");
    qmlRegisterType<TransferModel>("cuteNews", 1, 0, "TransferModel");
    qmlRegisterType<UserInterfaceModel>("cuteNews", 1, 0, "UserInterfaceModel");
    qmlRegisterType<UrlOpenerModel>("cuteNews", 1, 0, "UrlOpenerModel");
    qmlRegisterType<ViewModeModel>("cuteNews", 1, 0, "ViewModeModel");
    qmlRegisterUncreatableType<Subscriptions>("cuteNews", 1, 0, "Subscriptions", "");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cuteNews");
    app.setApplicationName("cuteNews");
    app.setApplicationVersion(VERSION_NUMBER);
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
    
    QScopedPointer<Database> database(Database::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<Transfers> transfers(Transfers::instance());
    QScopedPointer<UrlOpener> urlopener(UrlOpener::instance());
    Utils utils;
    
    QThread thread;
    Database::instance()->moveToThread(&thread);
    thread.start();
    
    Settings::instance()->setNetworkProxy();
    
    registerTypes();
    
    QDeclarativeEngine engine;
    QDeclarativeContext *context = engine.rootContext();
    context->setContextProperty("database", Database::instance());
    context->setContextProperty("downloads", Transfers::instance());
    context->setContextProperty("settings", Settings::instance());
    context->setContextProperty("subscriptions", Subscriptions::instance());
    context->setContextProperty("urlopener", UrlOpener::instance());
    context->setContextProperty("utils", &utils);
    context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
    context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
    
    QDeclarativeComponent component(&engine, "/opt/cutenews/qml/main.qml");
    component.create();
    
    if (component.isError()) {
        foreach (QDeclarativeError error, component.errors()) {
            qWarning() << error.toString();
        }
        
        return 0;
    }
    
    QObject::connect(&app, SIGNAL(aboutToQuit()), &thread, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), Transfers::instance(), SLOT(save()));
    
    return app.exec();
}
