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

#ifndef JAVASCRIPTENCLOSUREREQUEST_H
#define JAVASCRIPTENCLOSUREREQUEST_H

#include "enclosurerequest.h"
#include "javascriptglobalobject.h"
#include <QNetworkRequest>
#include <QScriptable>

class QScriptEngine;

class JavaScriptEnclosureRequest : public EnclosureRequest
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptEnclosureRequest(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual QString errorString() const;

    virtual Enclosure result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void onRequestError(const QString &errorString);
    void onRequestFinished(const Enclosure &result);

private:
    void setErrorString(const QString &e);
    
    void setResult(const Enclosure &r);
    
    void setStatus(Status s);
    
    void initEngine();
    
    JavaScriptGlobalObject *m_global;
    QScriptEngine *m_engine;
    
    QString m_fileName;
    QString m_id;

    QString m_errorString;

    Enclosure m_result;

    Status m_status;
    
    bool m_evaluated;
};

class JavaScriptEnclosureRequestGlobalObject : public JavaScriptGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptEnclosureRequestGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void error(const QString &errorString);
    void finished(const Enclosure &result);

private:
    static QScriptValue newEnclosure(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newNetworkRequest(QScriptContext *context, QScriptEngine *engine);
};

class JavaScriptEnclosure : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(QNetworkRequest request READ request WRITE setRequest)
    Q_PROPERTY(QString operation READ operation WRITE setOperation)
    Q_PROPERTY(QString data READ data WRITE setData)

public:
    explicit JavaScriptEnclosure(QObject *parent = 0);
    
    QString fileName() const;
    void setFileName(const QString &f);
    
    QNetworkRequest request() const;
    void setRequest(const QNetworkRequest &r);
    
    QString operation() const;
    void setOperation(const QString &o);
    
    QString data() const;
    void setData(const QString &d);
};

class JavaScriptNetworkRequest : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QVariantMap headers READ headers WRITE setHeaders)
    
public:
    explicit JavaScriptNetworkRequest(QObject *parent = 0);
    
    QString url() const;
    void setUrl(const QString &u);
    
    QVariantMap headers() const;
    void setHeaders(const QVariantMap &h);

public Q_SLOTS:
    QVariant header(const QString &name) const;
    void setHeader(const QString &name, const QVariant &value);
};

Q_DECLARE_METATYPE(Enclosure*)
Q_DECLARE_METATYPE(QNetworkRequest*)

#endif // JAVASCRIPTENCLOSUREREQUEST_H
