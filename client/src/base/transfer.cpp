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
#include "json.h"
#include "requests.h"
#include "utils.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TransferRoleNames : public QHash<int, QByteArray>
{

public:
    TransferRoleNames() :
        QHash<int, QByteArray>()
    {
        insert(Transfer::BytesTransferredRole, "bytesTransferred");
        insert(Transfer::BytesTransferredStringRole, "bytesTransferredString");
        insert(Transfer::CategoryRole, "category");
        insert(Transfer::CustomCommandRole, "customCommand");
        insert(Transfer::CustomCommandOverrideEnabledRole, "customCommandOverrideEnabled");
        insert(Transfer::DownloadPathRole, "downloadPath");
        insert(Transfer::ErrorStringRole, "errorString");
        insert(Transfer::FileNameRole, "fileName");
        insert(Transfer::IdRole, "id");
        insert(Transfer::NameRole, "name");
        insert(Transfer::PluginSettingsRole, "pluginSettings");
        insert(Transfer::PriorityRole, "priority");
        insert(Transfer::PriorityStringRole, "priorityString");
        insert(Transfer::ProgressRole, "progress");
        insert(Transfer::ProgressStringRole, "progressString");
        insert(Transfer::SizeRole, "size");
        insert(Transfer::SizeStringRole, "sizeString");
        insert(Transfer::SpeedRole, "speed");
        insert(Transfer::SpeedStringRole, "speedString");
        insert(Transfer::StatusRole, "status");
        insert(Transfer::StatusStringRole, "statusString");
        insert(Transfer::TransferTypeRole, "transferType");
        insert(Transfer::TransferTypeStringRole, "transferTypeString");
        insert(Transfer::UrlRole, "url");
        insert(Transfer::UsePluginRole, "usePlugin");
    }
};

QHash<int, QByteArray> Transfer::roles = TransferRoleNames();

Transfer::Transfer(Transfer::TransferType transferType, QObject *parent) :
    QObject(parent),
    m_nam(0),
    m_ownNetworkAccessManager(false),
    m_bytesTransferred(0),
    m_priority(NormalPriority),
    m_progress(0),
    m_size(0),
    m_speed(0),
    m_status(Paused),
    m_transferType(transferType)
{
}

QHash<int, QByteArray> Transfer::roleNames() {
    return roles;
}

QVariant Transfer::data(int role) const {
    switch (role) {
    case BytesTransferredRole:
        return bytesTransferred();
    case BytesTransferredStringRole:
        return bytesTransferredString();
    case ErrorStringRole:
        return errorString();
    case IdRole:
        return id();
    case NameRole:
        return name();
    case PriorityRole:
        return priority();
    case PriorityStringRole:
        return priorityString();
    case ProgressRole:
        return progress();
    case ProgressStringRole:
        return progressString();
    case SizeRole:
        return size();
    case SizeStringRole:
        return sizeString();
    case SpeedRole:
        return speed();
    case SpeedStringRole:
        return speedString();
    case StatusRole:
        return status();
    case StatusStringRole:
        return statusString();
    case TransferTypeRole:
        return transferType();
    case TransferTypeStringRole:
        return transferTypeString();
    case UrlRole:
        return url();
    default:
        return QVariant();
    }
}

QVariant Transfer::data(const QByteArray &roleName) {
    return data(roles.key(roleName));
}

bool Transfer::setData(int role, const QVariant &value) {
    switch (role) {
    case PriorityRole:
        setPriority(Priority(value.toInt()));
        return true;
    default:
        return false;
    }
}

bool Transfer::setData(const QByteArray &roleName, const QVariant &value) {
    return setData(roles.key(roleName), value);
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
        emit dataChanged(this, BytesTransferredRole);
    }
}

QString Transfer::bytesTransferredString() const {
    return Utils::formatBytes(bytesTransferred());
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
        emit dataChanged(this, IdRole);
    }
}

QString Transfer::name() const {
    return m_name;
}

