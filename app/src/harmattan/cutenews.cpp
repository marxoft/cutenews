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
#include "shareui.h"
#include "subscriptionplugins.h"
#include "subscriptions.h"
#include "transfers.h"
#include "urlopenermodel.h"
#include "utils.h"
#include <QDBusConnection>
#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <QGLWidget>
#include <QStringList>
#include <qdeclarative.h>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

CuteNews::CuteNews(QObject *parent) :
    QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews");
    connection.registerObject("/org/marxoft/cutenews", this, QDBusConnection::ExportScriptableSlots);
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

bool CuteNews::showArticle(const QStringList &articleId) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showArticle" << articleId;
#endif
    return articleId.isEmpty() ? false : showArticle(articleId.first().toInt());
}

bool CuteNews::showWindow() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showWindow";
#endif
    if (!m_window) {
        m_window = new QDeclarativeView;
        ShareUi *shareui = new ShareUi(m_window);
        SubscriptionPlugins *plugins = new SubscriptionPlugins(m_window);
        UrlOpenerModel *urlopener = new UrlOpenerModel(m_window);
        Utils *utils = new Utils(m_window);
        
        QDeclarativeContext *context = m_window->rootContext();
        context->setContextProperty("cutenews", this);
        context->setContextProperty("database", Database::instance());
        context->setContextProperty("downloads", Transfers::instance());
        context->setContextProperty("eventfeed", EventFeed::instance());
        context->setContextProperty("plugins", plugins);
        context->setContextProperty("settings", Settings::instance());
        context->setContextProperty("shareui", shareui);
        context->setContextProperty("subscriptions", Subscriptions::instance());
        context->setContextProperty("urlopener", urlopener);
        context->setContextProperty("utils", utils);
        context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
        context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
        context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
        
        m_window->setViewport(new QGLWidget);
        m_window->setSource(QUrl::fromLocalFile("/opt/cutenews/qml/main.qml"));
        m_window->showFullScreen();
    }
    else {    
        m_window->activateWindow();
    }
    
    return true;
}
