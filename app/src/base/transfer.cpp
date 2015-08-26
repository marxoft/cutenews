/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "database.h"
#include "definitions.h"
#include "json.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionplugins.h"
#include "utils.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDir>
#include <QProcess>
#ifdef MEEGO_EDITION_HARMATTAN
#include <TransferUI/Client>
#include <TransferUI/Transfer>
#endif
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

#ifdef MEEGO_EDITION_HARMATTAN
TransferUI::Client* Transfer::tuiClient = 0;
#endif
#ifdef SYMBIAN_OS
static const qint64 BUFFER_SIZE = 1024 * 512;
#endif

Transfer::Transfer(QObject *parent) :
    QObject(parent),
    m_nam(0),
    m_reply(0),
    m_process(0),
    m_ownNetworkAccessManager(false),
    m_canceled(false),
    m_priority(NormalPriority),
    m_progress(0),
    m_size(0),
    m_bytesTransferred(0),
    m_redirects(0),
    m_status(Paused),
    m_subscriptionId(-1),
    m_transferType(Download),
    m_requestId(-1)
{
#ifdef MEEGO_EDITION_HARMATTAN
    if (!tuiClient) {
        tuiClient = new TransferUI::Client;
        tuiClient->init();
    }
    
    m_tuiTransfer = tuiClient->registerTransfer(QString(), TransferUI::Client::TRANSFER_TYPES_DOWNLOAD);
    m_tuiTransfer->setIcon("icon-m-content-attachment");

    connect(m_tuiTransfer, SIGNAL(start()), this, SLOT(queue()));
    connect(m_tuiTransfer, SIGNAL(pause()), this, SLOT(pause()));
    connect(m_tuiTransfer, SIGNAL(cancel()), this, SLOT(cancel()));
    connect(m_tuiTransfer, SIGNAL(repairError()), this, SLOT(queue()));
#endif
}

Transfer::~Transfer() {
    if (m_reply) {
        delete m_reply;
        m_reply = 0;
    }
#ifdef MEEGO_EDITION_HARMATTAN
    if (m_tuiTransfer) {
        if (tuiClient) {
            tuiClient->removeTransfer(m_tuiTransfer->transferId());
        }

        delete m_tuiTransfer;
        m_tuiTransfer = 0;
    }
#endif
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

QString Transfer::downloadPath() const {
    return m_downloadPath;
}

void Transfer::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        m_downloadPath = path.endsWith("/") ? path : path + "/";
        emit downloadPathChanged();
        
        if (!fileName().isEmpty()) {
            m_file.setFileName(downloadPath() + fileName());
            m_bytesTransferred = m_file.size();
            
            if ((m_size > 0) && (m_bytesTransferred > 0)) {
                setProgress(m_bytesTransferred * 100 / m_size);
            }
        }
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setDownloadPath" << path;
#endif
}

QString Transfer::errorString() const {
    return m_errorString;
}

void Transfer::setErrorString(const QString &es) {
    m_errorString = es;
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setErrorString" << es;
#endif
}

QString Transfer::fileName() const {
    return m_fileName;
}

void Transfer::setFileName(const QString &name) {
    if (name != fileName()) {
        m_fileName = name;

        switch (transferType()) {
        case Transfer::Download:
            m_fileName.replace(ILLEGAL_FILENAME_CHARS_RE, "_");
            break;
        default:
            break;
        }

        emit fileNameChanged();
        
        if (!downloadPath().isEmpty()) {
            m_file.setFileName(downloadPath() + fileName());
            m_bytesTransferred = m_file.size();
            
            if ((m_size > 0) && (m_bytesTransferred > 0)) {
                setProgress(m_bytesTransferred * 100 / m_size);
            }
        }
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setFileName" << name;
#endif
}

QString Transfer::id() const {
    return m_id;
}

void Transfer::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setId" << i;
#endif
}

Transfer::Priority Transfer::priority() const {
    return m_priority;
}

void Transfer::setPriority(Priority p) {
    if (p != priority()) {
        m_priority = p;
        emit priorityChanged();
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setPriority" << p;
#endif
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
#ifdef MEEGO_EDITION_HARMATTAN
        if ((tuiClient) && (tuiClient->isTUIVisible())) {
            if (m_tuiTransfer) {
                m_tuiTransfer->setProgress(float(p) / 100);
            }
        }
#endif
    }
}

qint64 Transfer::size() const {
    return m_size;
}

