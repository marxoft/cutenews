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

#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

#include <QObject>
#include <QQueue>
#include <QSqlQuery>

class Transfer;
class QProcess;
class QUrl;

class Subscriptions : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int activeSubscription READ activeSubscription NOTIFY activeSubscriptionChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    
    Q_ENUMS(Status)
    
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
    
    int activeSubscription() const;
    
    int progress() const;
    
    Status status() const;
    QString statusText() const;

public Q_SLOTS:
    void cancel();
    void update(int id);
    
private:
    void setProgress(int p);
    
    void setStatus(Status s);
    void setStatusText(const QString &t);
    
    void next();
    void update();
    
    void downloadIcon(const QUrl &url);
    void parseXml(const QByteArray &xml);

private Q_SLOTS:
    void onXmlDownloadStatusChanged();
    void onIconDownloadStatusChanged();
    
    void onProcessError();
    void onProcessFinished(int exitCode);
    
    void onSubscriptionFetched(const QSqlQuery &query, int requestId);

Q_SIGNALS:
    void activeSubscriptionChanged(int subscriptionId);
    void progressChanged(int progress);
    void statusChanged(Status status);
    void statusTextChanged(const QString &text);

private:
    Subscriptions();
    
    static Subscriptions *self;
    
    Transfer *m_xmlDownload;
    Transfer *m_iconDownload;
    QProcess *m_process;
    
    int m_progress;
        
    Status m_status;
    QString m_statusText;
    
    QQueue<int> m_queue;
    
    int m_total;
    
    QSqlQuery m_query;
};

#endif // SUBSCRIPTIONS_H
