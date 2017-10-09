/*!
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

class QNetworkAccessManager;
class QNetworkReply;

struct EnclosureResult
{
    EnclosureResult() :
        fileName(QString()),
        request(QNetworkRequest()),
        operation("GET"),
        data(QByteArray())
    {
    }
    
    EnclosureResult(const QString &name, const QNetworkRequest &req, const QByteArray &op = QByteArray("GET"),
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
    Q_PROPERTY(EnclosureResult result READ result NOTIFY finished)
    Q_PROPERTY(QByteArray resultData READ resultData NOTIFY finished)
    Q_PROPERTY(QString resultFileName READ resultFileName NOTIFY finished)
    Q_PROPERTY(QNetworkRequest resultNetworkRequest READ resultNetworkRequest NOTIFY finished)
    Q_PROPERTY(QByteArray resultOperation READ resultOperation NOTIFY finished)
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

    explicit EnclosureRequest(QObject *parent = 0);

    QString errorString() const;

    EnclosureResult result() const;

    QByteArray resultData() const;

    QString resultFileName() const;

    QNetworkRequest resultNetworkRequest() const;

    QByteArray resultOperation() const;

    Status status() const;

public Q_SLOTS:
    bool cancel();

    bool getEnclosure(const QString &url);

private Q_SLOTS:
    void checkEnclosure(QNetworkReply *reply);

Q_SIGNALS:
    void finished(EnclosureRequest *req);
    void statusChanged(EnclosureRequest::Status s);

private:
    void setErrorString(const QString &e);

    void setResult(const EnclosureResult &r);

    void setStatus(Status s);

    QNetworkAccessManager* networkAccessManager();

    QNetworkAccessManager *m_nam;

    QString m_errorString;

    EnclosureResult m_result;

    Status m_status;
};

Q_DECLARE_METATYPE(EnclosureResult)

#endif // ENCLOSUREREQUEST_H
