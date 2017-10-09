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

#include "enclosurerequest.h"
#include "json.h"
#include "logger.h"
#include "requests.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

EnclosureRequest::EnclosureRequest(QObject *parent) :
    QObject(parent),
    m_nam(0),
    m_status(Idle)
{
}

QString EnclosureRequest::errorString() const {
    return m_errorString;
}

void EnclosureRequest::setErrorString(const QString &e) {
    m_errorString = e;

    if (!e.isEmpty()) {
        Logger::log("EnclosureRequest::error: " + e);
    }
}

EnclosureResult EnclosureRequest::result() const {
    return m_result;
}

void EnclosureRequest::setResult(const EnclosureResult &r) {
    m_result = r;
}

QByteArray EnclosureRequest::resultData() const {
    return m_result.data;
}

QString EnclosureRequest::resultFileName() const {
    return m_result.fileName;
}

QNetworkRequest EnclosureRequest::resultNetworkRequest() const {
    return m_result.request;
}

QByteArray EnclosureRequest::resultOperation() const {
    return m_result.operation;
}

EnclosureRequest::Status EnclosureRequest::status() const {
    return m_status;
}

void EnclosureRequest::setStatus(EnclosureRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool EnclosureRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool EnclosureRequest::getEnclosure(const QString &url) {
    Logger::log("EnclosureRequest::getEnclosure(). URL: " + url, Logger::MediumVerbosity);

    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(EnclosureResult());
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/enclosures/" + url.toUtf8().toBase64()));
    connect(this, SIGNAL(finished(EnclosureRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void EnclosureRequest::checkEnclosure(QNetworkReply *reply) {
    switch (reply->error()) {
        case QNetworkReply::NoError:
            break;
        case QNetworkReply::OperationCanceledError:
            setStatus(Canceled);
            emit finished(this);
            return;
        default:
            setErrorString(reply->errorString());
            setStatus(Error);
            emit finished(this);
            return;
    }

    const QVariantMap result = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toMap();

    if (result.isEmpty()) {
        setErrorString(tr("Enclosure result is empty"));
        setStatus(Error);
    }
    else {
        m_result.data = result.value("data").toByteArray();
        m_result.fileName = result.value("fileName").toString();
        m_result.operation = result.value("operation").toByteArray();
        const QVariantMap request = result.value("request").toMap();
        const QVariantMap headers = request.value("headers").toMap();
        m_result.request.setUrl(request.value("url").toString());

        if (!headers.isEmpty()) {
            QMapIterator<QString, QVariant> iterator(headers);

            while (iterator.hasNext()) {
                iterator.next();
                m_result.request.setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
            }
        }

        setStatus(Ready);
    }

    emit finished(this);
}

QNetworkAccessManager* EnclosureRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkEnclosure(QNetworkReply*)));
    }

    return m_nam;
}
