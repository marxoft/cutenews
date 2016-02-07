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

#include "database.h"
#include "definitions.h"
#include "json.h"
#include "subscription.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDateTime>
#include <QUrl>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

static QSqlDatabase getDatabase() {
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen()) {
        db.open();
    }
    
    return db;
}

Database* Database::self = 0;

Database::Database() :
    QObject()
{
}

Database::~Database() {
    self = 0;
}

Database* Database::instance() {
    return self ? self : self = new Database;
}

int Database::subscriptionId(const QSqlQuery &query) {
    return query.value(0).toInt();
}

int Database::subscriptionCacheSize(const QSqlQuery &query) {
    return query.value(1).toInt();
}

QString Database::subscriptionDescription(const QSqlQuery &query) {
    return query.value(2).toString();
}

bool Database::subscriptionDownloadEnclosures(const QSqlQuery &query) {
    return query.value(3).toBool();
}

QString Database::subscriptionIconPath(const QSqlQuery &query) {
    return query.value(4).toString();
}

QDateTime Database::subscriptionLastUpdated(const QSqlQuery &query) {
    return query.value(5).toDateTime();
}

QVariant Database::subscriptionSource(const QSqlQuery &query) {
    return subscriptionSourceType(query) == Subscription::Plugin ? QtJson::Json::parse(query.value(6).toString())
                                                                 : query.value(6).toString();
}

int Database::subscriptionSourceType(const QSqlQuery &query) {
    return query.value(7).toInt();
}

QString Database::subscriptionTitle(const QSqlQuery &query) {
    return query.value(8).toString();
}

int Database::subscriptionUpdateInterval(const QSqlQuery &query) {
    return query.value(9).toInt();
}

QUrl Database::subscriptionUrl(const QSqlQuery &query) {
    return query.value(10).toString();
}

int Database::subscriptionUnreadArticles(const QSqlQuery &query) {
    return query.value(11).toInt();
}

int Database::articleId(const QSqlQuery &query) {
    return query.value(0).toInt();
}

QString Database::articleAuthor(const QSqlQuery &query) {
    return query.value(1).toString();
}

QString Database::articleBody(const QSqlQuery &query) {
    return query.value(2).toString();
}

QStringList Database::articleCategories(const QSqlQuery &query) {
    return query.value(3).toStringList();
}

QDateTime Database::articleDate(const QSqlQuery &query) {
    return query.value(4).toDateTime();
}

QVariantList Database::articleEnclosures(const QSqlQuery &query) {
    return QtJson::Json::parse(query.value(5).toString()).toList();
}

bool Database::articleIsFavourite(const QSqlQuery &query) {
    return query.value(6).toBool();
}

bool Database::articleIsRead(const QSqlQuery &query) {
    return query.value(7).toBool();
}

int Database::articleSubscriptionId(const QSqlQuery &query) {
    return query.value(8).toInt();
}

QString Database::articleTitle(const QSqlQuery &query) {
    return query.value(9).toString();
}

QUrl Database::articleUrl(const QSqlQuery &query) {
    return query.value(10).toString();
}

QString Database::errorString() {
    return getDatabase().lastError().text();
}

bool Database::init() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
#ifdef SYMBIAN_OS
    db.setDatabaseName("cuteNews.db");
#else
    QDir().mkpath(DATABASE_PATH);
    db.setDatabaseName(DATABASE_PATH + "cuteNews.db");
#endif
    if (!db.isOpen()) {
        db.open();
    }
    
    QSqlQuery query = db.exec("CREATE TABLE IF NOT EXISTS subscriptions (id INTEGER PRIMARY KEY NOT NULL, \
    cacheSize INTEGER, description TEXT, downloadEnclosures INTEGER, iconPath TEXT, lastUpdated TEXT, source TEXT, \
    sourceType INTEGER, title TEXT, updateInterval INTEGER, url TEXT)");
    
    if (query.lastError().isValid()) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "Database::init" << query.lastError().text();
