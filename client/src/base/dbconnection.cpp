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

#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "json.h"
#include "logger.h"
#include "requests.h"
#include <QNetworkReply>

DBConnection::DBConnection(QObject *parent) :
    QObject(parent),
    m_nam(0),
    m_progress(0),
    m_status(Idle)
{
}

DBConnection::~DBConnection() {
    close();
}

QString DBConnection::errorString() const {
    return m_errorString;
}

void DBConnection::setErrorString(const QString &e) {
    m_errorString = e;

    if (!e.isEmpty()) {
        Logger::log("DBConnection::error(). " + e);
        emit DBNotify::instance()->error(e);
    }
}

int DBConnection::progress() const {
    return m_progress;
}

void DBConnection::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

QVariant DBConnection::result() const {
    return m_result;
}

void DBConnection::setResult(const QVariant &r) {
    m_result = r;
}

DBConnection::Status DBConnection::status() const {
    return m_status;
}

void DBConnection::setStatus(DBConnection::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
        
        switch (s) {
        case Active:
            setProgress(0);
            break;
        default:
            setProgress(100);
            break;
        }
    }
}

DBConnection* DBConnection::connection() {
    return new DBConnection;
}

DBConnection* DBConnection::connection(QObject *obj, const char *slot) {
    DBConnection *conn = new DBConnection(obj);
    QObject::connect(conn, SIGNAL(finished(DBConnection*)), obj, slot);
    return conn;
}

void DBConnection::addSubscription(const QVariantMap &properties) {
    if (status() == Active) {
        return;
    }
    
    addSubscriptions(QVariantList() << properties);
}

void DBConnection::addSubscriptions(const QVariantList &subscriptions) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply =
    networkAccessManager(SLOT(onSubscriptionsAdded(QNetworkReply*)))->post(buildRequest("/subscriptions",
                                                                           QNetworkAccessManager::PostOperation),
                                                                           QtJson::Json::serialize(subscriptions));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::deleteSubscription(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply =
    networkAccessManager(SLOT(onSubscriptionDeleted(QNetworkReply*)))->deleteResource(buildRequest("/subscriptions/" + id, QNetworkAccessManager::DeleteOperation));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::updateSubscription(const QString &id, const QVariantMap &properties) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply =
    networkAccessManager(SLOT(onSubscriptionUpdated(QNetworkReply*)))->put(buildRequest("/subscriptions/" + id,
                                                                           QNetworkAccessManager::PutOperation),
                                                                           QtJson::Json::serialize(properties));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::markSubscriptionRead(const QString &id, bool isRead) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const QString path = isRead ? QString("/subscriptions/read") : QString("/subscriptions/unread");
    QVariantMap params;
    params["id"] = id;
    QNetworkReply *reply =
    networkAccessManager(SLOT(onSubscriptionRead(QNetworkReply*)))->get(buildRequest(path, params));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::markAllSubscriptionsRead() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);    
    QNetworkReply *reply =
    networkAccessManager(SLOT(onAllSubscriptionsRead(QNetworkReply*)))->get(buildRequest("/subscriptions/read"));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::fetchSubscription(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply =
    networkAccessManager(SLOT(onReplyFinished(QNetworkReply*)))->get(buildRequest("/subscriptions/" + id));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::fetchSubscriptions(int offset, int limit) {
    if (status() == Active) {
        return;
    }
    
    QVariantMap params;
    params["offset"] = offset;
    params["limit"] = limit;
    fetchSubscriptions(params);
}

void DBConnection::fetchSubscriptions(const QStringList &ids) {
    if (status() == Active) {
        return;
    }
    
    QVariantMap params;
    params["id"] = ids.join(",");
    fetchSubscriptions(params);
}

void DBConnection::fetchSubscriptions(const QVariantMap &params) {
    if (status() == Active) {
        return;
    }
    
    QNetworkReply *reply =
    networkAccessManager(SLOT(onReplyFinished(QNetworkReply*)))->get(buildRequest("/subscriptions", params));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::deleteArticle(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply =
    networkAccessManager(SLOT(onArticleDeleted(QNetworkReply*)))->deleteResource(buildRequest("/articles/" + id,
                                                                                 QNetworkAccessManager::DeleteOperation));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::deleteReadArticles(int expiryDate) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QVariantMap params;
    params["expiry"] = expiryDate;
    QNetworkReply *reply =
    networkAccessManager(SLOT(onReadArticlesDeleted(QNetworkReply*)))->get(buildRequest("/articles/deleteread",
                                                                                        params));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::markArticleFavourite(const QString &id, bool isFavourite) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const QString path = isFavourite ? QString("/articles/favourite") : QString("/articles/unfavourite");
    QVariantMap params;
    params["id"] = id;
    QNetworkReply *reply =
    networkAccessManager(SLOT(onArticleFavourited(QNetworkReply*)))->get(buildRequest(path, params));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::markArticleRead(const QString &id, bool isRead) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const QString path = isRead ? QString("/articles/read") : QString("/articles/unread");
    QVariantMap params;
    params["id"] = id;
    QNetworkReply *reply =
    networkAccessManager(SLOT(onArticleRead(QNetworkReply*)))->get(buildRequest(path, params));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::fetchArticle(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply =
    networkAccessManager(SLOT(onReplyFinished(QNetworkReply*)))->get(buildRequest("/articles/" + id));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::fetchArticles(int offset, int limit) {
    if (status() == Active) {
        return;
    }
    
    QVariantMap params;
    params["offset"] = offset;
    params["limit"] = limit;
    fetchArticles(params);
}

void DBConnection::fetchArticles(const QStringList &ids) {
    if (status() == Active) {
        return;
    }
    
    QVariantMap params;
    params["id"] = ids.join(",");
    fetchArticles(params);
}

void DBConnection::fetchArticles(const QVariantMap &params) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    QNetworkReply *reply = networkAccessManager(SLOT(onReplyFinished(QNetworkReply*)))->get(buildRequest("/articles",
                                                                                                         params));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onReplyProgress(qint64, qint64)));
    connect(this, SIGNAL(finished(DBConnection*)), reply, SLOT(deleteLater()));
}

void DBConnection::fetchArticlesForSubscription(const QString &subscriptionId, int offset, int limit) {
    if (status() == Active) {
        return;
    }
    
    QVariantMap params;
    params["subscriptionId"] = subscriptionId;
    params["offset"] = offset;
    params["limit"] = limit;
    fetchArticles(params);
}

void DBConnection::searchArticles(const QString &query, int offset, int limit) {
    if (status() == Active) {
        return;
    }
    
    QVariantMap params;
    params["search"] = query;
    params["offset"] = offset;
    params["limit"] = limit;
    fetchArticles(params);
}

void DBConnection::close() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }
}

