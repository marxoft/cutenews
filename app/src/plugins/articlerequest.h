/*!
 * \file articlerequest.h
 *
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
#include <QVariantList>

/*!
 * Contains the result of an article request.
 *
 * The ArticleResult struct contains the properties that make up the result of an article request.
 */
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

    /*!
     * The author of the article.
     */
    QString author;

    /*!
     * The body of the article.
     */
    QString body;

    /*!
     * The list of the article's categories.
     */
    QStringList categories;

    /*!
     * The date of the article.
     */
    QDateTime date;

    /*!
     * The list of the article's enclosures.
     */
    QVariantList enclosures;

    /*!
     * The title of the article.
     */
    QString title;

    /*!
     * The url of the article.
     */
    QString url;
};

/*!
 * Performs a request for an article result.
 *
 * The ArticleRequest class is used to retrieve an article download request in cases where the RSS feed 
 * article URLs do not point to an actual file. Without the ArticleRequest class, it would not be possible 
 * for the cuteNews application to support the downloading of these articles.
 */
class ArticleRequest : public QObject
{
    Q_OBJECT

    /*!
     * Describes the last error that occurred.
     * 
     * \sa status
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)

    /*!
     * The result of the article request.
     *
     * \sa ArticleResult
     */
    Q_PROPERTY(ArticleResult result READ result NOTIFY finished)

    /*!
     * The \c author property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QString resultAuthor READ resultAuthor NOTIFY finished)

    /*!
     * The \c body property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QString resultBody READ resultBody NOTIFY finished)

    /*!
     * The \c categories property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QStringList resultCategories READ resultCategories NOTIFY finished)

    /*!
     * The \c date property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QDateTime resultDate READ resultDate NOTIFY finished)

    /*!
     * The \c enclosures property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QVariantList resultEnclosures READ resultEnclosures NOTIFY finished)

    /*!
     * The \c title property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QString resultTitle READ resultTitle NOTIFY finished)

    /*!
     * The \c url property of the result.
     *
     * \sa result, ArticleResult
     */
    Q_PROPERTY(QString resultUrl READ resultUrl NOTIFY finished)

    /*!
     * The current status of the article request.
     */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    /*!
     * Defines the possible states of ArticleRequest.
     */
    enum Status {
        /*!
         * The request is idle.
         */
        Idle = 0,

        /*!
         * The request is active.
         */
        Active,

        /*!
         * The request has been canceled.
         */
        Canceled,

        /*!
         * The request has been completed successfully.
         */
        Ready,

        /*!
         * The request was unsuccessful due to error.
         */
        Error
    };

    /*!
     * Constructs an ArticleRequest with the parent set to \a parent.
     */
    explicit ArticleRequest(QObject *parent = 0) : QObject(parent) {}

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return either a description of the last error, or an empty string 
     * if no error has occurred.
     *
     * \sa status()
     */
    virtual QString errorString() const = 0;

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return the result of a successful article request.
     *
     * \sa ArticleResult
     */
    virtual ArticleResult result() const = 0;

    /*!
     * This is a convenience method that returns the \c author property of the result.
     *
     * \sa result(), ArticleResult
     */
    QString resultAuthor() const { return result().author; }

    /*!
     * This is a convenience method that returns the \c body property of the result.
     *
     * \sa result(), ArticleResult
     */
    QString resultBody() const { return result().body; }

    /*!
     * This is a convenience method that returns the \c categories property of the result.
     *
     * \sa result(), ArticleResult
     */
    QStringList resultCategories() const { return result().categories; }

    /*!
     * This is a convenience method that returns the \c date property of the result.
     *
     * \sa result(), ArticleResult
     */
    QDateTime resultDate() const { return result().date; }

    /*!
     * This is a convenience method that returns the \c enclosures property of the result.
     *
     * \sa result, ArticleResult
     */
    QVariantList resultEnclosures() const { return result().enclosures; }

    /*!
     * This is a convenience method that returns the \c title property of the result.
     *
     * \sa result(), ArticleResult
     */
    QString resultTitle() const { return result().title; }

    /*!
     * This is a convenience method that returns the \c url property of the result.
     *
     * \sa result(), ArticleResult
     */
    QString resultUrl() const { return result().url; }

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return the current status of the article request.
     */
    virtual Status status() const = 0;

public Q_SLOTS:
    /*!
     * This method should cancel the request if possible and return \c true if successful.
     *
     * By default, this method does nothing and returns \c false.
     */
    virtual bool cancel() { return false; }

    /*!
     * Pure virtual method.
     * 
     * This method must be re-implemented to fetch the article result for \a url and return \c true if successful.
     *
     * The \a settings are a QVariantMap with the keys provided by the 'articleSettings' key in the plugin's JSON declaration. 
     * The values are either those set by the user, or the default values specified in the plugin's JSON declaration.
     */
    virtual bool getArticle(const QString &url, const QVariantMap &settings) = 0;

Q_SIGNALS:
    /*!
     * This signal should be emitted when the article request is finished.
     *
     * The request is finished when its status is either Ready, Canceled or Error.
     *
     * \sa Status, status(), statusChanged()
     */
    void finished(ArticleRequest *req);

    /*!
     * This signal should be emitted when the status of the article request changes.
     *
     * \sa Status, status()
     */
    void statusChanged(ArticleRequest::Status s);
};

Q_DECLARE_METATYPE(ArticleResult)

#endif // ARTICLEREQUEST_H
