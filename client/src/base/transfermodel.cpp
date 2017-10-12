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

#include "transfermodel.h"
#include "download.h"
#include "enclosuredownload.h"
#include "json.h"
#include "logger.h"
#include "requests.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

TransferModel* TransferModel::self = 0;

TransferModel::TransferModel() :
    QAbstractListModel(),
    m_nam(0),
    m_status(Idle)
{
#if QT_VERSION < 0x050000
    setRoleNames(Transfer::roleNames());
#endif
}

TransferModel::~TransferModel() {
    self = 0;
}

TransferModel* TransferModel::instance() {
    return self ? self : self = new TransferModel;
}

QString TransferModel::errorString() const {
    return m_errorString;
}

void TransferModel::setErrorString(const QString &e) {
    m_errorString = e;
    
    if (!e.isEmpty()) {
        emit error(e);
    }
}

TransferModel::Status TransferModel::status() const {
    return m_status;
}

void TransferModel::setStatus(TransferModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> TransferModel::roleNames() const {
    return Transfer::roleNames();
}
#endif

int TransferModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int TransferModel::columnCount(const QModelIndex &) const {
    return 5;
}

QVariant TransferModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Priority");
        case 2:
            return tr("Progress");
        case 3:
            return tr("Speed");
        case 4:
            return tr("Status");
        default:
            break;
        }
    }
    
    return QVariant();
}

QVariant TransferModel::data(const QModelIndex &index, int role) const {
    if (const Transfer *transfer = get(index.row())) {
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0:
                return transfer->data(Transfer::NameRole);
            case 1:
                return transfer->data(Transfer::PriorityStringRole);
            case 2:
                return transfer->data(Transfer::ProgressStringRole);
            case 3:
                return transfer->data(Transfer::SpeedStringRole);
            case 4:
                return transfer->data(Transfer::StatusStringRole);
            default:
                return QVariant();
            }
        }
        
        return transfer->data(role);
    }
    
    return QVariant();
}

QMap<int, QVariant> TransferModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
    
    if (const Transfer *transfer = get(index.row())) {
        for (int i = Transfer::BytesTransferredRole; i <= Transfer::UsePluginRole; i++) {
            map[i] = transfer->data(i);
        }
    }
    
    return map;
}

bool TransferModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (Transfer *transfer = get(index.row())) {
        return transfer->setData(role, value);
    }
    
    return false;
}

bool TransferModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    QMapIterator<int, QVariant> iterator(roles);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(index, iterator.value(), iterator.key())) {
            return false;
        }
    }

    return true;
}

QVariant TransferModel::data(int row, const QByteArray &role) const {
    return data(index(row), roleNames().key(role));
}

QVariantMap TransferModel::itemData(int row) const {
    QVariantMap map;
    
    if (const Transfer *transfer = get(row)) {
        for (int i = Transfer::BytesTransferredRole; i <= Transfer::UsePluginRole; i++) {
            map[roleNames().value(i)] = transfer->data(i);
        }
    }

    return map;
}

bool TransferModel::setData(int row, const QVariant &value, const QByteArray &role) {
    return setData(index(row), value, roleNames().key(role));
}

bool TransferModel::setItemData(int row, const QVariantMap &roles) {
    QMapIterator<QString, QVariant> iterator(roles);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(row, iterator.value(), iterator.key().toUtf8())) {
            return false;
        }
    }

    return true;
}

QModelIndexList TransferModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                     Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int TransferModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList indexes = match(index(start), roleNames().key(role), value, Qt::MatchFlags(flags));
    return indexes.isEmpty() ? -1 : indexes.first().row();
}

Transfer* TransferModel::get(int row) const {
    if ((row >= 0) && (row < m_items.size())) {
        return m_items.at(row);
    }
    
    return 0;
}

Transfer* TransferModel::get(const QString &id) const {
    foreach (Transfer *transfer, m_items) {
        if (transfer->id() == id) {
            return transfer;
        }
    }
    
    return 0;
}

void TransferModel::addEnclosureDownload(const QString &url, bool usePlugin) {
    addEnclosureDownload(url, QString(), false, QString(), Transfer::NormalPriority, usePlugin);
}

