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
#include "definitions.h"
#include "subscriptions.h"
#include "utils.h"
#include <QDataStream>
#include <QDateTime>
#include <QDBusInterface>
#include <QFile>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

static const QString FILE_NAME("/home/user/.config/cuteNews/eventfeedsubscriptions");
static const int REQUEST_ID = Utils::createId();

EventFeed* EventFeed::self = 0;

QString base64SerializedVariant(const QVariant &value) {
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << value;
    return ba.toBase64();
}

EventFeed::EventFeed() :
    QObject(),
    m_dbusIf(0)
{   
    if (!publishedSubscriptions().isEmpty()) {
        connectSignals();
    }
}

EventFeed::~EventFeed() {
    self = 0;
}

EventFeed* EventFeed::instance() {
    return self ? self : self = new EventFeed;
}

void EventFeed::connectSignals() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "EventFeed::connectSignals";
#endif
    connect(Database::instance(), SIGNAL(articlesAdded(int, int)),
            this, SLOT(onArticlesAdded(int, int)), Qt::UniqueConnection);
    connect(Database::instance(), SIGNAL(subscriptionsAdded(int)),
            this, SLOT(onSubscriptionsAdded(int)), Qt::UniqueConnection);
    connect(Database::instance(), SIGNAL(subscriptionDeleted(int)),
            this, SLOT(onSubscriptionDeleted(int)), Qt::UniqueConnection);
}

void EventFeed::disconnectSignals() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "EventFeed::disconnectSignals";
#endif
    disconnect(Database::instance(), SIGNAL(articlesAdded(int, int)), this, SLOT(onArticlesAdded(int, int)));
    disconnect(Database::instance(), SIGNAL(subscriptionsAdded(int)), this, SLOT(onSubscriptionsAdded(int)));
    disconnect(Database::instance(), SIGNAL(subscriptionDeleted(int)), this, SLOT(onSubscriptionDeleted(int)));
}

QVariantList EventFeed::publishedSubscriptions() const {
    QFile file(FILE_NAME);
    QVariantList published;
    
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            published << file.readLine().trimmed().toInt();
        }
        
        file.close();
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "EventFeed::publishedSubscriptions" << published;
#endif
    return published;
}

bool EventFeed::subscriptionIsPublished(int subscriptionId) const {
    return publishedSubscriptions().contains(subscriptionId);
}

void EventFeed::addPublishedSubscription(int subscriptionId) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "EventFeed::addPublishedSubscription" << subscriptionId;
#endif
    QFile file(FILE_NAME);
    
    if (file.open(QFile::Append | QFile::Text)) {
        file.write(QByteArray::number(subscriptionId) + "\n");
        file.close();
        emit publishedSubscriptionsChanged();
        connectSignals();
    }
}

void EventFeed::removePublishedSubscription(int subscriptionId) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "EventFeed::removePublishedSubscription" << subscriptionId;
#endif
    QVariantList published = publishedSubscriptions();
    
    if (published.contains(subscriptionId)) {
        published.removeAll(subscriptionId);
        
        QFile file(FILE_NAME);
        
        if (published.isEmpty()) {
            file.remove();
            emit publishedSubscriptionsChanged();
            disconnectSignals();
        }
        else if (file.open(QFile::WriteOnly | QFile::Text)) {
            foreach (QVariant v, published) {
                file.write(v.toByteArray() + "\n");
            }
            
            file.close();
            emit publishedSubscriptionsChanged();
        }
    }
}

void EventFeed::setSubscriptionPublished(int subscriptionId, bool isPublished) {
    if (isPublished) {
        addPublishedSubscription(subscriptionId);
    }
    else {
        removePublishedSubscription(subscriptionId);
    }
}

void EventFeed::addPublishedSubscriptions(QSqlQuery query, int requestId) {
    if (requestId == REQUEST_ID) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::addPublishedSubscriptions. Adding published subscription ids";
#endif
        disconnect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                   this, SLOT(addPublishedSubscriptions(QSqlQuery, int)));
        
        QVariantList published = publishedSubscriptions();
        published.removeAll(-1);
        
        while (query.next()) {
            published << query.value(0);
        }
        
        QFile file(FILE_NAME);
        
        if (published.isEmpty()) {
            file.remove();
            emit publishedSubscriptionsChanged();
            disconnectSignals();
        }
        else if (file.open(QFile::WriteOnly | QFile::Text)) {
            foreach (QVariant v, published) {
                file.write(v.toByteArray() + "\n");
            }
            
            file.close();
            emit publishedSubscriptionsChanged();
        }
    }
}

void EventFeed::postArticlesToFeed(QSqlQuery query, int requestId) {
    if (requestId == REQUEST_ID) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::postArticlesToFeed. Posting articles";
#endif
        disconnect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                   this, SLOT(postArticlesToFeed(QSqlQuery, int)));
        
        if (!m_dbusIf) {
            m_dbusIf = new QDBusInterface("com.nokia.home.EventFeed", "/eventfeed", "com.nokia.home.EventFeed",
                                          QDBusConnection::sessionBus(), this);
        }
        
        while (query.next()) {
            QVariantList args;
            QVariantMap itemArgs;
            itemArgs.insert("action",
                            QString("org.marxoft.cutenews /org/marxoft/cutenews org.marxoft.cutenews showArticle %1")
                            .arg(base64SerializedVariant(query.value(0).toInt())));
            itemArgs.insert("body", query.value(1).toString().remove(QRegExp("<[^>]*>")));
            itemArgs.insert("icon", QString("/usr/share/icons/hicolor/64x64/apps/cutenews.png"));
            itemArgs.insert("sourceDisplayName", query.value(5).toString());
            itemArgs.insert("sourceName", QString("cutenews%1").arg(query.value(2).toInt()));
            itemArgs.insert("timestamp", query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            itemArgs.insert("title", query.value(4).toString());
            args.append(itemArgs);
            
            m_dbusIf->callWithArgumentList(QDBus::NoBlock, "addItem", args);
        }
    }
}

void EventFeed::onArticlesAdded(int count, int subscriptionId) {
    if (subscriptionIsPublished(subscriptionId)) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::onArticlesAdded. Fetching articles";
#endif
        connect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                this, SLOT(postArticlesToFeed(QSqlQuery, int)));
                                       
        Database::execQuery(QString("SELECT articles.id, articles.body, articles.subscriptionId, articles.date, \
        articles.title, subscriptions.title FROM articles LEFT JOIN subscriptions ON \
        articles.subscriptionId = subscriptions.id WHERE articles.subscriptionId = %1 \
        ORDER BY articles.id DESC LIMIT %2").arg(subscriptionId).arg(count), REQUEST_ID);
    }
}

void EventFeed::onSubscriptionsAdded(int count) {
    if (subscriptionIsPublished(-1)) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "EventFeed::onSubscriptionsAdded. Fetching new subscription ids";
#endif
        connect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                this, SLOT(addPublishedSubscriptions(QSqlQuery, int)));
        Database::execQuery(QString("SELECT id FROM subscriptions ORDER BY id DESC LIMIT %1").arg(count), REQUEST_ID);
    }
}

void EventFeed::onSubscriptionDeleted(int subscriptionId) {
    if (m_dbusIf) {
        m_dbusIf->call(QDBus::NoBlock, "removeItemsBySourceName", QString("cutenews%1").arg(subscriptionId));
    }
    
    removePublishedSubscription(subscriptionId);
}
