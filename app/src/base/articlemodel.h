/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef ARTICLEMODEL_H
#define ARTICLEMODEL_H

#include <QAbstractListModel>
#include <QSqlQuery>

class Article;

class ArticleModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)
    
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AuthorRole,
        BodyRole,
        CategoriesRole,
        DateRole,
        EnclosuresRole,
        FavouriteRole,
        ReadRole,
        SubscriptionIdRole,
        TitleRole,
        UrlRole
    };
    
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
    
    explicit ArticleModel(QObject *parent = 0);
    
    QString errorString() const;
    
    Status status() const;
    
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    
    Q_INVOKABLE Article* get(int row) const;
    
    Q_INVOKABLE int match(const QByteArray &role, const QVariant &value) const;

public Q_SLOTS:
    void clear();
    void load(int subscriptionId);
    void search(const QString &query);
    void reload();

private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);

private Q_SLOTS:
    void onArticleChanged(Article *article);
    void onArticlesAdded(int count, int subscriptionId);
    void onArticleDeleted(int articleId, int subscriptionId);
    void onArticleFavourited(int articleId, bool isFavourite);
    void onArticlesFetched(QSqlQuery query, int requestId);
    void onSubscriptionDeleted(int id);

Q_SIGNALS:
    void countChanged(int count);
    void statusChanged(Status status);
    
private:
    QList<Article*> m_list;
    
    QHash<int, QByteArray> m_roles;
    
    QString m_errorString;
    
    Status m_status;
    
    int m_subscriptionId;
    
    QString m_query;
};

#endif // ARTICLEMODEL_H
