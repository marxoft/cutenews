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

#ifndef DBNOTIFY_H
#define DBNOTIFY_H

#include <QObject>
#include <QVariantMap>

class DBNotify : public QObject
{
    Q_OBJECT

public:
    ~DBNotify();
    
    static DBNotify* instance();

Q_SIGNALS:
    void subscriptionsAdded(const QVariantList &subscriptions);
    void subscriptionDeleted(const QString &id);
    void subscriptionUpdated(const QString &id, const QVariantMap &properties);
    void subscriptionRead(const QString &id, bool isRead, const QVariantMap &properties);
    void allSubscriptionsRead();
    
    void articleDeleted(const QString &id);
    void articleFavourited(const QString &id, bool isFavourite, const QVariantMap &properties);
    void articleRead(const QString &articleId, const QString &subscriptionId, bool isRead,
                     const QVariantMap &properties);
    void readArticlesDeleted(int count);
    
    void error(const QString &errorString);

private:
    DBNotify();
    
    static DBNotify *self;
    
    friend class DBConnection;
};

#endif // DBNOTIFY_H