void Transfer::setName(const QString &n) {
    if (n != name()) {
        m_name = n;
        emit nameChanged();
        emit dataChanged(this, NameRole);
    }
}

Transfer::Priority Transfer::priority() const {
    return m_priority;
}

void Transfer::setPriority(Priority p) {
    QVariantMap properties;
    properties["priority"] = p;
    update(properties);
}

void Transfer::updatePriority(Priority p) {
    if (p != priority()) {
        m_priority = p;
        emit priorityChanged();
        emit dataChanged(this, PriorityRole);
    }
}

QString Transfer::priorityString() const {
    switch (priority()) {
    case HighestPriority:
        return tr("Highest");
    case HighPriority:
        return tr("High");
    case NormalPriority:
        return tr("Normal");
    case LowPriority:
        return tr("Low");
    case LowestPriority:
        return tr("Lowest");
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
        emit dataChanged(this, ProgressRole);
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

QString Transfer::sizeString() const {
    return Utils::formatBytes(size());
}

int Transfer::speed() const {
    switch (status()) {
    case Downloading:
    case Uploading:
        return m_speed;
    default:
        return 0;
    }
}

void Transfer::setSpeed(int s) {
    if (s != speed()) {
        m_speed = s;
        emit speedChanged();
        emit dataChanged(this, SpeedRole);
    }
}

QString Transfer::speedString() const {
    return Utils::formatBytes(speed()) + "/s";
}

Transfer::Status Transfer::status() const {
    return m_status;
}

void Transfer::setStatus(Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged();
        emit dataChanged(this, StatusRole);
        
        switch (s) {
        case Paused:
        case Canceled:
        case Failed:
        case Completed:
            emit finished(this);
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

QString Transfer::transferTypeString() const {
    switch (transferType()) {
    case Download:
        return tr("Download");
    case EnclosureDownload:
        return tr("Enclosure download");
    case Upload:
        return tr("Upload");
    default:
        return QString();
    }
}

QString Transfer::url() const {
    return m_url;
}

void Transfer::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
        emit dataChanged(this, UrlRole);
    }
}

void Transfer::reload() {
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/" + id()));
    connect(reply, SIGNAL(finished()), this, SLOT(onChanged()));
}

void Transfer::queue() {
    start();
}

void Transfer::start() {
    QVariantMap params;
    params["id"] = id();
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/start", params));
    connect(reply, SIGNAL(finished()), this, SLOT(onChanged()));
}

void Transfer::pause() {
    QVariantMap params;
    params["id"] = id();
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/pause", params));
    connect(reply, SIGNAL(finished()), this, SLOT(onChanged()));
}

void Transfer::cancel() {
    QVariantMap params;
    params["id"] = id();
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/cancel", params));
    connect(reply, SIGNAL(finished()), this, SLOT(onCanceled()));
}

void Transfer::update(const QVariantMap &properties) {
    QNetworkReply *reply = networkAccessManager()->put(buildRequest("/transfers/" + id(),
                                                       QNetworkAccessManager::PutOperation),
                                                       QtJson::Json::serialize(properties));
    connect(reply, SIGNAL(finished()), this, SLOT(onChanged()));
}

void Transfer::load(const QVariantMap &properties) {
    if (!properties.isEmpty()) {
        setBytesTransferred(properties.value("bytesTransferred", 0).toLongLong());
        setErrorString(properties.value("errorString").toString());
        setId(properties.value("id").toString());
        setName(properties.value("name").toString());
        updatePriority(Priority(properties.value("priority", NormalPriority).toInt()));
        setProgress(properties.value("progress", 0).toInt());
        setSize(properties.value("size", 0).toLongLong());
        setSpeed(properties.value("speed", 0).toInt());
        setStatus(Status(properties.value("status", Paused).toInt()));
        setUrl(properties.value("url").toString());
    }
}

void Transfer::onChanged() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) {
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        load(QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toMap());
    }
    
    reply->deleteLater();
}

void Transfer::onCanceled() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) {
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        setStatus(Canceled);
    }
    
    reply->deleteLater();
}
