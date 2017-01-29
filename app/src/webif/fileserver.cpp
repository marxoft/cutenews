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

#include "fileserver.h"
#include "definitions.h"
#include "diskcache.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>

FileServer::FileServer(QObject *parent) :
    QObject(parent),
    m_nam(0)
{
}

bool FileServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    if (request->method() != QHttpRequest::HTTP_GET) {
        response->setHeader("Content-Length", "0");
        response->writeHead(QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
        response->end();
        return true;
    }
    
    QString filePath = request->path();
    const QString dir = filePath.left(filePath.lastIndexOf("/") + 1);

    if (!dir.startsWith(CACHE_PATH)) {
        filePath = filePath.mid(filePath.indexOf("/") + 1);
        filePath.prepend(WEB_INTERFACE_PATH);
    }
    
    if (!QFile::exists(filePath)) {
        if (dir.startsWith(CACHE_PATH)) {
            const QByteArray url = QByteArray::fromBase64(filePath.mid(filePath.lastIndexOf("/") + 1).toUtf8());
            getCachedFile(dir, QUrl::fromEncoded(url), response);
            return true;
        }
        
        return false;
    }
    
    QFile file(filePath);
    
    if (file.open(QFile::ReadOnly)) {
        response->setHeader("Content-Length", QByteArray::number(file.size()));
        response->writeHead(QHttpResponse::STATUS_OK);
        response->end(file.readAll());
        file.close();
        return true;
    }
    
    response->setHeader("Content-Length", "0");
    response->writeHead(QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
    response->end();
    return true;
}

QNetworkDiskCache* FileServer::cache(const QString &cacheDir) {
    QNetworkAccessManager *manager = networkAccessManager();
    DiskCache *dc = qobject_cast<DiskCache*>(manager->cache());
    
    if (!dc) {
        dc = new DiskCache(manager);
        manager->setCache(dc);
    }

    if (m_replies.isEmpty()) {
        dc->setCacheDirectory(cacheDir);
    }

    return dc;
}

QNetworkAccessManager* FileServer::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(writeCachedFile(QNetworkReply*)));
    }

    return m_nam;
}

void FileServer::getCachedFile(const QString &cacheDir, const QUrl &url, QHttpResponse *response) {
    if (cache(cacheDir)->cacheDirectory() == cacheDir) {
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply *reply = networkAccessManager()->get(request);
        m_replies.insert(reply, response);
    }
    else {
        response->setProperty("cacheDir", cacheDir);
        response->setProperty("url", url);
        m_queue.enqueue(response);
    }
}

void FileServer::writeCachedFile(QNetworkReply *reply) {
    QHttpResponse *response = m_replies.value(reply);

    if (response) {
        QString redirect = QString::fromUtf8(reply->rawHeader("Location"));

        if (!redirect.isEmpty()) {
            int redirects = response->property("redirects").toInt();

            if (redirects < MAX_REDIRECTS) {
                response->setProperty("redirects", ++redirects);

                if (!redirect.startsWith("http")) {
                    const QUrl url = reply->url();
                    
                    if (redirect.startsWith("/")) {
                        redirect.prepend(url.scheme() + "://" + url.authority());
                    }
                    else {
                        redirect.prepend(url.scheme() + "://" + url.authority() + "/");
                    }
                }

                QNetworkAccessManager *manager = reply->manager();

                if (manager) {
                    QNetworkRequest request(redirect);
                    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
                    m_replies.insert(manager->get(request), response);
                }
                else {
                    response->setHeader("Content-Length", "0");
                    response->writeHead(QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
                    response->end();
                }
            }
        }
        else if (reply->error() == QNetworkReply::NoError) {
            response->setHeader("Content-Length", QByteArray::number(reply->bytesAvailable()));
            response->writeHead(QHttpResponse::STATUS_OK);
            response->end(reply->readAll());
        }
        else {
            response->setHeader("Content-Length", "0");
            response->writeHead(QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
            response->end();
        }
    }

    m_replies.remove(reply);
    reply->deleteLater();

    if ((m_replies.isEmpty()) && (!m_queue.isEmpty())) {
       QHttpResponse *response = m_queue.dequeue();

       if (response) {
           getCachedFile(response->property("cacheDir").toString(), response->property("url").toString(), response);
       }
    }
}
