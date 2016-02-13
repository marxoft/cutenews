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
#include "article.h"
#include "articlemodel.h"
#include "database.h"
#include "definitions.h"
#include "networkproxytypemodel.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionmodel.h"
#include "subscriptionplugins.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "transfermodel.h"
#include "transfers.h"
#include "urlopenermodel.h"
#include "userinterfacemodel.h"
#include "utils.h"
#include "viewmodemodel.h"
#include <QSqlQuery>
#include <QDBusConnection>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <qdeclarative.h>
#include <QDebug>

static const QString WIDGET_FILENAME("/opt/cutenews/qml/Widget.qml");
static const QString WINDOW_FILENAME("/opt/cutenews/qml/MainWindow.qml");

CuteNews* CuteNews::self = 0;

CuteNews::CuteNews() :
    QObject(),
    m_engine(0)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews");
    connection.registerObject("/org/marxoft/cutenews", this, QDBusConnection::ExportScriptableSlots);
}

CuteNews::~CuteNews() {
    self = 0;
}

CuteNews* CuteNews::instance() {
    return self ? self : self = new CuteNews;
}

bool CuteNews::showArticle(int articleId) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showArticle" << articleId;
#endif
    if ((articleId) && (showWindow())) {
        emit articleRequested(articleId);
        return true;
    }
    
    return false;
}

bool CuteNews::showWidget() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showWidget";
#endif
    if (m_widget.isNull()) {
        if (QObject *obj = createQmlObject(WIDGET_FILENAME)) {
            m_widget = obj;
            return true;
        }
    }
    
    return false;
}

bool CuteNews::showWindow() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showWindow";
#endif
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
    
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
    qmlRegisterType<Article>("cuteNews", 1, 0, "Article");
    qmlRegisterType<ArticleModel>("cuteNews", 1, 0, "ArticleModel");
    qmlRegisterType<NetworkProxyTypeModel>("cuteNews", 1, 0, "NetworkProxyTypeModel");
    qmlRegisterType<SelectionModel>("cuteNews", 1, 0, "SelectionModel");
    qmlRegisterType<Subscription>("cuteNews", 1, 0, "Subscription");
    qmlRegisterType<SubscriptionModel>("cuteNews", 1, 0, "SubscriptionModel");
    qmlRegisterType<SubscriptionSourceTypeModel>("cuteNews", 1, 0, "SubscriptionSourceTypeModel");
    qmlRegisterType<Transfer>("cuteNews", 1, 0, "Transfer");
    qmlRegisterType<TransferModel>("cuteNews", 1, 0, "TransferModel");
    qmlRegisterType<UserInterfaceModel>("cuteNews", 1, 0, "UserInterfaceModel");
    qmlRegisterType<ViewModeModel>("cuteNews", 1, 0, "ViewModeModel");
    qmlRegisterUncreatableType<Subscriptions>("cuteNews", 1, 0, "Subscriptions", "");
    
    m_engine = new QDeclarativeEngine(this);
    
    QDeclarativeContext *context = m_engine->rootContext();
    context->setContextProperty("cutenews", this);
    context->setContextProperty("database", Database::instance());
    context->setContextProperty("downloads", Transfers::instance());
    context->setContextProperty("plugins", new SubscriptionPlugins(this));
    context->setContextProperty("settings", Settings::instance());
    context->setContextProperty("subscriptions", Subscriptions::instance());
    context->setContextProperty("urlopener", new UrlOpenerModel(this));
    context->setContextProperty("utils", new Utils(this));
    context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
    context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
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
        foreach (QDeclarativeError error, component->errors()) {
            qDebug() << error.toString();
        }        
    }
    
    delete component;
    delete context;
    return 0;
}
