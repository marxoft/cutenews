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

#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

#include <QObject>
#include <QTimer>

class DBConnection;
class QNetworkAccessManager;
class QNetworkReply;

class Subscriptions : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString activeSubscription READ activeSubscription NOTIFY activeSubscriptionChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    
    Q_ENUMS(Status StatusInterval)

public:
    enum Status {
        Idle = 0,
        Active,
        Canceled,
        Finished,
        Error
    };
    
    enum StatusInterval {
        DefaultStatusInterval = 3000,
        LongStatusInterval = 10000
    };
        
    ~Subscriptions();
    
    static Subscriptions* instance();
    
    QString activeSubscription() const;
    
    int progress() const;
    
    Status status() const;
    QString statusText() const;

public Q_SLOTS:            
    void getStatus(int interval);
    
    void create(const QString &source, int sourceType, bool downloadEnclosures = false, int updateInterval = 0);
    
    bool importFromOpml(const QString &fileName, bool downloadEnclosures = false, int updateInterval = 0);
    
    void update(const QString &id);
    void update(const QStringList &ids);
    bool updateAll();
    
    void cancel();

private Q_SLOTS:
    void getStatus();
    void checkStatus(QNetworkReply *reply);
    
    void onConnectionFinished(DBConnection *connection);

Q_SIGNALS:
    void activeSubscriptionChanged(const QString &subscriptionId);
    void progressChanged(int progress);
    void statusChanged(Subscriptions::Status s);
    void statusTextChanged(const QString &text);

private:
    Subscriptions();
    
    void setActiveSubscription(const QString &id);
    
    void setProgress(int p);
    
    void setStatus(Status s);
    void setStatusText(const QString &t);
        
    QNetworkAccessManager* networkAccessManager(const char *slot);
    
    static Subscriptions *self;
    
    QNetworkAccessManager *m_nam;
    
    QString m_activeSubscription;
    
    int m_progress;    
        
    Status m_status;
    QString m_statusText;
    QTimer m_statusTimer;
};

#endif // SUBSCRIPTIONS_H
