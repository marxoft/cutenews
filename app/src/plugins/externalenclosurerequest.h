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

#ifndef EXTERNALENCLOSUREREQUEST_H
#define EXTERNALENCLOSUREREQUEST_H

#include "enclosurerequest.h"
#include <QNetworkRequest>

class QProcess;

class ExternalEnclosureRequest : public EnclosureRequest
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit ExternalEnclosureRequest(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual QString errorString() const;

    virtual EnclosureResult result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void onRequestError();
    void onRequestFinished(int exitCode);

private:
    void setErrorString(const QString &e);
    
    void setResult(const EnclosureResult &r);
    
    void setStatus(Status s);
    
    QProcess* process();
    
    QProcess *m_process;
    
    QString m_fileName;
    QString m_id;

    QString m_errorString;

    EnclosureResult m_result;

    Status m_status;    
};

#endif // EXTERNALENCLOSUREREQUEST_H
