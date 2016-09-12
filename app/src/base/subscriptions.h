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
#include <QQueue>
#include <QTimer>

class DBConnection;
class Download;
class FeedRequest;
class Subscription;
class QProcess;
class QString;

class Subscriptions : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString activeSubscription READ activeSubscription NOTIFY activeSubscriptionChanged)
    Q_PROPERTY(bool offlineModeEnabled READ offlineModeEnabled WRITE setOfflineModeEnabled
               NOTIFY offlineModeEnabledChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(qint64 scheduledUpdatesInterval READ scheduledUpdatesInterval WRITE setScheduledUpdatesInterval
               NOTIFY scheduledUpdatesIntervalChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    
    Q_ENUMS(Status)
#ifdef DBUS_INTERFACE
    Q_CLASSINFO("D-Bus Interface", "org.marxoft.cutenews.subscriptions")
#endif
public:
    enum Status {
        Idle = 0,
        Active,
        Canceled,
        Finished,
        Error
    };
        
    ~Subscriptions();
    
    static Subscriptions* instance();
    
    QString activeSubscription() const;
    
    bool offlineModeEnabled() const;
    
    int progress() const;
    
    int scheduledUpdatesInterval() const;
    
    Status status() const;
    QString statusText() const;

public Q_SLOTS:
    void setOfflineModeEnabled(bool enabled);
    
    void setScheduledUpdatesInterval(int interval);
    
    void cancel();
    
    void create(const QString &source, int sourceType, bool downloadEnclosures = false, int updateInterval = 0);
    bool importFromOpml(const QString &fileName, bool downloadEnclosures = false, int updateInterval = 0);
    
    Q_SCRIPTABLE void update(const QString &id);
    Q_SCRIPTABLE void update(const QStringList &ids);
    Q_SCRIPTABLE bool updateAll();

private Q_SLOTS:
    void getScheduledUpdates();
    
    void onFeedDownloadFinished();
    void onIconDownloadFinished();
    
    void onFeedRequestFinished(FeedRequest *request);
    
    void onProcessError();
    void onProcessFinished(int exitCode);
    
    void onSubscriptionFetched(Subscription *subscription);
    void onSubscriptionIdsFetched(DBConnection *connection);
    
    void onConnectionFinished(DBConnection *connection);

Q_SIGNALS:
    void activeSubscriptionChanged(const QString &subscriptionId);
    void offlineModeEnabledChanged(bool enabled);
    void progressChanged(int progress);
    void scheduledUpdatesIntervalChanged(int interval);
    void statusChanged(Subscriptions::Status s);
    void statusTextChanged(const QString &text);

private:
    Subscriptions();
    
    void setProgress(int p);
    
    void setStatus(Status s);
    void setStatusText(const QString &t);
    
    void next();
    void update();
    
    void downloadIcon(const QString &url);
    void parseXml(const QByteArray &xml);
    
    Download* feedDownloader();
    Download* iconDownloader();
    FeedRequest* feedRequest(const QString &pluginId);
    Subscription* subscription();
    QProcess* process();
    
    static Subscriptions *self;
#ifdef USE_FAVICONS
    static const QString FAVICONS_URL;
#endif
    Download *m_feedDownloader;
    Download *m_iconDownloader;
    FeedRequest *m_feedRequest;
    Subscription *m_subscription;
    QProcess *m_process;
    
    int m_progress;

    QTimer m_updateTimer;
        
    Status m_status;
    QString m_statusText;
    
    QQueue<QString> m_queue;
    
    int m_total;    
};

#endif // SUBSCRIPTIONS_H
