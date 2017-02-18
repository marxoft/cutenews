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

#ifndef JAVASCRIPTFEEDREQUEST_H
#define JAVASCRIPTFEEDREQUEST_H

#include "feedrequest.h"
#include "javascriptglobalobject.h"

class QScriptEngine;

class JavaScriptFeedRequest : public FeedRequest
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptFeedRequest(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual QString errorString() const;

    virtual QByteArray result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getFeed(const QVariantMap &settings);

private Q_SLOTS:
    void onRequestError(const QString &errorString);
    void onRequestFinished(const QString &result);

private:
    void setErrorString(const QString &e);
    
    void setResult(const QByteArray &r);
    
    void setStatus(Status s);
    
    void initEngine();
    
    JavaScriptGlobalObject *m_global;
    QScriptEngine *m_engine;
    
    QString m_fileName;
    QString m_id;

    QString m_errorString;

    QByteArray m_result;

    Status m_status;
    
    bool m_evaluated;
};

class JavaScriptFeedRequestGlobalObject : public JavaScriptGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptFeedRequestGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void error(const QString &errorString);
    void finished(const QString &result);
};

#endif // JAVASCRIPTFEEDREQUEST_H
