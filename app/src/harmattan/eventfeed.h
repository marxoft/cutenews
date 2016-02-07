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
#include <QSqlQuery>
#include <QVariantList>

class QDBusInterface;

class EventFeed : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QVariantList publishedSubscriptions READ publishedSubscriptions NOTIFY publishedSubscriptionsChanged)

public:
    ~EventFeed();
    
    static EventFeed* instance();
    
    QVariantList publishedSubscriptions() const;
    
    Q_INVOKABLE bool subscriptionIsPublished(int subscriptionId) const;
    
public Q_SLOTS:
    void addPublishedSubscription(int subscriptionId);
    void removePublishedSubscription(int subscriptionId);
    void setSubscriptionPublished(int subscriptionId, bool isPublished);
    
private Q_SLOTS:
    void addPublishedSubscriptions(QSqlQuery, int requestId);
    
    void postArticlesToFeed(QSqlQuery query, int requestId);
    
    void onArticlesAdded(int count, int subscriptionId);
    
    void onSubscriptionsAdded(int count);
    void onSubscriptionDeleted(int subscriptionId);

Q_SIGNALS:
    void publishedSubscriptionsChanged();
    
private:
    EventFeed();
    
    void connectSignals();
    void disconnectSignals();
    
    static EventFeed *self;
    
    QDBusInterface *m_dbusIf;
};

#endif // EVENTFEED_H
