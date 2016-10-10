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

#ifndef ENCLOSUREDOWNLOAD_H
#define ENCLOSUREDOWNLOAD_H

#include "transfer.h"
#include <QFile>

class DBConnection;
class EnclosureRequest;
class QNetworkReply;
class QNetworkRequest;
class QProcess;

struct Command
{
    Command(const QString &dir, const QString &com) :
        workingDirectory(dir),
        command(com)
    {
    }

    QString workingDirectory;
    QString command;
};

typedef QList<Command> CommandList;

class EnclosureDownload : public Transfer
{
    Q_OBJECT
    
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString customCommand READ customCommand WRITE setCustomCommand NOTIFY customCommandChanged)
    Q_PROPERTY(bool customCommandOverrideEnabled READ customCommandOverrideEnabled WRITE setCustomCommandOverrideEnabled
               NOTIFY customCommandOverrideEnabledChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString subscriptionId READ subscriptionId WRITE setSubscriptionId)

public:
    explicit EnclosureDownload(QObject *parent = 0);
            
    QString category() const;
    void setCategory(const QString &c);
    
    QString customCommand() const;
    void setCustomCommand(const QString &c);
    bool customCommandOverrideEnabled() const;
    void setCustomCommandOverrideEnabled(bool enabled);
    
    QString downloadPath() const;
    void setDownloadPath(const QString &path);
        
    QString fileName() const;
    void setFileName(const QString &fn);
    
    QString subscriptionId() const;
    void setSubscriptionId(const QString &i);
    
public Q_SLOTS:
    virtual void queue();
    virtual void start();
    virtual void pause();
    virtual void cancel();    

private Q_SLOTS:
    void onSubscriptionFetched(DBConnection *connection);
    void onEnclosureRequestFinished(EnclosureRequest *request);
    void onReplyMetaDataChanged();
    void onReplyReadyRead();
    void onReplyFinished();
    void onCustomCommandFinished(int exitCode);
    void onCustomCommandError();
    
Q_SIGNALS:
    void categoryChanged();
    void customCommandChanged();
    void customCommandOverrideEnabledChanged();
    void downloadPathChanged();
    void fileNameChanged();
    void subscriptionIdChanged();

private:
    QProcess* process();

    void startDownload(QNetworkRequest &u, const QByteArray &operation = QByteArray("GET"),
                       const QByteArray &data = QByteArray());
    void startDownload(const QString &u);
    void followRedirect(const QString &u);
        
    bool executeCustomCommands();
    void executeCustomCommand(const Command &command);
    
    void moveDownloadedFiles();
    
    QNetworkReply *m_reply;
    QProcess *m_process;
    
    QFile m_file;
    
    bool m_canceled;
    
    QString m_category;
    
    QString m_customCommand;
    bool m_customCommandOverrideEnabled;
    
    QString m_downloadPath;
        
    QString m_fileName;

    QString m_subscriptionId;
    
    int m_redirects;
    
    CommandList m_commands;
    
    bool m_metadataSet;
};
    
#endif // ENCLOSUREDOWNLOAD_H