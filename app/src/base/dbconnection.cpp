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
#include "logger.h"
#include "utils.h"
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QThread>

QThread* DBConnection::asyncThread = 0;

const QString DBConnection::SUBSCRIPTION_FIELDS("subscriptions.id, subscriptions.description, subscriptions.downloadEnclosures, subscriptions.iconPath, subscriptions.lastUpdated, subscriptions.source, subscriptions.sourceType, subscriptions.title, subscriptions.updateInterval, subscriptions.url");
const QString DBConnection::ARTICLE_FIELDS("articles.id, articles.author, articles.body, articles.categories, articles.date, articles.enclosures, articles.isFavourite, articles.isRead, articles.subscriptionId, articles.title, articles.url");

DBConnection::DBConnection(bool asynchronous) :
    QObject(),
    m_asynchronous(asynchronous),
    m_progress(0),
    m_status(Idle)
{
    if ((asynchronous) && (asyncThread)) {
        moveToThread(asyncThread);
    }
}

DBConnection::~DBConnection() {
    close();
}

bool DBConnection::isAsynchronous() const {
    return m_asynchronous;
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
    return new DBConnection(true);
}

DBConnection* DBConnection::connection(QObject *obj, const char *slot) {
    DBConnection *conn = new DBConnection(true);
    QObject::connect(conn, SIGNAL(finished(DBConnection*)), obj, slot);
    return conn;
}

QThread* DBConnection::asynchronousThread() {
    return asyncThread;
}

void DBConnection::setAsynchronousThread(QThread *thread) {
    asyncThread = thread;
}

void DBConnection::addSubscription(const QVariantList &properties) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_addSubscription", connType, Q_ARG(QVariantList, properties));
}

void DBConnection::addSubscriptions(const QList<QVariantList> &subscriptions) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_addSubscriptions", connType, Q_ARG(QList<QVariantList>, subscriptions));
}

void DBConnection::deleteSubscription(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_deleteSubscription", connType, Q_ARG(QString, id));
}

void DBConnection::updateSubscription(const QString &id, const QVariantMap &properties, bool fetchResult) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_updateSubscription", connType, Q_ARG(QString, id),
                              Q_ARG(QVariantMap, properties), Q_ARG(bool, fetchResult));
}

void DBConnection::markSubscriptionRead(const QString &id, bool isRead, bool fetchResult) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_markSubscriptionRead", connType, Q_ARG(QString, id), Q_ARG(bool, isRead),
                              Q_ARG(bool, fetchResult));
}

void DBConnection::markAllSubscriptionsRead() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_markAllSubscriptionsRead", connType);
}

void DBConnection::fetchSubscription(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchSubscription", connType, Q_ARG(QString, id));
}

void DBConnection::fetchSubscriptions() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchSubscriptions", connType);
}

void DBConnection::fetchSubscriptions(const QStringList &ids) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchSubscriptions", connType, Q_ARG(QStringList, ids));
}

void DBConnection::fetchSubscriptions(const QString &criteria) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchSubscriptions", connType, Q_ARG(QString, criteria));
}

void DBConnection::addArticle(const QVariantList &properties, const QString &subscriptionId) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_addArticle", connType, Q_ARG(QVariantList, properties),
                              Q_ARG(QString, subscriptionId));
}

void DBConnection::addArticles(const QList<QVariantList> &articles, const QString &subscriptionId) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_addArticles", connType, Q_ARG(QList<QVariantList>, articles),
                              Q_ARG(QString, subscriptionId));
}

void DBConnection::deleteArticle(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_deleteArticle", connType, Q_ARG(QString, id));
}

void DBConnection::deleteReadArticles(int expiryDate) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_deleteReadArticles", connType, Q_ARG(int, expiryDate));
}

void DBConnection::updateArticle(const QString &id, const QVariantMap &properties, bool fetchResult) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_updateArticle", connType, Q_ARG(QString, id), Q_ARG(QVariantMap, properties),
                              Q_ARG(bool, fetchResult));
}

void DBConnection::markArticleFavourite(const QString &id, bool isFavourite, bool fetchResult) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_markArticleFavourite", connType, Q_ARG(QString, id), Q_ARG(bool, isFavourite),
                              Q_ARG(bool, fetchResult));
}

void DBConnection::markArticleRead(const QString &id, bool isRead, bool fetchResult) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_markArticleRead", connType, Q_ARG(QString, id), Q_ARG(bool, isRead),
                              Q_ARG(bool, fetchResult));
}

