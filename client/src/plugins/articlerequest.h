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

#ifndef ARTICLEREQUEST_H
#define ARTICLEREQUEST_H

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QVariantList>

class QNetworkAccessManager;
class QNetworkReply;

struct ArticleResult
{
    ArticleResult() :
        author(QString()),
        body(QString()),
        categories(QStringList()),
        date(QDateTime()),
        enclosures(QVariantList()),
        title(QString()),
        url(QString())
    {
    }
    
    ArticleResult(const QString &a, const QString &b, const QStringList &c, const QDateTime &d, const QVariantList &e,
            const QString &t, const QString &u) :
        author(a),
        body(b),
        categories(c),
        date(d),
        enclosures(e),
        title(t),
        url(u)
    {
    }

    QString author;
    QString body;
    QStringList categories;
    QDateTime date;
    QVariantList enclosures;
    QString title;
    QString url;
};

class ArticleRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(ArticleResult result READ result NOTIFY finished)
    Q_PROPERTY(QString resultAuthor READ resultAuthor NOTIFY finished)
    Q_PROPERTY(QString resultBody READ resultBody NOTIFY finished)
    Q_PROPERTY(QStringList resultCategories READ resultCategories NOTIFY finished)
    Q_PROPERTY(QDateTime resultDate READ resultDate NOTIFY finished)
    Q_PROPERTY(QString resultDateString READ resultDateString NOTIFY finished)
    Q_PROPERTY(QVariantList resultEnclosures READ resultEnclosures NOTIFY finished)
    Q_PROPERTY(bool resultHasEnclosures READ resultHasEnclosures NOTIFY finished)
    Q_PROPERTY(QString resultTitle READ resultTitle NOTIFY finished)
    Q_PROPERTY(QString resultUrl READ resultUrl NOTIFY finished)
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

    explicit ArticleRequest(QObject *parent = 0);

    QString errorString() const;

    ArticleResult result() const;

    QString resultAuthor() const;

    QString resultBody() const;

    QStringList resultCategories() const;

    QDateTime resultDate() const;

    QString resultDateString() const;

    QVariantList resultEnclosures() const;

    bool resultHasEnclosures() const;

    QString resultTitle() const;

    QString resultUrl() const;

    Status status() const;

public Q_SLOTS:
    bool cancel();

    bool getArticle(const QString &url);

private Q_SLOTS:
    void checkArticle(QNetworkReply *reply);

Q_SIGNALS:
    void finished(ArticleRequest *req);
    void statusChanged(ArticleRequest::Status s);

private:
    void setErrorString(const QString &e);

    void setResult(const ArticleResult &r);

    void setStatus(Status s);

    QNetworkAccessManager* networkAccessManager();

    QNetworkAccessManager *m_nam;

    QString m_errorString;

    ArticleResult m_result;

    Status m_status;
};

Q_DECLARE_METATYPE(ArticleResult)

#endif // ARTICLEREQUEST_H
