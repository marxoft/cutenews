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

#include "article.h"
#include "articlemodel.h"
#include "cachingnetworkaccessmanagerfactory.h"
#include "categorymodel.h"
#include "categorynamemodel.h"
#include "clipboard.h"
#include "concurrenttransfersmodel.h"
#include "cutenews.h"
#include "database.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "loggerverbositymodel.h"
#include "maskeditem.h"
#include "networkproxytypemodel.h"
#include "pluginconfigmodel.h"
#include "pluginmanager.h"
#include "pluginsettings.h"
#include "screenorientationmodel.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionmodel.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "transfermodel.h"
#include "transferprioritymodel.h"
#include "transfers.h"
#include "utils.h"
#include <QApplication>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <qdeclarative.h>
#include <QSsl>
#include <QSslConfiguration>
#include <QThread>

inline void registerTypes() {
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
    qRegisterMetaType<DBConnection::Status>("DBConnection::Status");
    qmlRegisterType<Article>("cuteNews", 1, 0, "Article");
    qmlRegisterType<ArticleModel>("cuteNews", 1, 0, "ArticleModel");
    qmlRegisterType<CategoryModel>("cuteNews", 1, 0, "CategoryModel");
    qmlRegisterType<CategoryNameModel>("cuteNews", 1, 0, "CategoryNameModel");
    qmlRegisterType<ConcurrentTransfersModel>("cuteNews", 1, 0, "ConcurrentTransfersModel");
    qmlRegisterType<LoggerVerbosityModel>("cuteNews", 1, 0, "LoggerVerbosityModel");
    qmlRegisterType<MaskedItem>("cuteNews", 1, 0, "MaskedItem");
    qmlRegisterType<NetworkProxyTypeModel>("cuteNews", 1, 0, "NetworkProxyTypeModel");
    qmlRegisterType<PluginConfigModel>("cuteNews", 1, 0, "PluginConfigModel");
    qmlRegisterType<PluginSettings>("cuteNews", 1, 0, "PluginSettings");
    qmlRegisterType<ScreenOrientationModel>("cuteNews", 1, 0, "ScreenOrientationModel");
    qmlRegisterType<SelectionModel>("cuteNews", 1, 0, "SelectionModel");
    qmlRegisterType<Subscription>("cuteNews", 1, 0, "Subscription");
    qmlRegisterType<SubscriptionModel>("cuteNews", 1, 0, "SubscriptionModel");
    qmlRegisterType<SubscriptionSourceTypeModel>("cuteNews", 1, 0, "SubscriptionSourceTypeModel");
    qmlRegisterType<TransferModel>("cuteNews", 1, 0, "TransferModel");
    qmlRegisterType<TransferPriorityModel>("cuteNews", 1, 0, "TransferPriorityModel");
    
    qmlRegisterUncreatableType<DBConnection>("cuteNews", 1, 0, "DBConnection", "");
    qmlRegisterUncreatableType<FeedPluginConfig>("cuteNews", 1, 0, "FeedPluginConfig", "");
    qmlRegisterUncreatableType<Subscriptions>("cuteNews", 1, 0, "Subscriptions", "");
    qmlRegisterUncreatableType<Transfer>("cuteNews", 1, 0, "Transfer", "");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cuteNews");
    app.setApplicationName("cuteNews");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setQuitOnLastWindowClosed(false);

    Logger::setFileName(Settings::loggerFileName());
    Logger::setVerbosity(Settings::loggerVerbosity());

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(config);

    initDatabase();
    registerTypes();
    Settings::setNetworkProxy();

    QScopedPointer<Clipboard> clipboard(Clipboard::instance());
    QScopedPointer<CuteNews> cutenews(CuteNews::instance());
    QScopedPointer<DBNotify> notify(DBNotify::instance());
    QScopedPointer<PluginManager> plugins(PluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<Transfers> transfers(Transfers::instance());
    
    Logger logger;
    Utils utils;
    QThread thread;
    thread.start();
    DBConnection::setAsynchronousThread(&thread);
    DBConnection connection(true);
    
    QDeclarativeView view;
    view.engine()->setNetworkAccessManagerFactory(new CachingNetworkAccessManagerFactory);
    QDeclarativeContext *context = view.rootContext();
    
    context->setContextProperty("clipboard", clipboard.data());
    context->setContextProperty("cutenews", cutenews.data());
    context->setContextProperty("database", &connection);
    context->setContextProperty("qmlview", &view);
    context->setContextProperty("logger", &logger);
    context->setContextProperty("notifier", notify.data());
    context->setContextProperty("plugins", plugins.data());
    context->setContextProperty("settings", settings.data());
    context->setContextProperty("subscriptions", subscriptions.data());
    context->setContextProperty("transfers", transfers.data());
    context->setContextProperty("utils", &utils);
    context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
    context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);

    view.setSource(QUrl::fromLocalFile(app.applicationDirPath() + "/qml/main.qml"));
    view.showFullScreen();
    Logger::log(view.errors());
    
    QObject::connect(&app, SIGNAL(lastWindowClosed()), cutenews.data(), SLOT(quit()));
    QObject::connect(view.engine(), SIGNAL(warnings(QList<QDeclarativeError>)),
                     &logger, SLOT(log(QList<QDeclarativeError>)));
    QObject::connect(settings.data(), SIGNAL(loggerFileNameChanged(QString)), &logger, SLOT(setFileName(QString)));
    QObject::connect(settings.data(), SIGNAL(loggerVerbosityChanged(int)), &logger, SLOT(setVerbosity(int)));
    QObject::connect(settings.data(), SIGNAL(offlineModeEnabledChanged(bool)),
                     subscriptions.data(), SLOT(setOfflineModeEnabled(bool)));
    QObject::connect(settings.data(), SIGNAL(maximumConcurrentTransfersChanged(int)),
                     transfers.data(), SLOT(setMaximumConcurrentTransfers(int)));
    
    return app.exec();
}
