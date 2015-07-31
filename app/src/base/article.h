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

#ifndef ARTICLE_H
#define ARTICLE_H

#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QStringList>
#include <QVariantList>

class QSqlQuery;

class Article : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString author READ author NOTIFY authorChanged)
    Q_PROPERTY(QString body READ body NOTIFY bodyChanged)
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QDateTime date READ date NOTIFY dateChanged)
    Q_PROPERTY(QVariantList enclosures READ enclosures NOTIFY enclosuresChanged)
    Q_PROPERTY(bool favourite READ isFavourite NOTIFY favouriteChanged)
    Q_PROPERTY(bool read READ isRead NOTIFY readChanged)
    Q_PROPERTY(int subscriptionId READ subscriptionId NOTIFY subscriptionIdChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)

public:
    explicit Article(QObject *parent = 0);
    explicit Article(const QSqlQuery &query, QObject *parent = 0);
    explicit Article(int id, const QString &author, const QString &body, const QStringList &categories,
                     const QDateTime &date, const QVariantList &enclosures, bool isFavourite, bool isRead,
                     int subscriptionId, const QString &title, const QUrl &url, QObject *parent = 0);
    
    int id() const;
    
    QString author() const;
    
    QString body() const;
    
    QStringList categories() const;
    
    QDateTime date() const;
    
    QVariantList enclosures() const;
    
    bool isFavourite() const;
    
    bool isRead() const;
        
    int subscriptionId() const;
    
    QString title() const;
    
    QUrl url() const;

public Q_SLOTS:
    void load(int id);

private:
    void setId(int i);
    
    void setAuthor(const QString &a);
    
    void setBody(const QString &b);
    
    void setCategories(const QStringList &c);
    
    void setDate(const QDateTime &d);
    
    void setEnclosures(const QVariantList &e);
    
    void setFavourite(bool f);
    
    void setRead(bool r);
    
    void setSubscriptionId(int i);
    
    void setTitle(const QString &t);
    
    void setUrl(const QUrl &u);

private Q_SLOTS:
    void onArticleFetched(const QSqlQuery &query, int requestId);
    void onArticleFavourited(int articleId, bool isFavourite);
    void onArticleRead(int articleId, int subscriptionId, bool isRead);
    void onSubscriptionRead(int subscriptionId, bool isRead);

Q_SIGNALS:
    void idChanged();
    void authorChanged();
    void bodyChanged();
    void categoriesChanged();
    void dataChanged(Article *article);
    void dateChanged();
    void enclosuresChanged();
    void favouriteChanged();
    void readChanged();
    void subscriptionIdChanged();
    void titleChanged();
    void urlChanged();

private:
    int m_id;
    
    QString m_author;
    
    QString m_body;
    
    QStringList m_categories;
    
    QDateTime m_date;
    
    QVariantList m_enclosures;
    
    bool m_favourite;
        
    bool m_read;
    
    int m_subscriptionId;
    
    QString m_title;
    
    QUrl m_url;
};

#endif // ARTICLE_H
