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

#include "download.h"
#include "definitions.h"
#include "logger.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

Download::Download(QObject *parent) :
    Transfer(Transfer::Download, parent),
    m_reply(0),
    m_canceled(false),
    m_redirects(0),
    m_metadataSet(false)
{
}

QByteArray Download::readAll() const {
    return m_response;
}

void Download::queue() {
    switch (status()) {
    case Canceled:
    case Completed:
    case Queued:
    case Connecting:
    case Downloading:
        return;
    default:
        break;
    }
    
    setStatus(Queued);
}

void Download::start() {
    switch (status()) {
    case Connecting:
    case Downloading:
        return;
    default:
        break;
    }
    
    startDownload(url());
}

void Download::pause() {
    switch (status()) {
    case Paused:
    case Canceled:
    case Completed:
    case Connecting:
        return;
    default:
        break;
    }
    
    if ((m_reply) && (m_reply->isRunning())) {
        m_canceled = false;
        m_reply->abort();
    }
    else {
        setStatus(Paused);
    }
}

void Download::cancel() {
    switch (status()) {
    case Canceled:
    case Completed:
        return;
    default:
        break;
    }
    
    if ((m_reply) && (m_reply->isRunning())) {
        m_canceled = true;
        m_reply->abort();
    }
    else {
        setStatus(Canceled);
    }
}

void Download::startDownload(const QString &u) {
    Logger::log("Download::startDownload(). URL: " + u, Logger::LowVerbosity);
    QNetworkRequest request(u);
    request.setRawHeader("User-Agent", USER_AGENT);
    setStatus(Downloading);
    m_redirects = 0;
    m_response.clear();
    m_reply = networkAccessManager()->get(request);
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void Download::followRedirect(const QString &u) {
    Logger::log("Download::followRedirect(). URL: " + u, Logger::LowVerbosity);
    QNetworkRequest request(u);
    request.setRawHeader("User-Agent", USER_AGENT);
    m_redirects++;
    m_response.clear();
    m_reply = networkAccessManager()->get(request);
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void Download::onReplyMetaDataChanged() {
    if ((m_metadataSet) || (m_reply->error() != QNetworkReply::NoError) ||
        (!m_reply->rawHeader("Location").isEmpty())) {
        return;
    }

    qint64 bytes = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    
    if (bytes <= 0) {
        bytes = m_reply->rawHeader("Content-Length").toLongLong();
    }
    
    if (bytes > 0) {
        setSize(bytes + bytesTransferred());
    }
    
    m_metadataSet = true;
}

void Download::onReplyReadyRead() {
    if (!m_metadataSet) {
        return;
    }

    const qint64 bytes = m_reply->bytesAvailable();

    if (bytes < DOWNLOAD_BUFFER_SIZE) {
        return;
    }

    m_response.append(m_reply->read(bytes));
    setBytesTransferred(bytesTransferred() + bytes);
    
    if (size() > 0) {
        setProgress(bytesTransferred() * 100 / size());
    }
}

void Download::onReplyFinished() {
    const QString redirect = QString::fromUtf8(m_reply->rawHeader("Location"));

    if (!redirect.isEmpty()) {
        m_reply->deleteLater();
        m_reply = 0;
        
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect);
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Failed);
        }
        
        return;
    }

    const QNetworkReply::NetworkError error = m_reply->error();
    const QString errorString = m_reply->errorString();

    if ((m_reply->isOpen()) && (error == QNetworkReply::NoError)) {
        const qint64 bytes = m_reply->bytesAvailable();
        
        if ((bytes > 0) && (m_metadataSet)) {
            m_response.append(m_reply->read(bytes));
            setBytesTransferred(bytesTransferred() + bytes);
            
            if (size() > 0) {
                setProgress(bytesTransferred() * 100 / size());
            }
        }
    }
    
    m_reply->deleteLater();
    m_reply = 0;
        
    switch (error) {
    case QNetworkReply::NoError:
        setErrorString(QString());
        setStatus(Completed);
        break;
    case QNetworkReply::OperationCanceledError:
        setErrorString(QString());
        
        if (m_canceled) {
            setStatus(Canceled);
        }
        else {
            setStatus(Paused);
        }
        
        break;
    default:
        setErrorString(errorString);
        setStatus(Failed);
        break;
    }    
}
