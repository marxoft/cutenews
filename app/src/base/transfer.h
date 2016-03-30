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

#include <QString>
#include <QUrl>
#include <QFile>
#include <QSqlQuery>
#include <qplatformdefs.h>

#ifdef MEEGO_EDITION_HARMATTAN
namespace TransferUI {
    class Client;
    class Transfer;
}
#endif

class QNetworkAccessManager;
class QNetworkReply;
class QProcess;

class Transfer : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(qint64 bytesTransferred READ bytesTransferred NOTIFY progressChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(Priority priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(QString priorityString READ priorityString NOTIFY priorityChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(qint64 size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(int subscriptionId READ subscriptionId WRITE setSubscriptionId NOTIFY subscriptionIdChanged)
    Q_PROPERTY(TransferType transferType READ transferType WRITE setTransferType NOTIFY transferTypeChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    
    Q_ENUMS(Priority Status TransferType)
    
public:
    enum Priority {
        HighPriority,
        NormalPriority,
        LowPriority
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
        Unknown
    };
    
    enum TransferType {
        Download,
        Upload
    };
    
    explicit Transfer(QObject *parent = 0);
    ~Transfer();
    
    void setNetworkAccessManager(QNetworkAccessManager *manager);
    
    qint64 bytesTransferred() const;
    
    QString downloadPath() const;
    void setDownloadPath(const QString &path);
    
    QString errorString() const;
        
    QString fileName() const;
    void setFileName(const QString &fn);
    
    QString id() const;
    void setId(const QString &i);
        
    Priority priority() const;
    void setPriority(Priority p);
    QString priorityString() const;
    
    int progress() const;
        
    qint64 size() const;
    void setSize(qint64 s);
    
    Status status() const;
    QString statusString() const;
    
    int subscriptionId() const;
    void setSubscriptionId(int i);
    
    TransferType transferType() const;
    void setTransferType(TransferType type);
    
    QUrl url() const;
    void setUrl(const QUrl &u);
    
    Q_INVOKABLE QByteArray readAll() const;
    
public Q_SLOTS:
    void queue();
    void start();
    void pause();
    void cancel();
    
private:
    void setErrorString(const QString &es);
    
    void setProgress(int p);
            
    void setStatus(Status s);
    
    void getSubscriptionSource();
            
    void startDownload(const QUrl &u);
    void followRedirect(const QUrl &u);
            
    void moveDownloadedFiles();    
    
private Q_SLOTS:
    void onProcessError();
    void onProcessFinished(int exitCode);
        
    void onReplyMetaDataChanged();
    void onReplyReadyRead();
    void onReplyFinished();
    
    void onSubscriptionSourceReady(QSqlQuery query, int requestId);
    
Q_SIGNALS:
    void downloadPathChanged();
    void fileNameChanged();
    void idChanged();
    void priorityChanged();
    void progressChanged();
    void sizeChanged();
    void statusChanged();
    void subscriptionIdChanged();
    void transferTypeChanged();
    void urlChanged();

private:
#ifdef MEEGO_EDITION_HARMATTAN
    static TransferUI::Client *tuiClient;
    TransferUI::Transfer *m_tuiTransfer;
#endif
        
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply;
    QProcess *m_process;
        
    QFile m_file;
    
    bool m_ownNetworkAccessManager;
    bool m_canceled;
        
    QString m_downloadPath;
    
    QString m_errorString;
        
    QString m_fileName;
    
    QString m_id;
    
    Priority m_priority;
    
    int m_progress;
        
    qint64 m_size;
    qint64 m_bytesTransferred;
    
    int m_redirects;
    
    Status m_status;
    
    int m_subscriptionId;
    
    TransferType m_transferType;
    
    QUrl m_url;
    
    int m_requestId;
#ifdef SYMBIAN_OS
    QByteArray m_buffer;
#endif
};
    
#endif // TRANSFER_H
