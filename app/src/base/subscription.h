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

#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QVariant>

class DBConnection;

class Subscription : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(bool downloadEnclosures READ downloadEnclosures NOTIFY downloadEnclosuresChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(QString iconPath READ iconPath NOTIFY iconPathChanged)
    Q_PROPERTY(QDateTime lastUpdated READ lastUpdated NOTIFY lastUpdatedChanged)
    Q_PROPERTY(QString lastUpdatedString READ lastUpdatedString NOTIFY lastUpdatedChanged)
    Q_PROPERTY(bool read READ isRead NOTIFY unreadArticlesChanged)
    Q_PROPERTY(QVariant source READ source NOTIFY sourceChanged)
    Q_PROPERTY(SourceType sourceType READ sourceType NOTIFY sourceTypeChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(int updateInterval READ updateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(int unreadArticles READ unreadArticles NOTIFY unreadArticlesChanged)
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    
    Q_ENUMS(DataRole SourceType Status)

public:
    enum DataRole {
        AutoUpdateRole = Qt::UserRole + 1,
        DescriptionRole,
        DownloadEnclosuresRole,
        ErrorStringRole,
        IconPathRole,
        IdRole,
        LastUpdatedRole,
        LastUpdatedStringRole,
        ReadRole,
        SourceRole,
        SourceTypeRole,
        StatusRole,
        TitleRole,
        UnreadArticlesRole,
        UpdateIntervalRole,
        UrlRole
    };
    
    enum SourceType {
        Url = 0,
        LocalFile,
        Command,
        Plugin,
        None
    };
    
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
    
    explicit Subscription(QObject *parent = 0);
    explicit Subscription(const QString &id, const QString &description, bool downloadEnclosures,
                          const QString &iconPath, const QDateTime &lastUpdated, const QVariant &source,
                          SourceType sourceType, const QString &title, int updateInterval, const QString &url,
                          int unreadArticles, QObject *parent = 0);

    Q_INVOKABLE QVariant data(int role) const;
    Q_INVOKABLE bool setData(int role, const QVariant &value);
    
    QString id() const;
        
    QString description() const;
    
    bool downloadEnclosures() const;
    
    QString errorString() const;
        
    QString iconPath() const;
    
    QDateTime lastUpdated() const;
    QString lastUpdatedString() const;
    
    bool isRead() const;
    
    QVariant source() const;
    SourceType sourceType() const;
    
    Status status() const;
    
    QString title() const;
        
    int updateInterval() const;
    
    QString url() const;
    
    int unreadArticles() const;

    bool autoUpdate() const;
    void setAutoUpdate(bool enabled);

public Q_SLOTS:
    void load(const QString &id);
    void markRead(bool read = true);
    void remove();
    void update(const QVariantMap &properties);

private Q_SLOTS:
    void onArticlesAdded(const QStringList &articleIds, const QString &subscriptionId);
    void onArticlesDeleted(const QStringList &articleIds, const QString &subscriptionId);
    void onArticleRead(const QString &articleId, const QString &subscriptionId, bool isRead);
    void onSubscriptionFetched(DBConnection *connection);
    void onSubscriptionRead(const QString &subscriptionId, bool isRead);
    void onAllSubscriptionsRead();
    void onSubscriptionUpdated(const QString &subscriptionId);

Q_SIGNALS:
    void idChanged();
    void cacheSizeChanged();
    void descriptionChanged();
    void dataChanged(Subscription *subscription, int role);
    void downloadEnclosuresChanged();
    void finished(Subscription *subscription);
    void iconPathChanged();
    void lastUpdatedChanged();
    void readChanged();
    void sourceChanged();
    void sourceTypeChanged();
    void statusChanged();
    void titleChanged();
    void updateIntervalChanged();
    void urlChanged();
    void unreadArticlesChanged();
    void autoUpdateChanged();

private:
    void setId(const QString &i);
        
    void setDescription(const QString &d);
    
    void setDownloadEnclosures(bool d);
    
    void setErrorString(const QString &e);
    
    void setIconPath(const QString &p);
    
    void setLastUpdated(const QDateTime &d);
    
    void setSource(const QVariant &s);
    void setSourceType(SourceType t);
    
    void setStatus(Status s);
    
    void setTitle(const QString &t);
        
    void setUpdateInterval(int i);
    
    void setUrl(const QString &u);
    
    void setUnreadArticles(int u);
    
    QString m_id;
           
    QString m_description;
    
    bool m_downloadEnclosures;
    
    QString m_errorString;
        
    QString m_iconPath;
    
    QDateTime m_lastUpdated;
        
    QVariant m_source;
    
    SourceType m_sourceType;
    
    Status m_status;

    QString m_title;
        
    int m_updateInterval;
    
    QString m_url;
    
    int m_unreadArticles;

    bool m_autoUpdate;
};

#endif // SUBSCRIPTION_H
