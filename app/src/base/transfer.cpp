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

#include "transfer.h"
#include "logger.h"
#include "utils.h"
#include <QNetworkAccessManager>

Transfer::Transfer(Transfer::TransferType transferType, QObject *parent) :
    QObject(parent),
    m_nam(0),
    m_ownNetworkAccessManager(false),
    m_priority(NormalPriority),
    m_progress(0),
    m_size(0),
    m_bytesTransferred(0),
    m_status(Paused),
    m_transferType(transferType)
{
}

QNetworkAccessManager* Transfer::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownNetworkAccessManager = true;
    }
    
    return m_nam;
}

void Transfer::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if ((m_nam) && (m_ownNetworkAccessManager)) {
        delete m_nam;
    }
    
    m_nam = manager;
    m_ownNetworkAccessManager = false;
}

qint64 Transfer::bytesTransferred() const {
    return m_bytesTransferred;
}

void Transfer::setBytesTransferred(qint64 b) {
    if (b != bytesTransferred()) {
        m_bytesTransferred = b;
        emit bytesTransferredChanged();
    }
}

QString Transfer::errorString() const {
    return m_errorString;
}

void Transfer::setErrorString(const QString &es) {
    m_errorString = es;
}

QString Transfer::id() const {
    return m_id;
}

void Transfer::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
    }
}

QString Transfer::name() const {
    return m_name;
}

void Transfer::setName(const QString &n) {
    if (n != name()) {
        m_name = n;
        emit nameChanged();
    }
}

Transfer::Priority Transfer::priority() const {
    return m_priority;
}

void Transfer::setPriority(Priority p) {
    if (p != priority()) {
        m_priority = p;
        emit priorityChanged();
    }
}

QString Transfer::priorityString() const {
    switch (priority()) {
    case HighPriority:
        return tr("High");
    case NormalPriority:
        return tr("Normal");
    case LowPriority:
        return tr("Low");
    default:
        return QString();
    }
}

int Transfer::progress() const {
    return m_progress;
}

void Transfer::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged();
    }
}

QString Transfer::progressString() const {
    return tr("%1 of %2 (%3%)").arg(Utils::formatBytes(bytesTransferred())).arg(Utils::formatBytes(size()))
                              .arg(progress());
}

qint64 Transfer::size() const {
    return m_size;
}

void Transfer::setSize(qint64 s) {
    if (s != size()) {
        m_size = s;
        emit sizeChanged();
        
        if ((m_size > 0) && (m_bytesTransferred > 0)) {
            setProgress(m_bytesTransferred * 100 / m_size);
        }
    }
}

Transfer::Status Transfer::status() const {
    return m_status;
}

void Transfer::setStatus(Status s) {
    if (s != status()) {
        m_status = s;
        Logger::log(QString("Transfer::setStatus(). ID: %1, Status: %2").arg(id()).arg(statusString()),
                    Logger::LowVerbosity);
        emit statusChanged();
        
        switch (s) {
        case Paused:
        case Canceled:
        case Failed:
        case Completed:
            emit finished();
            break;
        default:
            break;
        }
    }
}

QString Transfer::statusString() const {
    switch (status()) {
    case Paused:
        return tr("Paused");
    case Canceled:
        return tr("Canceled");
    case Failed:
        return tr("Failed: %1").arg(errorString());
    case Completed:
        return tr("Completed");
    case Queued:
        return tr("Queued");
    case Connecting:
        return tr("Connecting");
    case Downloading:
        return tr("Downloading");
    case Uploading:
        return tr("Uploading");
    case ExecutingCustomCommand:
        return tr("Executing custom command");
    default:
        return QString();
    }
}

Transfer::TransferType Transfer::transferType() const {
    return m_transferType;
}

QString Transfer::url() const {
    return m_url;
}

void Transfer::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
    }
}