#endif
        return false;
    }
    
    query = db.exec("CREATE TABLE IF NOT EXISTS articles (id INTEGER PRIMARY KEY NOT NULL, author TEXT, body TEXT, \
    categories TEXT, date TEXT, enclosures TEXT, isFavourite INTEGER, isRead INTEGER, \
    subscriptionId INTEGER REFERENCES subscriptions(id) ON DELETE CASCADE, title TEXT, url TEXT)");
    
    if (query.lastError().isValid()) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "Database::init" << query.lastError().text();
#endif
        return false;
    }
#ifndef NO_SQLITE_FOREIGN_KEYS
    query = db.exec("PRAGMA foreign_keys = ON");
    
    if (query.lastError().isValid()) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "Database::init" << query.lastError().text();
#endif
        return false;
    }
#endif
    return true;
}

bool Database::addSubscription(const QVariantList &properties) {
    return QMetaObject::invokeMethod(instance(), "_p_addSubscription", Qt::QueuedConnection,
                                     Q_ARG(QVariantList, properties));
}

bool Database::addSubscriptions(const QList<QVariantList> &subscriptions) {
    return QMetaObject::invokeMethod(instance(), "_p_addSubscriptions", Qt::QueuedConnection,
                                     Q_ARG(QList<QVariantList>, subscriptions));
}

bool Database::deleteSubscription(int id) {
    return QMetaObject::invokeMethod(instance(), "_p_deleteSubscription", Qt::QueuedConnection, Q_ARG(int, id));
}

bool Database::updateSubscription(int id, const QVariantMap &properties) {
    return QMetaObject::invokeMethod(instance(), "_p_updateSubscription", Qt::QueuedConnection, Q_ARG(int, id),
                               Q_ARG(QVariantMap, properties));
}

bool Database::markSubscriptionRead(int id, bool isRead) {
    return QMetaObject::invokeMethod(instance(), "_p_markSubscriptionRead", Qt::QueuedConnection, Q_ARG(int, id),
                                     Q_ARG(bool, isRead));
}

bool Database::fetchSubscription(int subscriptionId, int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_fetchSubscription", Qt::QueuedConnection,
                                     Q_ARG(int, subscriptionId), Q_ARG(int, requestId));
}

bool Database::fetchSubscriptions(int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_fetchSubscriptions", Qt::QueuedConnection, Q_ARG(int, requestId));
}

bool Database::fetchSubscriptions(const QString &criteria, int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_fetchSubscriptions", Qt::QueuedConnection,
                                     Q_ARG(QString, criteria), Q_ARG(int, requestId));
}

bool Database::addArticle(const QVariantList &properties, int subscriptionId) {
    return QMetaObject::invokeMethod(instance(), "_p_addArticle", Qt::QueuedConnection, Q_ARG(QVariantList, properties),
                                     Q_ARG(int, subscriptionId));
}

bool Database::addArticles(const QList<QVariantList> &articles, int subscriptionId) {
    return QMetaObject::invokeMethod(instance(), "_p_addArticles", Qt::QueuedConnection,
                                     Q_ARG(QList<QVariantList>, articles), Q_ARG(int, subscriptionId));
}

bool Database::deleteArticle(int id) {
    return QMetaObject::invokeMethod(instance(), "_p_deleteArticle", Qt::QueuedConnection, Q_ARG(int, id));
}

bool Database::markArticleFavourite(int id, bool isFavourite) {
    return QMetaObject::invokeMethod(instance(), "_p_markArticleFavourite", Qt::QueuedConnection, Q_ARG(int, id),
                                     Q_ARG(bool, isFavourite));
}

bool Database::markArticleRead(int id, bool isRead) {
    return QMetaObject::invokeMethod(instance(), "_p_markArticleRead", Qt::QueuedConnection, Q_ARG(int, id),
                                     Q_ARG(bool, isRead));
}

