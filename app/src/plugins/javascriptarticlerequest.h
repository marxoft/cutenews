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

#ifndef JAVASCRIPTARTICLEREQUEST_H
#define JAVASCRIPTARTICLEREQUEST_H

#include "articlerequest.h"
#include "javascriptglobalobject.h"
#include <QScriptable>

class QScriptEngine;

class JavaScriptArticleRequest : public ArticleRequest
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptArticleRequest(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual QString errorString() const;

    virtual ArticleResult result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getArticle(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void onRequestError(const QString &errorString);
    void onRequestFinished(const ArticleResult &result);

private:
    void setErrorString(const QString &e);
    
    void setResult(const ArticleResult &r);
    
    void setStatus(Status s);
    
    void initEngine();
    
    JavaScriptGlobalObject *m_global;
    QScriptEngine *m_engine;
    
    QString m_fileName;
    QString m_id;

    QString m_errorString;

    ArticleResult m_result;

    Status m_status;
    
    bool m_evaluated;
};

class JavaScriptArticleRequestGlobalObject : public JavaScriptGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptArticleRequestGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void error(const QString &errorString);
    void finished(const ArticleResult &result);

private:
    static QScriptValue newArticleResult(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newNetworkRequest(QScriptContext *context, QScriptEngine *engine);
};

class JavaScriptArticleResult : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString author READ author WRITE setAuthor)
    Q_PROPERTY(QString body READ body WRITE setBody)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories)
    Q_PROPERTY(QDateTime date READ date WRITE setDate)
    Q_PROPERTY(QVariantList enclosures READ enclosures WRITE setEnclosures)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString url READ url WRITE setUrl)

public:
    explicit JavaScriptArticleResult(QObject *parent = 0);
    
    QString author() const;
    void setAuthor(const QString &a);

    QString body() const;
    void setBody(const QString &b);

    QStringList categories() const;
    void setCategories(const QStringList &c);

    QDateTime date() const;
    void setDate(const QDateTime &d);

    QVariantList enclosures() const;
    void setEnclosures(const QVariantList &e);

    QString title() const;
    void setTitle(const QString &t);

    QString url() const;
    void setUrl(const QString &u);
};

Q_DECLARE_METATYPE(ArticleResult*)

#endif // JAVASCRIPTARTICLEREQUEST_H