void DBConnection::fetchArticle(const QString &id) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchArticle", connType, Q_ARG(QString, id));
}

void DBConnection::fetchArticles() {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchArticles", connType);
}

void DBConnection::fetchArticles(const QStringList &ids) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchArticles", connType, Q_ARG(QStringList, ids));
}

void DBConnection::fetchArticles(const QString &criteria) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_fetchArticles", connType, Q_ARG(QString, criteria));
}

void DBConnection::exec(const QString &statement) {
    if (status() == Active) {
        return;
    }
    
    setStatus(Active);
    const Qt::ConnectionType connType = isAsynchronous() ? Qt::QueuedConnection : Qt::DirectConnection;
    QMetaObject::invokeMethod(this, "_p_exec", connType, Q_ARG(QString, statement));
}

void DBConnection::clear() {
    if (status() != Active) {
        m_query.clear();
    }
}

void DBConnection::close() {
    if ((!m_connectionName.isEmpty()) && (status() != Active)) {
        QSqlDatabase::removeDatabase(m_connectionName);
        m_connectionName.clear();
    }
}

bool DBConnection::nextRecord() {
    if (status() == Ready) {
        return m_query.next();
    }
    
    return false;
}

QVariant DBConnection::value(int index) const {
    if (status() == Ready) {
        return m_query.value(index);
    }
    
    return QVariant();
}

QVariant DBConnection::value(const QString &name) const {
    if (status() == Ready) {
        return m_query.record().value(name);
    }
    
    return QVariant();
}

