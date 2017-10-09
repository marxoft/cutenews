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
#include <QTimer>

class QNetworkAccessManager;

class Transfers : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int active READ active NOTIFY activeChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    
public:
    ~Transfers();
    
    static Transfers* instance();
        
    int active() const;
    int count() const;
    
    Q_INVOKABLE Transfer* addEnclosureDownload(const QString &url, bool usePlugin);
    Q_INVOKABLE Transfer* addEnclosureDownload(const QString &url, const QString &command, const QString &category,
            int priority, bool usePlugin);
    
    Q_INVOKABLE Transfer* get(int i) const;
    Q_INVOKABLE Transfer* get(const QString &id) const;
    
public Q_SLOTS:
    void setMaximumConcurrentTransfers(int maximum);
    
    bool start();
    bool pause();
    bool start(const QString &id);
    bool pause(const QString &id);
    bool cancel(const QString &id);
    
    void load();
    void save();

private Q_SLOTS:
    void startNextTransfers();
    
    void onTransferStatusChanged();
    
Q_SIGNALS:
    void activeChanged(int active);
    void countChanged(int count);
    void transferAdded(Transfer *transfer);
    
private:
    Transfers();
    
    void getNextTransfers();
    
    void removeTransfer(Transfer *transfer);

    void addActiveTransfer(Transfer *transfer);
    void removeActiveTransfer(Transfer *transfer);
    
    static Transfers *self;
        
    QNetworkAccessManager *m_nam;
    
    QTimer m_queueTimer;
    
    QList<Transfer*> m_transfers;
    QList<Transfer*> m_active;    
};
    
#endif // TRANSFERS_H
