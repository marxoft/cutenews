/*
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

#include "javascriptarticlerequest.h"
#include "logger.h"
#include <QFile>
#include <QScriptEngine>

JavaScriptArticleRequest::JavaScriptArticleRequest(const QString &id, const QString &fileName, QObject *parent) :
    ArticleRequest(parent),
    m_global(0),
    m_engine(0),
    m_fileName(fileName),
    m_id(id),
    m_evaluated(false)
{
}

QString JavaScriptArticleRequest::fileName() const {
    return m_fileName;
}

QString JavaScriptArticleRequest::id() const {
    return m_id;
}

QString JavaScriptArticleRequest::errorString() const {
    return m_errorString;
}

void JavaScriptArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult JavaScriptArticleRequest::result() const {
    return m_result;
}

void JavaScriptArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status JavaScriptArticleRequest::status() const {
    return m_status;
}

void JavaScriptArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

void JavaScriptArticleRequest::initEngine() {
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
            Logger::log("JavaScriptArticleRequest::initEngine(): Error evaluating JavaScript file: "
                        + result.toString());
            return;
        }
        
        Logger::log("JavaScriptArticleRequest::initEngine(): JavaScript file evaluated OK", Logger::MediumVerbosity);
        m_evaluated = true;
        m_global = new JavaScriptArticleRequestGlobalObject(m_engine);
        
        connect(m_global, SIGNAL(error(QString)), this, SLOT(onRequestError(QString)));
        connect(m_global, SIGNAL(finished(ArticleResult)), this, SLOT(onRequestFinished(ArticleResult)));
        
        m_engine->installTranslatorFunctions();
    }
    else {
        Logger::log("JavaScriptArticleRequest::initEngine(): Error reading JavaScript file: "
                    + file.errorString());
    }
}

bool JavaScriptArticleRequest::cancel() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancel").call(QScriptValue()).toBool();
}

bool JavaScriptArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }
    
    initEngine();
    QScriptValue func = m_engine->globalObject().property("getArticle");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << url
                                                                                 << m_engine->toScriptValue(settings));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptArticleRequest::getArticle(). Error calling getArticle(): " + errorString);
            setErrorString(errorString);
            setResult(ArticleResult());
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
        Logger::log("JavaScriptArticleRequest::getArticle(). getArticle() function not defined");
        setErrorString(tr("getArticle() function not defined"));
        setResult(ArticleResult());
        setStatus(Error);
        emit finished(this);
    }

    return false;
}

void JavaScriptArticleRequest::onRequestError(const QString &errorString) {
    Logger::log("JavaScriptArticleRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(ArticleResult());
    setStatus(Error);
    emit finished(this);
}

void JavaScriptArticleRequest::onRequestFinished(const ArticleResult &result) {
    Logger::log("JavaScriptArticleRequest::onRequestFinished()", Logger::MediumVerbosity);
    setResult(result);
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

JavaScriptArticleRequestGlobalObject::JavaScriptArticleRequestGlobalObject(QScriptEngine *engine) :
    JavaScriptGlobalObject(engine)
{
    QScriptValue article = engine->newQObject(new JavaScriptArticleResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<ArticleResult>(), article);
    engine->setDefaultPrototype(qMetaTypeId<ArticleResult*>(), article);
    engine->globalObject().setProperty("ArticleResult", engine->newFunction(newArticleResult));
}

QScriptValue JavaScriptArticleRequestGlobalObject::newArticleResult(QScriptContext *context,
        QScriptEngine *engine) {
    switch (context->argumentCount()) {
    case 0:
        return engine->toScriptValue(ArticleResult());
    case 7:
        return engine->toScriptValue(ArticleResult(context->argument(0).toString(), context->argument(1).toString(),
                    context->argument(2).toVariant().toStringList(), context->argument(3).toDateTime(),
                    context->argument(4).toVariant().toList(), context->argument(5).toString(),
                    context->argument(6).toString()));
    default:
        return context->throwError(QScriptContext::SyntaxError,
                                   QObject::tr("ArticleResult constructor requires either 0 or 7 arguments."));
    }
}

JavaScriptArticleResult::JavaScriptArticleResult(QObject *parent) :
    QObject(parent)
{
}

QString JavaScriptArticleResult::author() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->author;
    }
    
    return QString();
}

void JavaScriptArticleResult::setAuthor(const QString &a) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->author = a;
    }
}

QString JavaScriptArticleResult::body() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->body;
    }
    
    return QString();
}

void JavaScriptArticleResult::setBody(const QString &b) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->body = b;
    }
}

QStringList JavaScriptArticleResult::categories() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->categories;
    }
    
    return QStringList();
}

void JavaScriptArticleResult::setCategories(const QStringList &c) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->categories = c;
    }
}

QDateTime JavaScriptArticleResult::date() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->date;
    }
    
    return QDateTime();
}

void JavaScriptArticleResult::setDate(const QDateTime &d) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->date = d;
    }
}

QVariantList JavaScriptArticleResult::enclosures() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->enclosures;
    }
    
    return QVariantList();
}

void JavaScriptArticleResult::setEnclosures(const QVariantList &e) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->enclosures = e;
    }
}

QString JavaScriptArticleResult::title() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->title;
    }
    
    return QString();
}

void JavaScriptArticleResult::setTitle(const QString &t) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->title = t;
    }
}

QString JavaScriptArticleResult::url() const {
    if (const ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        return article->url;
    }
    
    return QString();
}

void JavaScriptArticleResult::setUrl(const QString &u) {
    if (ArticleResult *article = qscriptvalue_cast<ArticleResult*>(thisObject())) {
        article->url = u;
    }
}
