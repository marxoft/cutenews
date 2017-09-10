/*!
 * \file enclosurerequest.h
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

#ifndef ENCLOSUREREQUEST_H
#define ENCLOSUREREQUEST_H

#include <QObject>
#include <QNetworkRequest>

/*!
 * Contains the result of an enclosure request.
 *
 * The EnclosureResult struct contains the properties that make up the result of an enclosure request.
 */
struct EnclosureResult
{
    EnclosureResult() :
        fileName(QString()),
        request(QNetworkRequest()),
        operation("GET"),
        data(QByteArray())
    {
    }
    
    EnclosureResult(const QString &name, const QNetworkRequest &req, const QByteArray &op = QByteArray("GET"),
              const QByteArray &d = QByteArray()) :
        fileName(name),
        request(req),
        operation(op),
        data(d)
    {
    }

    /*!
     * The filename to be used when downloading the enclosure.
     */
    QString fileName;

    /*!
     * The network request to be used when downloading the enclosure.
     */
    QNetworkRequest request;

    /*!
     * The operation verb to be used when downloading the enclosure.
     * 
     * The default value is QByteArray("GET").
     */
    QByteArray operation;

    /*!
     * The data to be sent to the server when downloading the enclosure.
     */
    QByteArray data;
};

/*!
 * Performs a request for an enclosure result.
 *
 * The EnclosureRequest class is used to retrieve an enclosure download request in cases where the RSS feed 
 * enclosure URLs do not point to an actual file. Without the EnclosureRequest class, it would not be possible 
 * for the cuteNews application to support the downloading of these enclosures.
 */
class EnclosureRequest : public QObject
{
    Q_OBJECT

    /*!
     * Describes the last error that occurred.
     * 
     * \sa status
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)

    /*!
     * The result of the enclosure request.
     *
     * \sa EnclosureResult
     */
    Q_PROPERTY(EnclosureResult result READ result NOTIFY finished)

    /*!
     * The \c data property of the result.
     *
     * \sa result, EnclosureResult
     */
    Q_PROPERTY(QByteArray resultData READ resultData NOTIFY finished)

    /*!
     * The \c fileName property of the result.
     *
     * \sa result, EnclosureResult
     */
    Q_PROPERTY(QString resultFileName READ resultFileName NOTIFY finished)

    /*!
     * The \c request property of the result.
     *
     * \sa result, EnclosureResult
     */
    Q_PROPERTY(QNetworkRequest resultNetworkRequest READ resultNetworkRequest NOTIFY finished)

    /*!
     * The \c operation property of the result.
     *
     * \sa result, EnclosureResult
     */
    Q_PROPERTY(QByteArray resultOperation READ resultOperation NOTIFY finished)

    /*!
     * The current status of the enclosure request.
     */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)

public:
    /*!
     * Defines the possible states of EnclosureRequest.
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
     * Constructs an EnclosureRequest with the parent set to \a parent.
     */
    explicit EnclosureRequest(QObject *parent = 0) : QObject(parent) {}

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
     * This method must be re-implemented to return the result of a successful enclosure request.
     *
     * \sa EnclosureResult
     */
    virtual EnclosureResult result() const = 0;

    /*!
     * This is a convenience method that returns the \c data property of the result.
     *
     * \sa result(), EnclosureResult
     */
    QByteArray resultData() const { return result().data; }

    /*!
     * This is a convenience method that returns the \c fileName property of the result.
     *
     * \sa result(), EnclosureResult
     */
    QString resultFileName() const { return result().fileName; }

    /*!
     * This is a convenience method that returns the \c request property of the result.
     *
     * \sa result(), EnclosureResult
     */
    QNetworkRequest resultNetworkRequest() const { return result().request; }

    /*!
     * This is a convenience method that returns the \c operation property of the result.
     *
     * \sa result(), EnclosureResult
     */
    QByteArray resultOperation() const { return result().operation; }

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return the current status of the enclosure request.
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
     * This method must be re-implemented to fetch the enclosure result for \a url and return \c true if successful.
     *
     * The \a settings are a QVariantMap with the keys provided by the 'enclosureSettings' key in the plugin's JSON declaration. 
     * The values are either those set by the user, or the default values specified in the plugin's JSON declaration.
     */
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings) = 0;

Q_SIGNALS:
    /*!
     * This signal should be emitted when the enclosure request is finished.
     *
     * The request is finished when its status is either Ready, Canceled or Error.
     *
     * \sa Status, status(), statusChanged()
     */
    void finished(EnclosureRequest *req);

    /*!
     * This signal should be emitted when the status of the enclosure request changes.
     *
     * \sa Status, status()
     */
    void statusChanged(EnclosureRequest::Status s);
};

Q_DECLARE_METATYPE(EnclosureResult)

#endif // ENCLOSUREREQUEST_H