void TransferModel::addEnclosureDownload(const QString &url, const QString &command, bool overrideGlobalCommand,
        const QString &category, int priority, bool usePlugin) {
    Logger::log("TransferModel::addEnclosureDownload(). URL: " + url, Logger::LowVerbosity);
    
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QVariantMap properties;
    properties["url"] = url;
    properties["customCommand"] = command;
    properties["customCommandOverrideEnabled"] = overrideGlobalCommand;
    properties["category"] = category;
    properties["priority"] = priority;
    properties["usePlugin"] = usePlugin;
    QNetworkReply *reply =
        networkAccessManager()->post(buildRequest("/transfers", QNetworkAccessManager::PostOperation),
                                                  QtJson::Json::serialize(QVariantList() << properties));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersLoaded()));
}

void TransferModel::start() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/start"));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersChanged()));
}

void TransferModel::pause() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers/pause"));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersChanged()));
}

bool TransferModel::start(const QString &id) {
    if (Transfer *transfer = get(id)) {
        transfer->queue();
        return true;
    }
    
    return false;
}

bool TransferModel::pause(const QString &id) {
    if (Transfer *transfer = get(id)) {
        transfer->pause();
        return true;
    }
    
    return false;
}

bool TransferModel::cancel(const QString &id) {
    if (Transfer *transfer = get(id)) {
        transfer->cancel();
        return true;
    }
    
    return false;
}

void TransferModel::load() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    clear();
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/transfers"));
    connect(reply, SIGNAL(finished()), this, SLOT(onTransfersLoaded()));
}

void TransferModel::append(Transfer *transfer) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items << transfer;
    connect(transfer, SIGNAL(dataChanged(Transfer*, int)), this, SLOT(onTransferDataChanged(Transfer*, int)));
    connect(transfer, SIGNAL(finished(Transfer*)), this, SLOT(onTransferFinished(Transfer*)));
    endInsertRows();
    emit countChanged(rowCount());
}

void TransferModel::remove(int row) {
    if ((row >= 0) && (row < m_items.size())) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.takeAt(row)->deleteLater();
        endRemoveRows();
        emit countChanged(rowCount());
    }
}

void TransferModel::clear() {
    if (!m_items.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_items);
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

QNetworkAccessManager* TransferModel::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }
    
    return m_nam;
}

void TransferModel::onTransferDataChanged(Transfer* transfer, int role) {
    const int row = m_items.indexOf(transfer);

    if (row == -1) {
        return;
    }

    int column = 0;

    switch (role) {
    case Transfer::BytesTransferredRole:
    case Transfer::ProgressRole:
    case Transfer::SizeRole:
        column = 2;
        break;
    case Transfer::SpeedRole:
        column = 3;
        break;
    case Transfer::StatusRole:
        column = 4;
        break;
    case Transfer::PriorityRole:
        column = 1;
        break;
    default:
        break;
    }

    const QModelIndex idx = index(row, column);
    emit dataChanged(idx, idx);
}

void TransferModel::onTransferFinished(Transfer *transfer) {
    const int row = m_items.indexOf(transfer);

    if (row == -1) {
        return;
    }
    
    switch (transfer->status()) {
    case Transfer::Completed:
    case Transfer::Canceled:
        remove(row);
        break;
    default:
        break;
    }
}

void TransferModel::onTransfersChanged() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        setErrorString(QString());
        setStatus(Ready);
        load();
    }
    else {
        setErrorString(reply->errorString());
        setStatus(Error);
    }
    
    reply->deleteLater();
}

void TransferModel::onTransfersLoaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    
    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        const QVariantList list = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toList();
        
        foreach (const QVariant &v, list) {
            const QVariantMap result = v.toMap();
            const Transfer::TransferType type = Transfer::TransferType(result.value("transferType", Transfer::Download)
                                                                       .toInt());
            Transfer *transfer;
            
            switch (type) {
            case Transfer::EnclosureDownload:
                transfer = new EnclosureDownload(result, this);
                break;
            default:
                transfer = new Download(result, this);
                break;
            }
            
            append(transfer);
        }
        
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(reply->errorString());
        setStatus(Error);
    }
    
    reply->deleteLater();
}