void DBConnection::_p_addSubscription(const QVariantList &properties) {
    m_query = QSqlQuery(database());
    m_query.prepare("INSERT INTO subscriptions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (const QVariant &property, properties) {
        m_query.addBindValue(property);
    }
    
    if (m_query.exec()) {
        setErrorString(QString());
        setStatus(Ready);
        emit DBNotify::instance()->subscriptionsAdded(QStringList() << properties.first().toString());
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_addSubscriptions(const QList<QVariantList> &subscriptions) {
    m_query = QSqlQuery(database());
    m_query.prepare("INSERT INTO subscriptions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (const QVariantList &subscription, subscriptions) {
        if (!subscription.isEmpty()) {
            m_query.addBindValue(subscription);
        }
    }
    
    if (m_query.execBatch()) {
        QStringList ids;

        foreach (const QVariant &v, subscriptions.first()) {
            ids << v.toString();
        }
        
        setErrorString(QString());
        setStatus(Ready);
        emit DBNotify::instance()->subscriptionsAdded(ids);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_deleteSubscription(const QString &id) {
    Logger::log("DBConnection::_p_deleteSubscription(). ID: " + id, Logger::MediumVerbosity);
    m_query = QSqlQuery(database());
#ifdef NO_SQLITE_FOREIGN_KEYS
    m_query.prepare("DELETE FROM articles WHERE subscriptionId = ?");
    m_query.addBindValue(id);
    
    if (!m_query.exec()) {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
        emit finished(this);
        return;
    }
#endif
    m_query.prepare("DELETE FROM subscriptions WHERE id = ?");
    m_query.addBindValue(id);
    
    if (m_query.exec()) {
        Utils::removeDirectory(CACHE_PATH + id);
        setErrorString(QString());
        setStatus(Ready);
        emit DBNotify::instance()->subscriptionDeleted(id);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_updateSubscription(const QString &id, const QVariantMap &properties, bool fetchResult) {
    Logger::log("DBConnection::_p_updateSubscription(). ID: " + id, Logger::MediumVerbosity);
    QString statement = QString("UPDATE subscriptions SET %1 = ? WHERE id = '%2'")
                               .arg(QStringList(properties.keys()).join(" = ?, "))
                               .arg(id);
    m_query = QSqlQuery(database());
    m_query.prepare(statement);
    
    foreach (const QVariant &value, properties.values()) {
        m_query.addBindValue(value);
    }
    
    if (m_query.exec()) {        
        if (fetchResult) {
            statement = QString("SELECT %1 FROM subscriptions WHERE id = '%2'").arg(SUBSCRIPTION_FIELDS).arg(id);
            
            if ((m_query.exec(statement)) && (m_query.next())) {
                setErrorString(QString());
                setStatus(Ready);
            }
            else {
                setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery())
                                                                     .arg(m_query.lastError().text()));
                setStatus(Error);
            }
        }
        else {
            setErrorString(QString());
            setStatus(Ready);
        }
        
        emit DBNotify::instance()->subscriptionUpdated(id);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_markSubscriptionRead(const QString &id, bool isRead, bool fetchResult) {
    Logger::log("DBConnection::_p_markSubscriptionRead(). ID: " + id
                + (isRead ? ", Status: unread" : ", Status: read"), Logger::MediumVerbosity);
    QString statement("UPDATE articles SET isRead = ?, lastRead = ? WHERE subscriptionId = ? AND isRead = ?");
    m_query = QSqlQuery(database());
    m_query.prepare(statement);
    m_query.addBindValue(isRead ? 1 : 0);
    m_query.addBindValue(isRead ? QDateTime::currentDateTime().toTime_t() : 0);
    m_query.addBindValue(id);
    m_query.addBindValue(isRead ? 0 : 1);
    
    if (m_query.exec()) {
        if (fetchResult) {
            statement = QString("SELECT %1 FROM subscriptions WHERE id = '%2'").arg(SUBSCRIPTION_FIELDS).arg(id);
            
            if ((m_query.exec(statement)) && (m_query.next())) {
                setErrorString(QString());
                setStatus(Ready);
            }
            else {
                setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery())
                                                                     .arg(m_query.lastError().text()));
                setStatus(Error);
            }
        }
        else {
            setErrorString(QString());
            setStatus(Ready);
        }
        
        emit DBNotify::instance()->subscriptionRead(id, isRead);
    }
    else {    
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_markAllSubscriptionsRead() {
    Logger::log("DBConnection::_p_markAllSubscriptionsRead()", Logger::MediumVerbosity);
    QString statement("UPDATE articles SET isRead = ?, lastRead = ? WHERE isRead = ?");
    m_query = QSqlQuery(database());
    m_query.prepare(statement);
    m_query.addBindValue(1);
    m_query.addBindValue(QDateTime::currentDateTime().toTime_t());
    m_query.addBindValue(0);
    
    if (m_query.exec()) {
        setErrorString(QString());
        setStatus(Ready);
        
        emit DBNotify::instance()->allSubscriptionsRead();
    }
    else {    
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_fetchSubscription(const QString &id) {
    m_query = QSqlQuery(database());
    m_query.prepare(QString("SELECT %1, COUNT(articles.id) FROM subscriptions LEFT JOIN articles ON \
    subscriptions.id = articles.subscriptionId AND articles.isRead = 0 WHERE subscriptions.id = ? \
    GROUP BY subscriptions.id").arg(SUBSCRIPTION_FIELDS));
    m_query.addBindValue(id);
    
    if ((m_query.exec()) && (m_query.next())) {
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_fetchSubscriptions() {
    _p_exec(QString("SELECT %1, COUNT(articles.id) FROM subscriptions LEFT JOIN articles ON \
    subscriptions.id = articles.subscriptionId AND articles.isRead = 0 GROUP BY subscriptions.id \
    ORDER BY subscriptions.rowid ASC").arg(SUBSCRIPTION_FIELDS));
}

void DBConnection::_p_fetchSubscriptions(const QStringList &ids) {
    _p_exec(QString("SELECT %1, COUNT(articles.id) FROM subscriptions LEFT JOIN articles \
    ON subscriptions.id = articles.subscriptionId AND articles.isRead = 0 WHERE subscriptions.id = '%2' \
    GROUP BY subscriptions.id ORDER BY subscriptions.rowid ASC").arg(SUBSCRIPTION_FIELDS)
                                                                .arg(ids.join("' OR subscriptions.id = ")));
}

void DBConnection::_p_fetchSubscriptions(const QString &criteria) {
    int pos = criteria.indexOf("ORDER");

    if (pos == -1) {
        pos = criteria.indexOf("LIMIT");
    }
    
    QString statement = QString("SELECT %1, COUNT(articles.id) FROM subscriptions LEFT JOIN articles \
    ON subscriptions.id = articles.subscriptionId AND articles.isRead = 0 %2 GROUP BY subscriptions.id")
    .arg(SUBSCRIPTION_FIELDS).arg(criteria.left(pos));

    if (pos != -1) {
        statement.append(" ");
        statement.append(criteria.mid(pos));
    }

    _p_exec(statement);
}

void DBConnection::_p_addArticle(const QVariantList &properties, const QString &subscriptionId) {
    m_query = QSqlQuery(database());
    m_query.prepare("INSERT INTO articles VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (const QVariant &property, properties) {
        m_query.addBindValue(property);
    }
    
    if (m_query.exec()) {
        setErrorString(QString());
        setStatus(Ready);
        emit DBNotify::instance()->articlesAdded(QStringList() << properties.first().toString(), subscriptionId);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_addArticles(const QList<QVariantList> &articles, const QString &subscriptionId) {
    m_query = QSqlQuery(database());
    m_query.prepare("INSERT INTO articles VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (const QVariantList &article, articles) {
        if (!article.isEmpty()) {
            m_query.addBindValue(article);
        }
    }
    
    if (m_query.execBatch()) {
        QStringList ids;
        
        foreach (const QVariant &v, articles.first()) {
            ids << v.toString();
        }

        setErrorString(QString());
        setStatus(Ready);
        emit DBNotify::instance()->articlesAdded(ids, subscriptionId);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_deleteArticle(const QString &id) {
    Logger::log("DBConnection::_p_deleteArticle(). ID: " + id, Logger::MediumVerbosity);
    m_query = QSqlQuery(database());
    m_query.prepare("SELECT subscriptionId FROM articles WHERE id = ?");
    m_query.addBindValue(id);
    
    if ((m_query.exec()) && (m_query.next())) {
        const QString &subscriptionId = m_query.value(0).toString();
        m_query.prepare("DELETE FROM articles WHERE id = ?");
        m_query.addBindValue(id);
        
        if (m_query.exec()) {
            Utils::removeDirectory(QString("%1%2/%3").arg(CACHE_PATH).arg(subscriptionId).arg(id));
            setErrorString(QString());
            setStatus(Ready);
            emit finished(this);
            emit DBNotify::instance()->articlesDeleted(QStringList() << id, subscriptionId);
            return;
        }
    }
    
    setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
    setStatus(Error);
    emit finished(this);
}

void DBConnection::_p_deleteReadArticles(int expiryDate) {
    Logger::log("DBConnection::_p_deleteReadArticles(). Expiry date: " + QString::number(expiryDate),
                Logger::MediumVerbosity);
    m_query = QSqlQuery(database());
    m_query.prepare("SELECT id, subscriptionId FROM articles WHERE isRead = ? AND isFavourite = 0 AND lastRead < ?");
    m_query.addBindValue(1);
    m_query.addBindValue(expiryDate);
    
    if (m_query.exec()) {
        QStringList articleIds;
        QStringList subscriptionIds;
        
        while (m_query.next()) {
            articleIds << m_query.value(0).toString();
            subscriptionIds << m_query.value(1).toString();
        }
        
        if (!articleIds.isEmpty()) {
            m_query.prepare("DELETE FROM articles WHERE isRead = ? AND isFavourite = 0 AND lastRead < ?");
            m_query.addBindValue(1);
            m_query.addBindValue(expiryDate);
            const int p = 100 / articleIds.size();
            
            if (m_query.exec()) {
                for (int i = 0; i < articleIds.size(); i++) {
                    Utils::removeDirectory(QString("%1%2/%3").arg(CACHE_PATH).arg(subscriptionIds.at(i))
                                                             .arg(articleIds.at(i)));
                    setProgress(progress() + p);
                }
                
                Logger::log(QString("DBConnection::_p_deleteReadArticles(). %1 articles deleted")
                            .arg(articleIds.size()), Logger::LowVerbosity);
                setErrorString(QString());
                setStatus(Ready);
                emit DBNotify::instance()->readArticlesDeleted(articleIds.size());
            }
            else {
                setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery())
                                                                     .arg(m_query.lastError().text()));
                setStatus(Error);
            }
        }
        else {
            Logger::log("DBConnection::_p_deleteReadArticles(). No articles deleted", Logger::LowVerbosity);
            setErrorString(QString());
            setStatus(Ready);
        }
    }
    else {    
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_updateArticle(const QString &id, const QVariantMap &properties, bool fetchResult) {
    Logger::log("DBConnection::_p_updateArticle(). ID: " + id, Logger::MediumVerbosity);
    QString statement = QString("UPDATE articles SET %1 = ? WHERE id = '%2'")
                               .arg(QStringList(properties.keys()).join(" = ?, "))
                               .arg(id);
    m_query = QSqlQuery(database());
    m_query.prepare(statement);
    
    foreach (const QVariant &value, properties.values()) {
        m_query.addBindValue(value);
    }
    
    if (m_query.exec()) {        
        if (fetchResult) {
            statement = QString("SELECT %1 FROM articles WHERE id = '%2'").arg(ARTICLE_FIELDS).arg(id);
            
            if ((m_query.exec(statement)) && (m_query.next())) {
                setErrorString(QString());
                setStatus(Ready);
            }
            else {
                setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery())
                                                                     .arg(m_query.lastError().text()));
                setStatus(Error);
            }
        }
        else {
            setErrorString(QString());
            setStatus(Ready);
        }
        
        emit DBNotify::instance()->articleUpdated(id);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_markArticleFavourite(const QString &id, bool isFavourite, bool fetchResult) {
    Logger::log("DBConnection::_p_markArticleFavourite(). ID: " + id
                + (isFavourite ? ", Status: favourite" : ", Status: unfavourite"), Logger::MediumVerbosity);
    QString statement("UPDATE articles SET isFavourite = ? WHERE id = ?");
    m_query = QSqlQuery(database());
    m_query.prepare(statement);
    m_query.addBindValue(isFavourite ? 1 : 0);
    m_query.addBindValue(id);
    
    if (m_query.exec()) {        
        if (fetchResult) {
            statement = QString("SELECT %1 FROM articles WHERE id = '%2'").arg(ARTICLE_FIELDS).arg(id);
            
            if ((m_query.exec(statement)) && (m_query.next())) {
                setErrorString(QString());
                setStatus(Ready);
            }
            else {
                setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery())
                                                                     .arg(m_query.lastError().text()));
                setStatus(Error);
            }
        }
        else {
            setErrorString(QString());
            setStatus(Ready);
        }
        
        emit DBNotify::instance()->articleFavourited(id, isFavourite);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_markArticleRead(const QString &id, bool isRead, bool fetchResult) {
    Logger::log("DBConnection::_p_markArticleRead(). ID: " + id
                + (isRead ? ", Status: read" : ", Status: unread"), Logger::MediumVerbosity);
    QString statement("UPDATE articles SET isRead = ?, lastRead = ? WHERE id = ?");
    m_query = QSqlQuery(database());
    m_query.prepare(statement);
    m_query.addBindValue(isRead ? 1 : 0);
    m_query.addBindValue(isRead ? QDateTime::currentDateTime().toTime_t() : 0);
    m_query.addBindValue(id);
    
    if (m_query.exec()) {
        if (fetchResult) {
            statement = QString("SELECT %1 FROM articles WHERE id = '%2'").arg(ARTICLE_FIELDS).arg(id);
        }
        else {
            statement = QString("SELECT subscriptionId FROM articles WHERE id = '%1'").arg(id);
        }
        
        if ((m_query.exec(statement)) && (m_query.next())) {
            const QString subscriptionId = m_query.record().value("subscriptionId").toString();
            setErrorString(QString());
            setStatus(Ready);
            emit DBNotify::instance()->articleRead(id, subscriptionId, isRead);
        }
        else {
            setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery())
                                                                 .arg(m_query.lastError().text()));
            setStatus(Error);
        }
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
};

void DBConnection::_p_fetchArticle(const QString &id) {
    m_query = QSqlQuery(database());
    m_query.prepare(QString("SELECT %1 FROM articles WHERE id = ?").arg(ARTICLE_FIELDS));
    m_query.addBindValue(id);
    
    if ((m_query.exec()) && (m_query.next())) {
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

void DBConnection::_p_fetchArticles() {
    _p_exec(QString("SELECT %1 FROM articles").arg(ARTICLE_FIELDS));
}

void DBConnection::_p_fetchArticles(const QStringList &ids) {
    _p_fetchArticles(QString("WHERE id = '%1'").arg(ids.join("' OR id = '")));
}

void DBConnection::_p_fetchArticles(const QString &criteria) {
    _p_exec(QString("SELECT %1 FROM articles %2").arg(ARTICLE_FIELDS).arg(criteria));
}

void DBConnection::_p_exec(const QString &statement) {
    m_query = QSqlQuery(database());
    
    if (m_query.exec(statement)) {
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(tr("Error executing query \"%1\": %2").arg(m_query.lastQuery()).arg(m_query.lastError().text()));
        setStatus(Error);
    }
    
    emit finished(this);
}

QSqlDatabase DBConnection::database() {
    if (m_connectionName.isEmpty()) {
        m_connectionName = Utils::createId();
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        db.setDatabaseName(DATABASE_NAME);
#ifndef NO_SQLITE_FOREIGN_KEYS
        if (!db.open()) {
            db.open();
        }
        
        db.exec("PRAGMA foreign_keys = ON");
#endif
    }

    return QSqlDatabase::database(m_connectionName);
}
