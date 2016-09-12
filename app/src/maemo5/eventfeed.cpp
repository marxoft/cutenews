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
#include "dbconnection.h"
#include "dbnotify.h"
#include "logger.h"
#include "settings.h"
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QFile>
#include <QRegExp>

const QString EventFeed::EVENT_FEED_SERVICE("org.hildon.eventfeed");
const QString EventFeed::EVENT_FEED_PATH("/org/hildon/eventfeed");
const QString EventFeed::EVENT_FEED_INTERFACE("org.hildon.eventfeed");

const QString EventFeed::EVENT_ACTION("dbus-send --print-reply --type=method_call --dest=org.marxoft.cutenews \
/org/marxoft/cutenews org.marxoft.cutenews.showArticle string:%1");

EventFeed* EventFeed::self = 0;

EventFeed::EventFeed() :
    QObject()
{
    connect(DBNotify::instance(), SIGNAL(articlesAdded(QStringList, QString)),
            this, SLOT(onArticlesAdded(QStringList, QString)));
    connect(DBNotify::instance(), SIGNAL(subscriptionDeleted(QString)), this, SLOT(onSubscriptionDeleted(QString)));
}

EventFeed::~EventFeed() {
    self = 0;
}

EventFeed* EventFeed::instance() {
    return self ? self : self = new EventFeed;
}

void EventFeed::postArticlesToFeed(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        Logger::log("EventFeed::postArticlesToFeed(). Posting articles", Logger::MediumVerbosity);
        const bool ready = m_items.isEmpty();
        
        while (connection->nextRecord()) {
            QVariantMap item;
            item["action"] = EVENT_ACTION.arg(connection->value(0).toString());
            item["body"] = connection->value(1).toString().remove(QRegExp("<[^>]*>"));
            item["footer"] = connection->value(6);
            item["icon"] = QString("cutenews");
            item["sourceDisplayName"] = connection->value(6);
            item["sourceName"] = QString("cutenews_%1").arg(connection->value(2).toString());
            item["timestamp"] = QDateTime::fromTime_t(connection->value(3).toInt()).toString(Qt::ISODate);
            item["title"] = connection->value(4);
            item["url"] = connection->value(5);
            m_items << item;
        }
        
        if ((ready) && (!m_items.isEmpty())) {
            postNextArticle();
        }
    }
    
    connection->deleteLater();
}

void EventFeed::postNextArticle() {
    if (m_items.isEmpty()) {
        return;
    }
    
    QDBusPendingCall call = addItemToEventFeed(m_items.takeFirst());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onArticlePosted(QDBusPendingCallWatcher*)));
}

void EventFeed::onArticlePosted(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<qlonglong> reply = *watcher;
    
    if (!reply.isError()) {
        const qlonglong id = reply.argumentAt<0>();
        
        if (id == -1) {
            m_items.clear();
        }
        else {
            postNextArticle();
        }
    }
    else {
        Logger::log("EventFeed:onArticlePosted(). Error: " + reply.error().message());
    }
    
    watcher->deleteLater();
}

void EventFeed::onArticlesAdded(const QStringList &articleIds, const QString &subscriptionId) {
    if (Settings::eventFeedEnabled()) {
        DBConnection *conn = DBConnection::connection(this, SLOT(postArticlesToFeed(DBConnection*)));
        conn->exec(QString("SELECT articles.id, articles.body, articles.subscriptionId, articles.date, \
        articles.title, articles.url, subscriptions.title FROM articles LEFT JOIN subscriptions ON \
        articles.subscriptionId = subscriptions.id WHERE articles.id = '%1' AND articles.subscriptionId = '%2'")
        .arg(articleIds.join("' OR articles.id = '")).arg(subscriptionId));
    }
}

void EventFeed::onSubscriptionDeleted(const QString &id) {
    if (Settings::eventFeedEnabled()) {
        removeItemsFromEventFeed(id);
    }
}

QDBusPendingCall EventFeed::addItemToEventFeed(const QVariantMap &item) {
    QDBusMessage message = QDBusMessage::createMethodCall(EVENT_FEED_SERVICE, EVENT_FEED_PATH,
                                                          EVENT_FEED_INTERFACE, "addItem");
    message.setArguments(QVariantList() << item);
    return QDBusConnection::sessionBus().asyncCall(message);
}

QDBusPendingCall EventFeed::removeItemsFromEventFeed(const QString &subscriptionId) {
    QDBusMessage message = QDBusMessage::createMethodCall(EVENT_FEED_SERVICE, EVENT_FEED_PATH,
                                                          EVENT_FEED_INTERFACE, "removeItemsBySourceName");
    message.setArguments(QVariantList() << QString("cutenews_%1").arg(subscriptionId));
    return QDBusConnection::sessionBus().asyncCall(message);
}
