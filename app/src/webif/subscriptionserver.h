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

#ifndef SUBSCRIPTIONSERVER_H
#define SUBSCRIPTIONSERVER_H

#include <QObject>
#include <QHash>

class DBConnection;
class QHttpRequest;
class QHttpResponse;

class SubscriptionServer : public QObject
{
    Q_OBJECT

public:
    explicit SubscriptionServer(QObject *parent = 0);
    
    bool handleRequest(QHttpRequest *request, QHttpResponse *response);

private Q_SLOTS:
    void onConnectionFinished(DBConnection *connection);
    void onSubscriptionFetched(DBConnection *connection);
    void onSubscriptionsFetched(DBConnection *connection);
    void onResponseDone();

private:
    void addResponse(DBConnection *connection, QHttpResponse *response);
    QHttpResponse* getResponse(DBConnection *connection);
    void removeResponse(QHttpResponse *response);
    
    QHash<DBConnection *, QHttpResponse*> m_responses;
};

#endif // SUBSCRIPTIONSERVER_H
