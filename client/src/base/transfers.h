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

#ifndef TRANSFERS_H
#define TRANSFERS_H

#include "transfer.h"

class QNetworkAccessManager;

class Transfers : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int active READ active NOTIFY activeChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    
    Q_ENUMS(Status)
    
public:
    enum Status {
        Idle = 0,
        Active,
        Ready,
        Error
    };
        
    ~Transfers();
    
    static Transfers* instance();
        
    int active() const;
    int count() const;
    
    QString errorString() const;
    
    Status status() const;
    
    Q_INVOKABLE void addEnclosureDownload(const QString &url, const QString &subscriptionId,
                                          const QString &category = QString(),
                                          int priority = Transfer::NormalPriority);
    
    Q_INVOKABLE Transfer* get(int i) const;
    Q_INVOKABLE Transfer* get(const QString &id) const;
    
public Q_SLOTS:    
    bool start();
    bool pause();
    bool start(const QString &id);
    bool pause(const QString &id);
    bool cancel(const QString &id);
    
    void load();
    void clear();
    
private Q_SLOTS:
    void onTransfersChanged();
    void onTransfersLoaded();

Q_SIGNALS:
    void activeChanged(int active);
    void countChanged(int count);
    void error(const QString &errorString);
    void statusChanged(Transfers::Status s);
    void transferAdded(Transfer *transfer);
    
private:
    Transfers();
    
    void setErrorString(const QString &e);
    
    void setStatus(Status s);
    
    void removeTransfer(Transfer *transfer);
    void addActiveTransfer(Transfer *transfer);
    void removeActiveTransfer(Transfer *transfer);
    
    QNetworkAccessManager* networkAccessManager();
    
    static Transfers *self;
        
    QNetworkAccessManager *m_nam;
    
    QString m_errorString;
    
    Status m_status;
        
    QList<Transfer*> m_transfers;
    QList<Transfer*> m_active;
};
    
#endif // TRANSFERS_H
