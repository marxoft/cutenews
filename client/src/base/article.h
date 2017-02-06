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

#ifndef ARTICLE_H
#define ARTICLE_H

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QVariantList>

class DBConnection;

class Article : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    Q_PROPERTY(QString author READ author NOTIFY authorChanged)
    Q_PROPERTY(QString body READ body NOTIFY bodyChanged)
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QDateTime date READ date NOTIFY dateChanged)
    Q_PROPERTY(QString dateString READ dateString NOTIFY dateChanged)
    Q_PROPERTY(QVariantList enclosures READ enclosures NOTIFY enclosuresChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(bool hasEnclosures READ hasEnclosures NOTIFY enclosuresChanged)
    Q_PROPERTY(bool favourite READ isFavourite NOTIFY favouriteChanged)
    Q_PROPERTY(bool read READ isRead NOTIFY readChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString subscriptionId READ subscriptionId NOTIFY subscriptionIdChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    
    Q_ENUMS(DataRole Status)

public:
    enum DataRole {
        AuthorRole = Qt::UserRole + 1,
        AutoUpdateRole,
        BodyRole,
        CategoriesRole,
        DateRole,
        DateStringRole,
        EnclosuresRole,
        ErrorStringRole,
        FavouriteRole,
        HasEnclosuresRole,
        IdRole,
        ReadRole,
        StatusRole,
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
    
    explicit Article(QObject *parent = 0);
    explicit Article(const QString &id, const QString &author, const QString &body, const QStringList &categories,
                     const QDateTime &date, const QVariantList &enclosures, bool isFavourite, bool isRead,
                     const QString &subscriptionId, const QString &title, const QString &url, QObject *parent = 0);

    static QHash<int, QByteArray> roleNames();

    Q_INVOKABLE QVariant data(int role) const;
    Q_INVOKABLE QVariant data(const QByteArray &roleName) const;
    Q_INVOKABLE bool setData(int role, const QVariant &value);
    Q_INVOKABLE bool setData(const QByteArray &roleName, const QVariant &value);
    
    QString id() const;
    
    QString author() const;
    
    QString body() const;
    
    QStringList categories() const;
    
    QDateTime date() const;
    QString dateString() const;
    
    QVariantList enclosures() const;
    
    QString errorString() const;
    
    bool hasEnclosures() const;
    
    bool isFavourite() const;
    
    bool isRead() const;
    
    Status status() const;
        
    QString subscriptionId() const;
    
    QString title() const;
    
    QString url() const;

    bool autoUpdate() const;
    void setAutoUpdate(bool enabled);

public Q_SLOTS:
    void load(const QString &id);
    void markFavourite(bool favourite = true);
    void markRead(bool read = true);
    void remove();

private Q_SLOTS:
    void onArticleFetched(DBConnection *connection);
    void onArticleFavourited(const QString &articleId, bool isFavourite, const QVariantMap &properties);
    void onArticleRead(const QString &articleId, const QString &subscriptionId, bool isRead,
                       const QVariantMap &properties);
    void onSubscriptionRead(const QString &subscriptionId, bool isRead);
    void onAllSubscriptionsRead();

Q_SIGNALS:
    void idChanged();
    void authorChanged();
    void bodyChanged();
    void categoriesChanged();
    void dataChanged(Article *article, int role);
    void dateChanged();
    void enclosuresChanged();
    void favouriteChanged();
    void finished(Article *article);
    void readChanged();
    void statusChanged();
    void subscriptionIdChanged();
    void titleChanged();
    void urlChanged();
    void autoUpdateChanged();

private:
    static QHash<int, QByteArray> roles;
    
    void setId(const QString &i);
    
    void setAuthor(const QString &a);
    
    void setBody(const QString &b);
    
    void setCategories(const QStringList &c);
    
    void setDate(const QDateTime &d);
    
    void setEnclosures(const QVariantList &e);
    
    void setErrorString(const QString &e);
    
    void setFavourite(bool f);
    
    void setRead(bool r);
    
    void setStatus(Status s);
    
    void setSubscriptionId(const QString &i);
    
    void setTitle(const QString &t);
    
    void setUrl(const QString &u);
    
    void load(const QVariantMap &properties);
    
    QString m_id;
    
    QString m_author;
    
    QString m_body;
    
    QStringList m_categories;
    
    QDateTime m_date;
    
    QVariantList m_enclosures;
    
    QString m_errorString;
    
    bool m_favourite;
        
    bool m_read;
    
    Status m_status;
    
    QString m_subscriptionId;
    
    QString m_title;
    
    QString m_url;

    bool m_autoUpdate;
};

#endif // ARTICLE_H
