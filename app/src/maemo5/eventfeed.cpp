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

#include "eventfeed.h"
#include "database.h"
#include "utils.h"
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>
#include <QRegExp>
#include <QSettings>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

static const QString EVENT_FEED_SERVICE("org.hildon.eventfeed");
static const QString EVENT_FEED_PATH("/org/hildon/eventfeed");
static const QString EVENT_FEED_INTERFACE("org.hildon.eventfeed");

static const QString EVENT_ACTION("dbus-send --print-reply --type=method_call --dest=org.marxoft.cutenews \
/org/marxoft/cutenews org.marxoft.cutenews.showArticle int32:%1");

static const int REQUEST_ID = Utils::createId();

EventFeed* EventFeed::self = 0;

static bool publishArticlesEnabled() {
    return QSettings("cuteNews", "EventFeed").value("publishArticles", false).toBool();
}

static QDBusPendingCall addItemToEventFeed(const QVariantMap &item) {
    QDBusMessage message = QDBusMessage::createMethodCall(EVENT_FEED_SERVICE, EVENT_FEED_PATH,
                                                          EVENT_FEED_INTERFACE, "addItem");
    message.setArguments(QVariantList() << item);
    return QDBusConnection::sessionBus().asyncCall(message);
}

static QDBusPendingCall removeItemsFromEventFeed(int subscriptionId) {
    QDBusMessage message = QDBusMessage::createMethodCall(EVENT_FEED_SERVICE, EVENT_FEED_PATH,
                                                          EVENT_FEED_INTERFACE, "removeItemsBySourceName");
    message.setArguments(QVariantList() << QString("cutenews_%1").arg(subscriptionId));
    return QDBusConnection::sessionBus().asyncCall(message);
}

EventFeed::EventFeed() :
    QObject()
{
    connect(Database::instance(), SIGNAL(articlesAdded(int, int)), this, SLOT(onArticlesAdded(int, int)));
    connect(Database::instance(), SIGNAL(subscriptionDeleted(int)), this, SLOT(onSubscriptionDeleted(int)));
}

EventFeed::~EventFeed() {
    self = 0;
}

EventFeed* EventFeed::instance() {
    return self ? self : self = new EventFeed;
}

void EventFeed::postArticlesToFeed(QSqlQuery query, int requestId) {
    if (requestId == REQUEST_ID) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::postArticlesToFeed. Posting articles";
#endif
        disconnect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                   this, SLOT(postArticlesToFeed(QSqlQuery, int)));
        
        const bool ready = m_items.isEmpty();
        
        while (query.next()) {
            QVariantMap item;
            item["action"] = EVENT_ACTION.arg(query.value(0).toInt());
            item["body"] = query.value(1).toString().remove(QRegExp("<[^>]*>"));
            item["footer"] = query.value(6);
            item["icon"] = QString("cutenews");
            item["sourceDisplayName"] = query.value(6);
            item["sourceName"] = QString("cutenews_%1").arg(query.value(2).toInt());
            item["timestamp"] = query.value(3).toDateTime().toString(Qt::ISODate);
            item["title"] = query.value(4);
            item["url"] = query.value(5);
            m_items << item;
        }
        
        if ((ready) && (!m_items.isEmpty())) {
            postNextArticle();
        }
    }
}

void EventFeed::postNextArticle() {
    if (m_items.isEmpty()) {
        return;
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "EventFeed::postNextArticle";
#endif
    QDBusPendingCall call = addItemToEventFeed(m_items.takeFirst());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onArticlePosted(QDBusPendingCallWatcher*)));
}

void EventFeed::onArticlePosted(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<qlonglong> reply = *watcher;
    
    if (!reply.isError()) {
        const qlonglong id = reply.argumentAt<0>();
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::onArticlePosted. Item id is" << id; 
#endif
        if (id == -1) {
            m_items.clear();
        }
        else {
            postNextArticle();
        }
    }
#ifdef CUTENEWS_DEBUG
    else {
        qDebug() << "EventFeed:onArticlePosted. Error:" << reply.error().message();
    }
#endif
    watcher->deleteLater();
}

void EventFeed::onArticlesAdded(int count, int subscriptionId) {
    if (publishArticlesEnabled()) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::onArticlesAdded. Fetching articles";
#endif
        connect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                this, SLOT(postArticlesToFeed(QSqlQuery, int)));
                                       
        Database::execQuery(QString("SELECT articles.id, articles.body, articles.subscriptionId, articles.date, \
        articles.title, articles.url, subscriptions.title FROM articles LEFT JOIN subscriptions ON \
        articles.subscriptionId = subscriptions.id WHERE articles.subscriptionId = %1 \
        ORDER BY articles.id DESC LIMIT %2").arg(subscriptionId).arg(count), REQUEST_ID);
    }
}

void EventFeed::onSubscriptionDeleted(int subscriptionId) {
    if (publishArticlesEnabled()) {
        removeItemsFromEventFeed(subscriptionId);
    }
}
