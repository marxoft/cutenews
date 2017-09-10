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

#ifndef GUARDIANARTICLEREQUEST_H
#define GUARDIANARTICLEREQUEST_H

#include "articlerequest.h"
#include <qhtmlparser/qhtmlparser.h>
#include <QVariantMap>

class QNetworkAccessManager;
class QNetworkReply;

class GuardianArticleRequest : public ArticleRequest
{
    Q_OBJECT

public:
    explicit GuardianArticleRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual ArticleResult result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getArticle(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void checkArticle();

private:
    void setErrorString(const QString &e);

    void setResult(const ArticleResult &r);
    
    void setStatus(Status s);

    void followRedirect(const QString &url, const char *slot);

    static void fixRelativeUrls(QString &page, const QString &baseUrl);

    static QString getRedirect(const QNetworkReply *reply);
    
    static void remove(QString &in, const QString &s);
    static void replace(QString &in, const QString &s, const QString &r);
    static void unescape(QString &s);

    void writeArticleAuthor(const QHtmlElement &element);
    void writeArticleBody(const QHtmlElement &element);
    void writeArticleCategories(const QHtmlElement &element);
    void writeArticleDate(const QHtmlElement &element);
    void writeArticleEnclosures(const QHtmlElement &element);
    void writeArticleTitle(const QHtmlElement &element);
    void writeArticleUrl(const QString &url);
    
    QNetworkAccessManager* networkAccessManager();

    static const int MAX_REDIRECTS;

    static const QByteArray USER_AGENT;

    QNetworkAccessManager *m_nam;

    QString m_errorString;

    ArticleResult m_result;
    
    Status m_status;

    int m_redirects;
    
    QVariantMap m_settings;
};

#endif // GUARDIANARTICLEREQUEST_H
