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

#include "javascriptfeedrequest.h"
#include "logger.h"
#include <QFile>
#include <QScriptEngine>

JavaScriptFeedRequest::JavaScriptFeedRequest(const QString &id, const QString &fileName, QObject *parent) :
    FeedRequest(parent),
    m_global(0),
    m_engine(0),
    m_fileName(fileName),
    m_id(id),
    m_evaluated(false)
{
}

QString JavaScriptFeedRequest::fileName() const {
    return m_fileName;
}

QString JavaScriptFeedRequest::id() const {
    return m_id;
}

QString JavaScriptFeedRequest::errorString() const {
    return m_errorString;
}

void JavaScriptFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray JavaScriptFeedRequest::result() const {
    return m_result;
}

void JavaScriptFeedRequest::setResult(const QByteArray &r) {
    m_result = r;
}

FeedRequest::Status JavaScriptFeedRequest::status() const {
    return m_status;
}

void JavaScriptFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

void JavaScriptFeedRequest::initEngine() {
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
            Logger::log("JavaScriptFeedRequest::initEngine(): Error evaluating JavaScript file: "
                        + result.toString());
            return;
        }
        
        Logger::log("JavaScriptFeedRequest::initEngine(): JavaScript file evaluated OK", Logger::MediumVerbosity);
        m_evaluated = true;
        m_global = new JavaScriptFeedRequestGlobalObject(m_engine);
        
        connect(m_global, SIGNAL(error(QString)), this, SLOT(onRequestError(QString)));
        connect(m_global, SIGNAL(finished(QString)), this, SLOT(onRequestFinished(QString)));
        
        m_engine->installTranslatorFunctions();
    }
    else {
        Logger::log("JavaScriptFeedRequest::initEngine(): Error reading JavaScript file: "
                    + file.errorString());
    }
}

bool JavaScriptFeedRequest::cancel() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancel").call(QScriptValue()).toBool();
}

bool JavaScriptFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }
    
    initEngine();
    QScriptValue func = m_engine->globalObject().property("getFeed");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << m_engine->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptFeedRequest::getFeed(). Error calling getFeed(): " + errorString);
            setErrorString(errorString);
            setResult(QByteArray());
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
        Logger::log("JavaScriptFeedRequest::getFeed(). getFeed() function not defined");
        setErrorString(tr("getFeed() function not defined"));
        setResult(QByteArray());
        setStatus(Error);
        emit finished(this);
    }

    return false;
}

void JavaScriptFeedRequest::onRequestError(const QString &errorString) {
    Logger::log("JavaScriptFeedRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(QByteArray());
    setStatus(Error);
    emit finished(this);
}

void JavaScriptFeedRequest::onRequestFinished(const QString &result) {
    Logger::log("JavaScriptFeedRequest::onRequestFinished()", Logger::MediumVerbosity);
    setErrorString(QString());
    setResult(result.toUtf8());
    setStatus(Ready);
    emit finished(this);
}

JavaScriptFeedRequestGlobalObject::JavaScriptFeedRequestGlobalObject(QScriptEngine *engine) :
    JavaScriptGlobalObject(engine)
{
}
