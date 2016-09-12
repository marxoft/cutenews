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

#ifndef FEEDREQUEST_H
#define FEEDREQUEST_H

#include <QObject>
#include <QVariantMap>

class FeedRequest : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(QByteArray result READ result NOTIFY finished)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Canceled,
        Ready,
        Error
    };
    
    explicit FeedRequest(QObject *parent = 0) : QObject(parent) {}
    
    virtual QString errorString() const = 0;
    
    virtual QByteArray result() const = 0;
    
    virtual Status status() const = 0;

public Q_SLOTS:
    virtual bool cancel() { return false; }
    virtual bool getFeed(const QVariantMap &settings) = 0;

Q_SIGNALS:
    void finished(FeedRequest *req);
    void statusChanged(FeedRequest::Status s);
};

#endif // FEEDREQUEST_H
