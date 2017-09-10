/*!
 * \file feedrequest.h
 *
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

#ifndef FEEDREQUEST_H
#define FEEDREQUEST_H

#include <QObject>
#include <QVariantMap>

/*!
 * Retrieves an RSS feed.
 *
 * The FeedRequest class is used for retrieving RSS feeds from arbitrary sources.
 */
class FeedRequest : public QObject
{
    Q_OBJECT

    /*!
     * Describes the last error that occurred.
     * 
     * \sa status
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)

    /*!
     * The result of the feed request.
     *
     * The QByteArray should contain a valid representation of an RSS feed if the request was successful.
     */
    Q_PROPERTY(QByteArray result READ result NOTIFY finished)

    /*!
     * The current status of the feed request.
     */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    /*!
     * Defines the possible states of FeedRequest.
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
     * Constructs a FeedRequest with the parent set to \a parent.
     */
    explicit FeedRequest(QObject *parent = 0) : QObject(parent) {}

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
     * This method must be re-implemented to return the result of a successful feed request.
     */
    virtual QByteArray result() const = 0;

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return the current status of the feed request.
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
     * This method must be re-implemented to fetch the RSS feed and return \c true if successful.
     *
     * The \a settings are a QVariantMap with the keys provided by the 'feedSettings' key in the plugin's JSON declaration. 
     * The values are either those set by the user, or the default values specified in the plugin's JSON declaration.
     */
    virtual bool getFeed(const QVariantMap &settings) = 0;

Q_SIGNALS:
    /*!
     * This signal should be emitted when the feed request is finished.
     *
     * The request is finished when its status is either Ready, Canceled or Error.
     *
     * \sa Status, status(), statusChanged()
     */
    void finished(FeedRequest *req);

    /*!
     * This signal should be emitted when the status of the feed request changes.
     *
     * \sa Status, status()
     */
    void statusChanged(FeedRequest::Status s);
};

#endif // FEEDREQUEST_H
