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

#include "article.h"
#include "articlemodel.h"
#include "articlerequest.h"
#include "categorynamemodel.h"
#include "concurrenttransfersmodel.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "logger.h"
#include "loggerverbositymodel.h"
#include "pluginmanager.h"
#include "screenorientationmodel.h"
#include "serversettings.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionmodel.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "transfermodel.h"
#include "transferprioritymodel.h"
#include "updateintervaltypemodel.h"
#include "urlopenermodel.h"
#include "utils.h"
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <qdeclarative.h>
#include <QSsl>
#include <QSslConfiguration>

inline void registerTypes() {
    qRegisterMetaType<ArticleRequest::Status>("ArticleRequest::Status");
    qRegisterMetaType<DBConnection::Status>("DBConnection::Status");
    qmlRegisterType<Article>("cuteNews", 1, 0, "Article");
    qmlRegisterType<ArticleModel>("cuteNews", 1, 0, "ArticleModel");
    qmlRegisterType<ArticleRequest>("cuteNews", 1, 0, "ArticleRequest");
    qmlRegisterType<CategoryNameModel>("cuteNews", 1, 0, "CategoryNameModel");
    qmlRegisterType<ConcurrentTransfersModel>("cuteNews", 1, 0, "ConcurrentTransfersModel");
    qmlRegisterType<DBConnection>("cuteNews", 1, 0, "DBConnection");
    qmlRegisterType<LoggerVerbosityModel>("cuteNews", 1, 0, "LoggerVerbosityModel");
    qmlRegisterType<ScreenOrientationModel>("cuteNews", 1, 0, "ScreenOrientationModel");
    qmlRegisterType<SelectionModel>("cuteNews", 1, 0, "SelectionModel");
    qmlRegisterType<Subscription>("cuteNews", 1, 0, "Subscription");
    qmlRegisterType<SubscriptionModel>("cuteNews", 1, 0, "SubscriptionModel");
    qmlRegisterType<SubscriptionSourceTypeModel>("cuteNews", 1, 0, "SubscriptionSourceTypeModel");
    qmlRegisterType<TransferPriorityModel>("cuteNews", 1, 0, "TransferPriorityModel");
    qmlRegisterType<UpdateIntervalTypeModel>("cuteNews", 1, 0, "UpdateIntervalTypeModel");
    
    qmlRegisterUncreatableType<FeedPluginConfig>("cuteNews", 1, 0, "FeedPluginConfig", "");
    qmlRegisterUncreatableType<PluginManager>("cuteNews", 1, 0, "PluginManager", "");
    qmlRegisterUncreatableType<ServerSettings>("cuteNews", 1, 0, "ServerSettings", "");
    qmlRegisterUncreatableType<Subscriptions>("cuteNews", 1, 0, "Subscriptions", "");
    qmlRegisterUncreatableType<Transfer>("cuteNews", 1, 0, "Transfer", "");
    qmlRegisterUncreatableType<TransferModel>("cuteNews", 1, 0, "TransferModel", "");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cutenews-client");
    app.setApplicationName("cutenews-client");
    app.setApplicationVersion(VERSION_NUMBER);
    
    const QStringList args = app.arguments();
    const int verbosity = args.indexOf("-v") + 1;
    
    if ((verbosity > 1) && (verbosity < args.size())) {
        Logger::setVerbosity(qMax(1, args.at(verbosity).toInt()));
    }
    else {
        Logger::setFileName(Settings::loggerFileName());
        Logger::setVerbosity(Settings::loggerVerbosity());
    }
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
        
    QScopedPointer<DBNotify> notify(DBNotify::instance());
    QScopedPointer<PluginManager> plugins(PluginManager::instance());
    QScopedPointer<ServerSettings> serversettings(ServerSettings::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlOpenerModel> opener(UrlOpenerModel::instance());
    
    DBConnection connection;
    Logger logger;
    Utils utils;
    
    registerTypes();
    
    QDeclarativeEngine engine;    
    QDeclarativeContext *context = engine.rootContext();
    context->setContextProperty("database", &connection);
    context->setContextProperty("logger", &logger);
    context->setContextProperty("notifier", notify.data());
    context->setContextProperty("plugins", plugins.data());
    context->setContextProperty("serversettings", serversettings.data());
    context->setContextProperty("settings", settings.data());
    context->setContextProperty("subscriptions", subscriptions.data());
    context->setContextProperty("transfers", transfers.data());
    context->setContextProperty("urlopener", opener.data());
    context->setContextProperty("utils", &utils);
    context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
    context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
    
    QObject::connect(&engine, SIGNAL(warnings(QList<QDeclarativeError>)), &logger, SLOT(log(QList<QDeclarativeError>)));
    QObject::connect(settings.data(), SIGNAL(loggerFileNameChanged(QString)), &logger, SLOT(setFileName(QString)));
    QObject::connect(settings.data(), SIGNAL(loggerVerbosityChanged(int)), &logger, SLOT(setVerbosity(int)));
    
    QDeclarativeComponent component(&engine, "/opt/cutenews-client/qml/MainWindow.qml");
    component.create();
    
    if (component.isError()) {
        Logger::log(component.errors());
    }
    
    return app.exec();
}
