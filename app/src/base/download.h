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

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "transfer.h"
#include <QTime>

class QNetworkReply;

class Download : public Transfer
{
    Q_OBJECT

public:
    explicit Download(QObject *parent = 0);
            
    Q_INVOKABLE QByteArray readAll() const;
    
public Q_SLOTS:
    virtual void queue();
    virtual void start();
    virtual void pause();
    virtual void cancel();    

private Q_SLOTS:
    void onReplyMetaDataChanged();
    void onReplyReadyRead();
    void onReplyFinished();

private:
    void startDownload(const QString &u);
    void followRedirect(const QString &u);
    
    QNetworkReply *m_reply;
    
    bool m_canceled;
    
    int m_redirects;
    
    bool m_metadataSet;
    
    QByteArray m_response;

    QTime m_speedTime;
};
    
#endif // DOWNLOAD_H