bool Database::fetchArticle(int articleId, int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_fetchArticle", Qt::QueuedConnection, Q_ARG(int, articleId),
                               Q_ARG(int, requestId));
}

bool Database::fetchArticles(int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_fetchArticles", Qt::QueuedConnection, Q_ARG(int, requestId));
}

bool Database::fetchArticles(const QString &criteria, int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_fetchArticles", Qt::QueuedConnection,
                                     Q_ARG(QString, criteria), Q_ARG(int, requestId));
}

bool Database::execQuery(const QString &statement, int requestId) {
    return QMetaObject::invokeMethod(instance(), "_p_execQuery", Qt::QueuedConnection, Q_ARG(QString, statement),
                                     Q_ARG(int, requestId));
}

void Database::_p_addSubscription(const QVariantList &properties) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO subscriptions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (QVariant property, properties) {
        query.addBindValue(property);
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_addSubscription" << query.boundValues();
#endif
    if (query.exec()) {
        emit subscriptionsAdded(1);
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_addSubscriptions(const QList<QVariantList> &subscriptions) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO subscriptions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (QVariantList subscription, subscriptions) {
        query.addBindValue(subscription);
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_addSubscriptions" << query.boundValues();
#endif
    if (query.execBatch()) {
        emit subscriptionsAdded(subscriptions.first().size());
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_deleteSubscription(int id) {
    QSqlQuery query(getDatabase());
#ifdef NO_SQLITE_FOREIGN_KEYS
    query.prepare("DELETE FROM articles WHERE subscriptionId = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        emit error(query.lastError().text());
        return;
    }
#endif
    query.prepare("SELECT iconPath FROM subscriptions WHERE id = ?");
    query.addBindValue(id);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_deleteSubscription" << id;
#endif
    if ((query.exec()) && (query.next())) {        
        const QString iconPath = query.value(0).toString();
        query.prepare("DELETE FROM subscriptions WHERE id = ?");
        query.addBindValue(id);
        
        if (query.exec()) {
            if (!iconPath.isEmpty()) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_deleteSubscription. Deleting icon" << iconPath;
#endif
                QFile::remove(iconPath);
            }
            
            emit subscriptionDeleted(id);
        }
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_updateSubscription(int id, const QVariantMap &properties) {
    QSqlQuery query(getDatabase());
    QString statement = QString("UPDATE subscriptions SET %1 = ? WHERE id = %2")
                               .arg(QStringList(properties.keys()).join(" = ?, "))
                               .arg(id);
    query.prepare(statement);
    
    foreach (QVariant value, properties.values()) {
        query.addBindValue(value);
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_updateSubscription" << statement << query.boundValues();
#endif
    if (query.exec()) {
        emit subscriptionUpdated(id);
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_markSubscriptionRead(int id, bool isRead) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE articles SET isRead = ? WHERE subscriptionId = ? AND isRead = ?");
    query.addBindValue(isRead ? 1 : 0);
    query.addBindValue(id);
    query.addBindValue(isRead ? 0 : 1);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_markSubscriptionRead" << id << isRead;
#endif
    if (query.exec()) {        
        emit subscriptionRead(id, isRead);
    }
    else {    
        emit error(query.lastError().text());
    }
}       

void Database::_p_fetchSubscription(int subscriptionId, int requestId) {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT subscriptions.*, COUNT(articles.id) FROM subscriptions LEFT JOIN articles ON \
    subscriptions.id = articles.subscriptionId AND articles.isRead = 0 WHERE subscriptions.id = ? \
    GROUP BY subscriptions.id");
    query.addBindValue(subscriptionId);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_fetchSubscription" << subscriptionId;
#endif
    query.exec();
    query.next();
    emit subscriptionFetched(query, requestId);
}

void Database::_p_fetchSubscriptions(int requestId) {
    QSqlQuery query(getDatabase());
    query.exec("SELECT subscriptions.*, COUNT(articles.id) FROM subscriptions LEFT JOIN articles ON \
    subscriptions.id = articles.subscriptionId AND articles.isRead = 0 GROUP BY subscriptions.id");
    emit subscriptionsFetched(query, requestId);
}

void Database::_p_fetchSubscriptions(const QString &criteria, int requestId) {
    QSqlQuery query(getDatabase());
    QString statement(QString("SELECT subscriptions.*, COUNT(articles.id) FROM subscriptions LEFT JOIN articles ON \
    subscriptions.id = articles.subscriptionId AND articles.isRead = 0 GROUP BY subscriptions.id %1").arg(criteria));
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_fetchSubscriptions:" << statement;
#endif
    query.exec(statement);
    emit subscriptionsFetched(query, requestId);
}

void Database::_p_addArticle(const QVariantList &properties, int subscriptionId) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO articles VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (QVariant property, properties) {
        query.addBindValue(property);
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_addArticle" << query.boundValues();
#endif
    if (query.exec()) {
        emit articlesAdded(1, subscriptionId);
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_addArticles(const QList<QVariantList> &articles, int subscriptionId) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO articles VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach (QVariantList article, articles) {
        query.addBindValue(article);
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_addArticles" << query.boundValues();
#endif
    if (query.execBatch()) {
        emit articlesAdded(articles.first().size(), subscriptionId);
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_deleteArticle(int id) {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT subscriptionId FROM articles WHERE id = ?");
    query.addBindValue(id);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_deleteArticle" << id;
#endif
    if ((query.exec()) && (query.next())) {
        const int subscriptionId = query.value(0).toInt();
        query.prepare("DELETE FROM articles WHERE id = ?");
        query.addBindValue(id);
        
        if (query.exec()) {
            emit articleDeleted(id, subscriptionId);
            return;
        }
    }
    
    emit error(query.lastError().text());
}

void Database::_p_markArticleFavourite(int id, bool isFavourite) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE articles SET isFavourite = ? WHERE id = ?");
    query.addBindValue(isFavourite ? 1 : 0);
    query.addBindValue(id);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_markArticleFavourite" << id << isFavourite;
#endif
    if (query.exec()) {
        emit articleFavourited(id, isFavourite);
    }
    else {
        emit error(query.lastError().text());
    }
}

void Database::_p_markArticleRead(int id, bool isRead) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE articles SET isRead = ? WHERE id = ?");
    query.addBindValue(isRead ? 1 : 0);
    query.addBindValue(id);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_markArticleRead" << id << isRead;
#endif
    if (query.exec()) {
        query.prepare("SELECT subscriptionId FROM articles WHERE id = ?");
        query.addBindValue(id);
        
        if ((query.exec()) && (query.next())) {
            emit articleRead(id, query.value(0).toInt(), isRead);
            return;
        }
    }
    
    emit error(query.lastError().text());
};

void Database::_p_fetchArticle(int articleId, int requestId) {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM articles WHERE id = ?");
    query.addBindValue(articleId);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_fetchArticle" << articleId;
#endif
    query.exec();
    query.next();
    emit articleFetched(query, requestId);
}

void Database::_p_fetchArticles(int requestId) {
    QSqlQuery query(getDatabase());
    query.exec("SELECT * FROM articles");
    emit articlesFetched(query, requestId);
}

void Database::_p_fetchArticles(const QString &criteria, int requestId) {
    QSqlQuery query(getDatabase());
    QString statement("SELECT * FROM articles " + criteria);
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database::_p_fetchArticles:" << statement;
#endif    
    query.exec(statement);
    emit articlesFetched(query, requestId);
}

void Database::_p_execQuery(const QString &statement, int requestId) {
    QSqlQuery query(getDatabase());
    query.exec(statement);
    emit queryReady(query, requestId);
}
