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

#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QHash>
#include <QQueue>

class QHttpRequest;
class QHttpResponse;
class QNetworkAccessManager;
class QNetworkDiskCache;
class QNetworkReply;

class FileServer : public QObject
{
    Q_OBJECT

public:
    explicit FileServer(QObject *parent = 0);
    
    bool handleRequest(QHttpRequest *request, QHttpResponse *response);

private Q_SLOTS:
    void writeCachedFile(QNetworkReply *reply);
    void onResponseDone();

private:
    QNetworkDiskCache* cache(const QString &cacheDir);
    QNetworkAccessManager* networkAccessManager();
    
    void insertReply(QNetworkReply *reply, QHttpResponse *response);
    QHttpResponse* takeReply(QNetworkReply *reply);
    
    void enqueueResponse(QHttpResponse *response);
    QHttpResponse* dequeueResponse();
    
    void getCachedFile(const QString &cacheDir, const QUrl &url, QHttpResponse *response);    

    QNetworkAccessManager *m_nam;

    QQueue<QHttpResponse*> m_responses;
    QHash<QNetworkReply*, QHttpResponse*> m_replies;
};

#endif // FILESERVER_H
