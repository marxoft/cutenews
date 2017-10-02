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

#include "cutenews.h"
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
#include <QDBusConnection>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <qdeclarative.h>

const QString CuteNews::WIDGET_FILENAME("/opt/cutenews-client/qml/Widget.qml");
const QString CuteNews::WINDOW_FILENAME("/opt/cutenews-client/qml/MainWindow.qml");

CuteNews* CuteNews::self = 0;

CuteNews::CuteNews() :
    QObject(),
    m_engine(0)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenewsclient");
    connection.registerObject("/org/marxoft/cutenewsclient", this, QDBusConnection::ExportScriptableSlots);
}

CuteNews::~CuteNews() {
    self = 0;
}

CuteNews* CuteNews::instance() {
    return self ? self : self = new CuteNews;
}

void CuteNews::loadData() {
    if (!Settings::serverAddress().isEmpty()) {
        Subscriptions::instance()->getStatus(Subscriptions::DefaultStatusInterval);
        ServerSettings::instance()->load();
        PluginManager::instance()->load();
    }
}

bool CuteNews::showArticle(const QString &articleId) {
    if ((!articleId.isEmpty()) && (showWindow())) {
        emit articleRequested(articleId);
        return true;
    }
    
    return false;
}

bool CuteNews::showWidget() {
    if (m_widget.isNull()) {
        if (QObject *obj = createQmlObject(WIDGET_FILENAME)) {
            m_widget = obj;
            return true;
        }
    }
    
    return false;
}

bool CuteNews::showWindow() {
    if (m_window.isNull()) {
        m_window = createQmlObject(WINDOW_FILENAME);
    }
    
    if (!m_window.isNull()) {
        QMetaObject::invokeMethod(m_window, "activate");
        return true;
    }
    
    return false;
}

void CuteNews::initEngine() {
    if (m_engine) {
        return;
    }
    
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
    
    m_engine = new QDeclarativeEngine(this);
    
    Logger *logger = new Logger(this);
    
    QDeclarativeContext *context = m_engine->rootContext();
    context->setContextProperty("cutenews", this);
    context->setContextProperty("database", new DBConnection(this));
    context->setContextProperty("logger", logger);
    context->setContextProperty("notifier", DBNotify::instance());
    context->setContextProperty("plugins", PluginManager::instance());
    context->setContextProperty("serversettings", ServerSettings::instance());
    context->setContextProperty("settings", Settings::instance());
    context->setContextProperty("subscriptions", Subscriptions::instance());
    context->setContextProperty("transfers", TransferModel::instance());
    context->setContextProperty("urlopener", UrlOpenerModel::instance());
    context->setContextProperty("utils", new Utils(this));
    context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
    context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
    
    connect(m_engine, SIGNAL(warnings(QList<QDeclarativeError>)), logger, SLOT(log(QList<QDeclarativeError>)));
    connect(Settings::instance(), SIGNAL(loggerFileNameChanged(QString)), logger, SLOT(setFileName(QString)));
    connect(Settings::instance(), SIGNAL(loggerVerbosityChanged(int)), logger, SLOT(setVerbosity(int)));
}

QObject* CuteNews::createQmlObject(const QString &fileName) {
    initEngine();
    QDeclarativeContext *context = new QDeclarativeContext(m_engine->rootContext());
    QDeclarativeComponent *component = new QDeclarativeComponent(m_engine, fileName, this);
    
    if (QObject *obj = component->create(context)) {
        context->setParent(obj);
        return obj;
    }
    
    if (component->isError()) {
        Logger::log(component->errors());
    }
    
    delete component;
    delete context;
    return 0;
}
