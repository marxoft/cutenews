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
#include <QList>
#include <QSqlQuery>
#include <QVariantMap>

class QDBusPendingCallWatcher;

class EventFeed : public QObject
{
    Q_OBJECT
    
public:
    ~EventFeed();
    
    static EventFeed* instance();    
    
private Q_SLOTS:    
    void postArticlesToFeed(QSqlQuery query, int requestId);
    void postNextArticle();
    void onArticlePosted(QDBusPendingCallWatcher *watcher);
    
    void onArticlesAdded(int count, int subscriptionId);
        
    void onSubscriptionDeleted(int subscriptionId);
    
private:
    EventFeed();
    
    static EventFeed *self;
    
    QList<QVariantMap> m_items;
};

#endif // EVENTFEED_H
