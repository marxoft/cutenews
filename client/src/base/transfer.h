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

#ifndef TRANSFER_H
#define TRANSFER_H

#include <QObject>
#include <QPointer>
#include <QVariant>

class QNetworkAccessManager;

class Transfer : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(qint64 bytesTransferred READ bytesTransferred NOTIFY bytesTransferredChanged)
    Q_PROPERTY(QString bytesTransferredString READ bytesTransferredString NOTIFY bytesTransferredChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(Priority priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(QString priorityString READ priorityString NOTIFY priorityChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString progressString READ progressString NOTIFY progressChanged)
    Q_PROPERTY(qint64 size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QString sizeString READ sizeString NOTIFY sizeChanged)
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(QString speedString READ speedString NOTIFY speedChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(TransferType transferType READ transferType CONSTANT)
    Q_PROPERTY(QString transferTypeString READ transferTypeString CONSTANT)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    
    Q_ENUMS(DataRole Priority Status TransferType)
    
public:
    enum DataRole {
        BytesTransferredRole = Qt::UserRole + 1,
        BytesTransferredStringRole,
        CategoryRole,
        CustomCommandRole,
        CustomCommandOverrideEnabledRole,
        DownloadPathRole,
        ErrorStringRole,
        FileNameRole,
        IdRole,
        NameRole,
        PriorityRole,
        PriorityStringRole,
        ProgressRole,
        ProgressStringRole,
        SizeRole,
        SizeStringRole,
        SpeedRole,
        SpeedStringRole,
        StatusRole,
        StatusStringRole,
        SubscriptionIdRole,
        TransferTypeRole,
        TransferTypeStringRole,
        UrlRole
    };
    
    enum Priority {
        HighestPriority = 0,
        HighPriority,
        NormalPriority,
        LowPriority,
        LowestPriority
    };
    
    enum Status {
        Paused = 0,
        Canceled,
        Failed,
        Completed,
        Queued,
        Connecting,
        Downloading,
        Uploading,
        ExecutingCustomCommand,
        Unknown
    };
    
    enum TransferType {
        Download = 0,
        EnclosureDownload,
        Upload
    };
    
    explicit Transfer(TransferType transferType, QObject *parent = 0);

    Q_INVOKABLE virtual QVariant data(int role) const;
    Q_INVOKABLE virtual bool setData(int role, const QVariant &value);
    
    QNetworkAccessManager* networkAccessManager();
    void setNetworkAccessManager(QNetworkAccessManager *manager);
    
    qint64 bytesTransferred() const;
    QString bytesTransferredString() const;
    
    QString errorString() const;
    
    QString id() const;
    
    QString name() const;
    
    Priority priority() const;
    void setPriority(Priority p);
    QString priorityString() const;
    
    int progress() const;
    QString progressString() const;
    
    qint64 size() const;
    QString sizeString() const;

    int speed() const;
    QString speedString() const;
    
    Status status() const;
    QString statusString() const;
    
    TransferType transferType() const;
    QString transferTypeString() const;
    
    QString url() const;    

public Q_SLOTS:
    void reload();
    void queue();
    void start();
    void pause();
    void cancel();
    
Q_SIGNALS:
    void bytesTransferredChanged();
    void dataChanged(Transfer *transfer, int role);
    void finished(Transfer *transfer);
    void idChanged();
    void nameChanged();
    void priorityChanged();
    void progressChanged();
    void sizeChanged();
    void speedChanged();
    void statusChanged();
    void urlChanged();

protected:
    void setBytesTransferred(qint64 b);
    
    void setErrorString(const QString &es);
    
    void setId(const QString &i);
    
    void setName(const QString &n);
    
    void setProgress(int p);
    
    void setSize(qint64 s);
    
    void setSpeed(int s);
    
    void setStatus(Status s);
        
    void setUrl(const QString &u);
        
    void updatePriority(Priority p);
    
    void update(const QVariantMap &properties);
    
    virtual void load(const QVariantMap &properties);

private Q_SLOTS:
    void onReplyFinished();

private:    
    QPointer<QNetworkAccessManager> m_nam;
    
    bool m_ownNetworkAccessManager;
    
    qint64 m_bytesTransferred;
    
    QString m_errorString;
        
    QString m_id;
    
    QString m_name;
    
    Priority m_priority;
    
    int m_progress;
    
    qint64 m_size;
    
    int m_speed;
    
    Status m_status;
    
    TransferType m_transferType;
    
    QString m_url;
};

#endif // TRANSFER_H