QNetworkAccessManager* DBConnection::networkAccessManager(const char *slot) {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }
    else {
        disconnect(m_nam, SIGNAL(finished(QNetworkReply*)), this, 0);
    }
    
    connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, slot);
    return m_nam;
}

void DBConnection::onSubscriptionsAdded(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        emit DBNotify::instance()->subscriptionsAdded(m_result.toList());
    }
}

void DBConnection::onSubscriptionDeleted(QNetworkReply *reply) {
    const QString id = reply->url().path().section("/", -1);
    onReplyFinished(reply);
    
    if (status() == Ready) {
        emit DBNotify::instance()->subscriptionDeleted(id);
    }
}

void DBConnection::onSubscriptionUpdated(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        const QVariantMap subscription = m_result.toMap();
        emit DBNotify::instance()->subscriptionUpdated(subscription.value("id").toString(), subscription);
    }
}

void DBConnection::onSubscriptionRead(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        const QVariantMap subscription = m_result.toMap();
        emit DBNotify::instance()->subscriptionRead(subscription.value("id").toString(),
                                                    subscription.value("unreadArticles", 0).toInt() == 0,
                                                    subscription);
    }
}

void DBConnection::onAllSubscriptionsRead(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        emit DBNotify::instance()->allSubscriptionsRead();
    }
}

void DBConnection::onArticleDeleted(QNetworkReply *reply) {
    const QString id = reply->url().path().section("/", -1);
    onReplyFinished(reply);
    
    if (status() == Ready) {
        emit DBNotify::instance()->articleDeleted(id);
    }
}

void DBConnection::onArticleFavourited(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        const QVariantMap article = m_result.toMap();
        emit DBNotify::instance()->articleFavourited(article.value("id").toString(), article.value("favourite").toBool(),
                                                     article);
    }
}

void DBConnection::onArticleRead(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        const QVariantMap article = m_result.toMap();
        emit DBNotify::instance()->articleRead(article.value("id").toString(),
                                               article.value("subscriptionId").toString(),
                                               article.value("read").toBool(), article);
    }
}

void DBConnection::onReadArticlesDeleted(QNetworkReply *reply) {
    onReplyFinished(reply);
    
    if (status() == Ready) {
        const int count = m_result.toMap().value("count", 0).toInt();
        
        if (count > 0) {
            emit DBNotify::instance()->readArticlesDeleted(count);
        }
    }
}

void DBConnection::onReplyProgress(qint64 current, qint64 total) {
    setProgress(qMax(1, int(current)) * 100 / qMax(1, int(total)));
}

void DBConnection::onReplyFinished(QNetworkReply *reply) {
    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        setErrorString(QString());
        setStatus(Canceled);
        emit finished(this);
        return;
    default:
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        return;
    }
    
    setResult(QtJson::Json::parse(QString::fromUtf8(reply->readAll())));
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}
