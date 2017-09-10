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

#include "javascriptenclosurerequest.h"
#include "logger.h"
#include <QFile>
#include <QScriptEngine>

JavaScriptEnclosureRequest::JavaScriptEnclosureRequest(const QString &id, const QString &fileName, QObject *parent) :
    EnclosureRequest(parent),
    m_global(0),
    m_engine(0),
    m_fileName(fileName),
    m_id(id),
    m_evaluated(false)
{
}

QString JavaScriptEnclosureRequest::fileName() const {
    return m_fileName;
}

QString JavaScriptEnclosureRequest::id() const {
    return m_id;
}

QString JavaScriptEnclosureRequest::errorString() const {
    return m_errorString;
}

void JavaScriptEnclosureRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

EnclosureResult JavaScriptEnclosureRequest::result() const {
    return m_result;
}

void JavaScriptEnclosureRequest::setResult(const EnclosureResult &r) {
    m_result = r;
}

EnclosureRequest::Status JavaScriptEnclosureRequest::status() const {
    return m_status;
}

void JavaScriptEnclosureRequest::setStatus(EnclosureRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

void JavaScriptEnclosureRequest::initEngine() {
    if (m_evaluated) {
        return;
    }
    
    if (!m_engine) {
        m_engine = new QScriptEngine(this);
    }

    QFile file(fileName());
    
    if (file.open(QFile::ReadOnly)) {
        const QScriptValue result = m_engine->evaluate(file.readAll(), fileName());
        file.close();
        
        if (result.isError()) {
            Logger::log("JavaScriptEnclosureRequest::initEngine(): Error evaluating JavaScript file: "
                        + result.toString());
            return;
        }
        
        Logger::log("JavaScriptEnclosureRequest::initEngine(): JavaScript file evaluated OK", Logger::MediumVerbosity);
        m_evaluated = true;
        m_global = new JavaScriptEnclosureRequestGlobalObject(m_engine);
        
        connect(m_global, SIGNAL(error(QString)), this, SLOT(onRequestError(QString)));
        connect(m_global, SIGNAL(finished(EnclosureResult)), this, SLOT(onRequestFinished(EnclosureResult)));
        
        m_engine->installTranslatorFunctions();
    }
    else {
        Logger::log("JavaScriptEnclosureRequest::initEngine(): Error reading JavaScript file: "
                    + file.errorString());
    }
}

bool JavaScriptEnclosureRequest::cancel() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancel").call(QScriptValue()).toBool();
}

bool JavaScriptEnclosureRequest::getEnclosure(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }
    
    initEngine();
    QScriptValue func = m_engine->globalObject().property("getEnclosure");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << url
                                                                                 << m_engine->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptEnclosureRequest::getEnclosure(). Error calling getEnclosure(): " + errorString);
            setErrorString(errorString);
            setResult(EnclosureResult());
            setStatus(Error);
            emit finished(this);
            return false;
        }

        if (result.toBool()) {
            setErrorString(QString());
            setStatus(Active);
            return true;
        }
    }
    else {
        Logger::log("JavaScriptEnclosureRequest::getEnclosure(). getEnclosure() function not defined");
        setErrorString(tr("getEnclosure() function not defined"));
        setResult(EnclosureResult());
        setStatus(Error);
        emit finished(this);
    }

    return false;
}

void JavaScriptEnclosureRequest::onRequestError(const QString &errorString) {
    Logger::log("JavaScriptEnclosureRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(EnclosureResult());
    setStatus(Error);
    emit finished(this);
}

void JavaScriptEnclosureRequest::onRequestFinished(const EnclosureResult &result) {
    Logger::log("JavaScriptEnclosureRequest::onRequestFinished()", Logger::MediumVerbosity);
    setResult(result);
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

JavaScriptEnclosureRequestGlobalObject::JavaScriptEnclosureRequestGlobalObject(QScriptEngine *engine) :
    JavaScriptGlobalObject(engine)
{
    QScriptValue enclosure = engine->newQObject(new JavaScriptEnclosureResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<EnclosureResult>(), enclosure);
    engine->setDefaultPrototype(qMetaTypeId<EnclosureResult*>(), enclosure);
    engine->globalObject().setProperty("EnclosureResult", engine->newFunction(newEnclosureResult));
    QScriptValue request = engine->newQObject(new JavaScriptNetworkRequest(engine));
    engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest>(), request);
    engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), request);
    engine->globalObject().setProperty("NetworkRequest", engine->newFunction(newNetworkRequest));
}