void Transfer::setSize(qint64 s) {
    if (s != size()) {
        m_size = s;
        emit sizeChanged();
#ifdef MEEGO_EDITION_HARMATTAN
        if (m_tuiTransfer) {
            m_tuiTransfer->setSize(s);
        }
#endif
        if ((m_size > 0) && (m_bytesTransferred > 0)) {
            setProgress(m_bytesTransferred * 100 / m_size);
        }
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setSize" << s;
#endif
}

Transfer::Status Transfer::status() const {
    return m_status;
}

void Transfer::setStatus(Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged();
#ifdef MEEGO_EDITION_HARMATTAN
        switch (s) {
        case Transfer::Queued:
            if (m_tuiTransfer) m_tuiTransfer->setPending(statusString());
            break;
        case Transfer::Downloading:
            if (m_tuiTransfer) m_tuiTransfer->markResumed();
            break;
        case Transfer::Completed:
            if (m_tuiTransfer) m_tuiTransfer->markCompleted();
            break;
        case Transfer::Canceled:
            if (m_tuiTransfer) m_tuiTransfer->markCancelled();
            break;
        case Transfer::Paused:
            if (m_tuiTransfer) m_tuiTransfer->markPaused();
            break;
        case Transfer::Failed:
            if (m_tuiTransfer) m_tuiTransfer->markRepairableFailure(statusString(), errorString(), tr("Retry"));
            break;
        default:
            break;
        }
#endif
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setStatus" << s;
#endif
}

QString Transfer::statusString() const {
    switch (status()) {
    case Paused:
        return tr("Paused");
    case Canceled:
        return tr("Canceled");
    case Failed:
        return tr("Failed");
    case Completed:
        return tr("Completed");
    case Queued:
        return tr("Queued");
    case Downloading:
        return tr("Downloading");
    case Uploading:
        return tr("Uploading");
    default:
        return QString();
    }
}

int Transfer::subscriptionId() const {
    return m_subscriptionId;
}

void Transfer::setSubscriptionId(int i) {
    if (i != subscriptionId()) {
        m_subscriptionId = i;
        emit subscriptionIdChanged();
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setSubscriptionId" << i;
#endif
}

Transfer::TransferType Transfer::transferType() const {
    return m_transferType;
}

void Transfer::setTransferType(TransferType type) {
    if (type != transferType()) {
        m_transferType = type;
        emit transferTypeChanged();
#ifdef MEEGO_EDITION_HARMATTAN
        if (m_tuiTransfer) {
            switch (type) {
            case Transfer::Upload:
                m_tuiTransfer->setTransferType(TransferUI::Client::TRANSFER_TYPES_UPLOAD);
                return;
            default:
                m_tuiTransfer->waitForCommit();
                m_tuiTransfer->setTransferType(TransferUI::Client::TRANSFER_TYPES_DOWNLOAD);
                m_tuiTransfer->setCanPause(true);
                m_tuiTransfer->commit();
                return;
            }
        }
#endif
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setTransferType" << type;
#endif
}

QUrl Transfer::url() const {
    return m_url;
}

void Transfer::setUrl(const QUrl &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::setUrl" << u;
#endif
}

QByteArray Transfer::readAll() const {
    return (m_reply) && (!m_reply->isRunning()) ? m_reply->readAll() : QByteArray();
}

void Transfer::queue() {
    switch (status()) {
    case Queued:
    case Downloading:
    case Uploading:
        return;
    default:
        break;
    }
    
    setStatus(Queued);
}

void Transfer::start() {
    switch (status()) {
    case Downloading:
    case Uploading:
        return;
    default:
        break;
    }
    
    switch (transferType()) {
    case Upload:
        return;
    default:
        break;
    }
    
    if (subscriptionId() == -1) {
        startDownload(url());
    }
    else {
        getSubscriptionSource();
    }
}

void Transfer::pause() {
    switch (status()) {
    case Paused:
    case Canceled:
    case Completed:
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

void Transfer::cancel() {
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
        m_file.remove();
        QDir().rmdir(downloadPath());
        setStatus(Canceled);
    }
}

void Transfer::getSubscriptionSource() {
    if (m_requestId == -1) {
        m_requestId = Utils::createId();
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::getSubscriptionSource. Subscription id" << subscriptionId() << ", request id" << m_requestId;
#endif
    connect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
            this, SLOT(onSubscriptionSourceReady(QSqlQuery, int)));
    Database::execQuery(QString("SELECT source, sourceType FROM subscriptions WHERE id = %1").arg(subscriptionId()),
                        m_requestId);
}

void Transfer::startDownload(const QUrl &u) {    
    const bool saveToFile = !m_file.fileName().isEmpty();
    
    if (saveToFile) {
        QDir().mkpath(downloadPath());
        
        if (!m_file.open(m_file.exists() ? QFile::Append : QFile::WriteOnly)) {
            setErrorString(m_file.errorString());
            setStatus(Failed);
            return;
        }
    }
    
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownNetworkAccessManager = true;
    }
    
    QNetworkRequest request(u);
    
    if (m_bytesTransferred > 0) {
        request.setRawHeader("Range", "bytes=" + QByteArray::number(m_bytesTransferred) + "-");
#ifdef CUTENEWS_DEBUG
        qDebug() << "Transfer::startDownload: Resuming download from" << m_bytesTransferred;
#endif
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::startDownload: Downloading" << u;
#endif
    setStatus(Downloading);
    
    m_redirects = 0;
    
    if (m_reply) {
        delete m_reply;
    }
    
    m_reply = m_nam->get(request);
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    
    if (saveToFile) {
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
}

void Transfer::followRedirect(const QUrl &u) {    
    const bool saveToFile = !m_file.fileName().isEmpty();
    
    if (saveToFile) {
        QDir().mkpath(downloadPath());
        
        if (!m_file.open(m_file.exists() ? QFile::Append : QFile::WriteOnly)) {
            setErrorString(m_file.errorString());
            setStatus(Failed);
            return;
        }
    }
    
    m_redirects++;

    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        m_ownNetworkAccessManager = true;
    }
    
    QNetworkRequest request(u);
    
    if (m_bytesTransferred > 0) {
        request.setRawHeader("Range", "bytes=" + QByteArray::number(m_bytesTransferred) + "-");
#ifdef CUTENEWS_DEBUG
        qDebug() << "Transfer::followRedirect: Resuming download from" << m_bytesTransferred;
#endif
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::followRedirect: Downloading" << u;
#endif
    if (m_reply) {
        delete m_reply;
    }
    
    m_reply = m_nam->get(request);
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    
    if (saveToFile) {
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    }
}

void Transfer::moveDownloadedFiles() {
    QDir destDir(Settings::instance()->downloadPath());
    
    if (!destDir.mkpath(destDir.path())) {
        setErrorString(tr("Cannot make download path %1").arg(destDir.path()));
        setStatus(Failed);
        return;
    }
    
    QDir downDir(downloadPath());
    
    foreach (QString oldFileName, downDir.entryList(QDir::Files)) {
        int i = 0;
        QString newFileName = QString("%1/%2").arg(destDir.path()).arg(oldFileName);

        while ((destDir.exists(newFileName)) && (i < 100)) {
            i++;
            newFileName = (i == 1 ? QString("%1(%2)%3").arg(newFileName.left(newFileName.lastIndexOf('.')))
                                                       .arg(i).arg(newFileName.mid(newFileName.lastIndexOf('.')))
                                  : QString("%1(%2)%3").arg(newFileName.left(newFileName.lastIndexOf('(')))
                                                       .arg(i).arg(newFileName.mid(newFileName.lastIndexOf('.'))));
        }
#ifdef CUTENEWS_DEBUG
        qDebug() << "Transfer::moveDownloadedFiles: Renaming downloaded file to" << newFileName;
#endif
        if (!destDir.rename(downDir.absoluteFilePath(oldFileName), newFileName)) {
            setErrorString(tr("Cannot rename downloaded file to %1").arg(newFileName));
            setStatus(Failed);
            return;
        }
    }
        
    downDir.rmdir(downDir.path());
    setErrorString(QString());
    setStatus(Completed);
}

void Transfer::onProcessError() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::onProcessError" << m_process->errorString();
#endif
    setErrorString(tr("Cannot obtain redirect from plugin."));
    setStatus(Failed);
}

void Transfer::onProcessFinished(int exitCode) {
    if (exitCode == 0) {
        const QVariant response = QtJson::Json::parse(m_process->readAllStandardOutput());
        
        if (response.type() == QVariant::Map) {
            const QVariantMap map = response.toMap();
            
            if (map.contains("url")) {
                if (map.contains("fileName")) {
                    setFileName(map.value("fileName").toString());
                }
                
                startDownload(map.value("url").toString());
                return;
            }
            
            if (map.contains("error")) {
                setErrorString(map.value("error").toString());
                setStatus(Failed);
                return;
            }
        }
        else {
            const QUrl u = response.toString();
        
            if (u.isValid()) {
                startDownload(u);
                return;
            }
        }
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Transfer::onProcessFinished" << m_process->errorString();
#endif
    setErrorString(tr("Cannot obtain redirect from plugin."));
    setStatus(Failed);
}

void Transfer::onReplyMetaDataChanged() {
    if (size() > 0) {
        return;
    }
    
    qint64 s = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    
    if (s <= 0) {
        s = m_reply->rawHeader("Content-Length").toLongLong();
    }

    if (s <= 0) {
        QVariant redirect = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if (!redirect.isNull()) {
            return;
        }

        redirect = m_reply->header(QNetworkRequest::LocationHeader);

        if (!redirect.isNull()) {
            return;
        }
    }
    
    setSize(s);
}

void Transfer::onReplyReadyRead() {
    m_bytesTransferred += m_reply->bytesAvailable();
#ifdef SYMBIAN_OS
    m_buffer += m_reply->readAll();

    if (m_buffer.size() >= BUFFER_SIZE) {
        m_file.write(m_buffer);
        m_buffer.clear();
    }
#else
    m_file.write(m_reply->readAll());
#endif
    if (m_size > 0) {
        setProgress(m_bytesTransferred * 100 / m_size);
    }
}

void Transfer::onReplyFinished() {
    QNetworkReply::NetworkError error = m_reply->error();
    QString errorString = m_reply->errorString();
    QVariant redirect = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (redirect.isNull()) {
        redirect = m_reply->header(QNetworkRequest::LocationHeader);        
    }
    
    const bool saveToFile = !m_file.fileName().isEmpty();
    
    if (saveToFile) {
#ifdef SYMBIAN_OS
        if (!m_buffer.isEmpty()) {
            m_file.write(m_buffer);
            m_buffer.clear();
        }
#endif
        m_file.close();
    }
        
    if (!redirect.isNull()) {
        m_reply->deleteLater();
        m_reply = 0;
        
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect.toString());
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Failed);
        }
        
        return;
    }
        
    switch (error) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        m_reply->deleteLater();
        m_reply = 0;
        setErrorString(QString());
        
        if (m_canceled) {
            if (saveToFile) {
                m_file.remove();
                QDir().rmdir(downloadPath());
            }
            
            setStatus(Canceled);
        }
        else {
            setStatus(Paused);
        }
        
        return;
    default:
        m_reply->deleteLater();
        m_reply = 0;
        setErrorString(errorString);
        setStatus(Failed);
        return;
    }
    
    if (saveToFile) {
        m_reply->deleteLater();
        m_reply = 0;
        moveDownloadedFiles();
    }
    else {
        setErrorString(QString());
        setStatus(Completed);
    }
}

void Transfer::onSubscriptionSourceReady(QSqlQuery query, int requestId) {
    if (requestId == m_requestId) {
        disconnect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                   this, SLOT(onSubscriptionSourceReady(QSqlQuery, int)));
        
        if (!query.next()) {
            setErrorString(tr("Cannot obtain redirect from plugin."));
            setStatus(Failed);
            return;
        }
        
        disconnect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionSourceReady(QSqlQuery, int)));
        
        const int sourceType = query.value(1).toInt();
        
        if (sourceType == Subscription::Plugin) {
            if (!m_process) {
                m_process = new QProcess(this);
                connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int)));
                connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
            }
        
            const QVariantMap plugin = QtJson::Json::parse(query.value(0).toString()).toMap();
            QString command = SubscriptionPlugins::command(plugin.value("pluginName").toString());
            
            if (command.isEmpty()) {
                setErrorString(tr("Cannot obtain redirect from plugin."));
                setStatus(Failed);
                return;
            }
        
            if (plugin.contains("params")) {
                QMapIterator<QString, QVariant> iterator(plugin.value("params").toMap());
            
                while (iterator.hasNext()) {
                    iterator.next();
                    command.append(" -");
                    command.append(iterator.key());
                    command.append(" ");
                    command.append(QtJson::Json::serialize(iterator.value()));
                }
            }
            
            command.append(" -e ");
            command.append(url().toString());
#ifdef CUTENEWS_DEBUG
            qDebug() << "Transfer::onSubscriptionSourceReady. Retrieving redirect from plugin:"
                     << plugin.value("pluginName").toString() << "with command:" << command;
#endif
            m_process->start(command);
            return;
        }
        
        startDownload(url());
    }
}
