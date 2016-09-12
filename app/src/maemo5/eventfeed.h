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

#ifndef EVENTFEED_H
#define EVENTFEED_H

#include <QObject>
#include <QDBusPendingCallWatcher>
#include <QStringList>
#include <QVariantMap>

class DBConnection;
class QDBusPendingCallWatcher;

class EventFeed : public QObject
{
    Q_OBJECT
    
public:
    ~EventFeed();
    
    static EventFeed* instance();    
    
private Q_SLOTS:    
    void postArticlesToFeed(DBConnection *connection);
    void postNextArticle();
    void onArticlePosted(QDBusPendingCallWatcher *watcher);
    
    void onArticlesAdded(const QStringList &articleIds, const QString &subscriptionId);
        
    void onSubscriptionDeleted(const QString &subscriptionId);
    
private:
    EventFeed();
    
    static QDBusPendingCall addItemToEventFeed(const QVariantMap &item);
    static QDBusPendingCall removeItemsFromEventFeed(const QString &subscriptionId);
    
    static EventFeed *self;
    
    static const QString EVENT_FEED_SERVICE;
    static const QString EVENT_FEED_PATH;
    static const QString EVENT_FEED_INTERFACE;
    static const QString EVENT_ACTION;
    
    QList<QVariantMap> m_items;
};

#endif // EVENTFEED_H
