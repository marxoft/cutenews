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

Enclosure JavaScriptEnclosureRequest::result() const {
    return m_result;
}

void JavaScriptEnclosureRequest::setResult(const Enclosure &r) {
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
        connect(m_global, SIGNAL(finished(QVariantMap)), this, SLOT(onRequestFinished(QVariantMap)));
        
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
            setResult(Enclosure());
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
        setResult(Enclosure());
        setStatus(Error);
        emit finished(this);
    }

    return false;
}

void JavaScriptEnclosureRequest::onRequestError(const QString &errorString) {
    Logger::log("JavaScriptEnclosureRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(Enclosure());
    setStatus(Error);
    emit finished(this);
}

void JavaScriptEnclosureRequest::onRequestFinished(const QVariantMap &result) {
    Logger::log("JavaScriptEnclosureRequest::onRequestFinished()", Logger::MediumVerbosity);
    const QString fileName = result.value("fileName").toString();
    const QVariantMap request = result.value("request").toMap();
    const QUrl url = request.value("url").toString();

    if ((!fileName.isEmpty()) && (!url.isEmpty())) {
        QNetworkRequest req(url);
        
        if (request.contains("headers")) {
            QMapIterator<QString, QVariant> iterator(request.value("headers").toMap());
            
            while (iterator.hasNext()) {
                iterator.next();
                req.setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
            }
        }

        m_result.fileName = fileName;
        m_result.request = req;
        m_result.operation = result.value("operation", "GET").toByteArray();
        m_result.data = result.value("data").toByteArray();

        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(tr("Filename or URL is empty"));
        setResult(Enclosure());
        setStatus(Error);
    }
    
    emit finished(this);
}
