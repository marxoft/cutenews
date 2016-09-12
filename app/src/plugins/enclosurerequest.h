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

#ifndef ENCLOSUREREQUEST_H
#define ENCLOSUREREQUEST_H

#include <QObject>
#include <QNetworkRequest>

struct Enclosure
{
    Enclosure() :
        fileName(QString()),
        request(QNetworkRequest()),
        operation("GET"),
        data(QByteArray())
    {
    }
    
    Enclosure(const QString &name, const QNetworkRequest &req, const QByteArray &op = QByteArray("GET"),
              const QByteArray &d = QByteArray()) :
        fileName(name),
        request(req),
        operation(op),
        data(d)
    {
    }
    
    QString fileName;
    QNetworkRequest request;
    QByteArray operation;
    QByteArray data;
};

class EnclosureRequest : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(Enclosure result READ result NOTIFY finished)
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
    
    explicit EnclosureRequest(QObject *parent = 0) : QObject(parent) {}
    
    virtual QString errorString() const = 0;
    
    virtual Enclosure result() const = 0;
    
    virtual Status status() const = 0;

public Q_SLOTS:
    virtual bool cancel() { return false; }
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings) = 0;

Q_SIGNALS:
    void finished(EnclosureRequest *req);
    void statusChanged(EnclosureRequest::Status s);
};

Q_DECLARE_METATYPE(Enclosure)

#endif // ENCLOSUREREQUEST_H
