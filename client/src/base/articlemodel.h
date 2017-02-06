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

#ifndef ARTICLEMODEL_H
#define ARTICLEMODEL_H

#include <QAbstractListModel>

class Article;
class DBConnection;

class ArticleModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(QString query READ query NOTIFY queryChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString subscriptionId READ subscriptionId NOTIFY subscriptionIdChanged)
    
    Q_ENUMS(Status)
    
public:
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
    
    explicit ArticleModel(QObject *parent = 0);
    
    QString errorString() const;
    
    int limit() const;
    void setLimit(int l);
    
    QString query() const;
    
    Status status() const;
    
    QString subscriptionId() const;
    
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    void fetchMore(const QModelIndex &parent = QModelIndex());    
    
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE QVariant data(int row, const QByteArray &role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE bool setData(int row, const QVariant &value, const QByteArray &role);
    
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QVariantMap itemData(int row) const;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Q_INVOKABLE bool setItemData(int row, const QVariantMap &roles);
    
    Q_INVOKABLE Article* get(int row) const;
    Q_INVOKABLE bool remove(int row);
    
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                          Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE int match(int start, const QByteArray &role, const QVariant &value,
                          int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;

public Q_SLOTS:
    void clear();
    void load(const QString &subscriptionId);
    void search(const QString &query);
    void reload();    

private Q_SLOTS:
    void onArticleChanged(Article *article, int role);
    void onArticleDeleted(const QString &articleId);
    void onArticleFavourited(const QString &articleId, bool isFavourite, const QVariantMap &properties);
    void onArticlesFetched(DBConnection *connection);
    void onSubscriptionDeleted(const QString &id);

Q_SIGNALS:
    void countChanged(int count);
    void limitChanged(int limit);
    void queryChanged(const QString &query);
    void statusChanged(ArticleModel::Status status);
    void subscriptionIdChanged(const QString &id);
    
private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);
        
    QList<Article*> m_list;
            
    QString m_errorString;
    
    int m_limit;
    int m_offset;
    bool m_moreResults;
    
    Status m_status;
    
    QString m_subscriptionId;
    
    QString m_query;
};

#endif // ARTICLEMODEL_H