QScriptValue JavaScriptEnclosureRequestGlobalObject::newEnclosureResult(QScriptContext *context,
        QScriptEngine *engine) {
    switch (context->argumentCount()) {
    case 0:
        return engine->toScriptValue(EnclosureResult());
    case 2:
        return engine->toScriptValue(EnclosureResult(context->argument(0).toString(),
                    qscriptvalue_cast<QNetworkRequest>(context->argument(1))));
    case 3:
        return engine->toScriptValue(EnclosureResult(context->argument(0).toString(),
                    qscriptvalue_cast<QNetworkRequest>(context->argument(1)),
                    context->argument(2).toString().toUtf8()));
    case 4:
        return engine->toScriptValue(EnclosureResult(context->argument(0).toString(),
                    qscriptvalue_cast<QNetworkRequest>(context->argument(1)),
                    context->argument(2).toString().toUtf8(), context->argument(3).toString().toUtf8()));
    default:
        return context->throwError(QScriptContext::SyntaxError,
                                   QObject::tr("EnclosureResult constructor requires either 0, 2, 3 or 4 arguments."));
    }
}

QScriptValue JavaScriptEnclosureRequestGlobalObject::newNetworkRequest(QScriptContext *context, QScriptEngine *engine) {
    switch (context->argumentCount()) {
    case 0:
        return engine->toScriptValue(QNetworkRequest());
    case 1:
        return engine->toScriptValue(QNetworkRequest(context->argument(0).toString()));
    default:
        return context->throwError(QScriptContext::SyntaxError,
                                   QObject::tr("NetworkRequest constructor requires either 0 or 1 arguments."));
    }
}

JavaScriptEnclosureResult::JavaScriptEnclosureResult(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptEnclosureResult::fileName() const {
    if (const EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        return enclosure->fileName;
    }
    
    return QString();
}

void JavaScriptEnclosureResult::setFileName(const QString &f) {
    if (EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        enclosure->fileName = f;
    }
}

QNetworkRequest JavaScriptEnclosureResult::request() const {
    if (const EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        return enclosure->request;
    }
    
    return QNetworkRequest();
}

void JavaScriptEnclosureResult::setRequest(const QNetworkRequest &r) {
    if (EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        enclosure->request = r;
    }
}

QString JavaScriptEnclosureResult::operation() const {
    if (const EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        return QString::fromUtf8(enclosure->operation);
    }
    
    return QString();
}

void JavaScriptEnclosureResult::setOperation(const QString &o) {
    if (EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        enclosure->operation = o.toUtf8();
    }
}

QString JavaScriptEnclosureResult::data() const {
    if (const EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        return QString::fromUtf8(enclosure->data);
    }
    
    return QString();
}

void JavaScriptEnclosureResult::setData(const QString &d) {
    if (EnclosureResult *enclosure = qscriptvalue_cast<EnclosureResult*>(thisObject())) {
        enclosure->data = d.toUtf8();
    }
}

JavaScriptNetworkRequest::JavaScriptNetworkRequest(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptNetworkRequest::url() const {
    if (const QNetworkRequest *request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        return request->url().toString();
    }
    
    return QString();
}

void JavaScriptNetworkRequest::setUrl(const QString &u) {
    if (QNetworkRequest *request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        request->setUrl(u);
    }
}

QVariantMap JavaScriptNetworkRequest::headers() const {
    if (const QNetworkRequest *request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        QVariantMap h;
        
        foreach (const QByteArray &header, request->rawHeaderList()) {
            h[QString::fromUtf8(header)] = QString::fromUtf8(request->rawHeader(header));
        }
        
        return h;
    }
    
    return QVariantMap();
}

void JavaScriptNetworkRequest::setHeaders(const QVariantMap &h) {
    if (QNetworkRequest *request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        QMapIterator<QString, QVariant> iterator(h);
        
        while (iterator.hasNext()) {
            iterator.next();
            request->setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
        }
    }
}

QVariant JavaScriptNetworkRequest::header(const QString &name) const {
    if (const QNetworkRequest *request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        return request->rawHeader(name.toUtf8());
    }
    
    return QVariant();
}

void JavaScriptNetworkRequest::setHeader(const QString &name, const QVariant &value) {
    if (QNetworkRequest *request = qscriptvalue_cast<QNetworkRequest*>(thisObject())) {
        request->setRawHeader(name.toUtf8(), value.toByteArray());
    }
}

