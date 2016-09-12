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

#include "cutenews.h"
#include "dbconnection.h"
#include "logger.h"
#include "mainwindow.h"
#include "settings.h"
#include "transfers.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QThread>
#ifdef DBUS_INTERFACE
#include <QDBusConnection>
#endif

CuteNews* CuteNews::self = 0;

CuteNews::CuteNews() :
    QObject()
{
#ifdef DBUS_INTERFACE
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews");
    connection.registerObject("/org/marxoft/cutenews", this, QDBusConnection::ExportScriptableSlots);
#endif
}

CuteNews::~CuteNews() {
    self = 0;
}

CuteNews* CuteNews::instance() {
    return self ? self : self = new CuteNews;
}

bool CuteNews::quit() {
    QThread *dbThread = DBConnection::asynchronousThread();

    if ((dbThread) && (dbThread != QThread::currentThread())) {
        if (dbThread->isRunning()) {
            Logger::log("CuteNews::quit(). Shutting down the database thread",
                        Logger::LowVerbosity);
            connect(dbThread, SIGNAL(finished()), this, SLOT(quit()), Qt::UniqueConnection);
            dbThread->quit();
            return false;
        }
    }
    
    const int expiryDays = Settings::readArticleExpiry();
    
    if (expiryDays > -1) {
        uint expiryDate = QDateTime::currentDateTime().toTime_t();

        if (expiryDays > 0) {
            expiryDate -= expiryDays * 86400;
        }
        
        Logger::log("CuteNews::quit(). Deleting read articles older than "
                    + QDateTime::fromTime_t(expiryDate).toString(Qt::ISODate));
        DBConnection(false).deleteExpiredArticles(expiryDate);
    }
    
    Transfers::instance()->save();
    Logger::log("CuteNews::quit(). Quitting the application", Logger::LowVerbosity);
    QCoreApplication::quit();
    return true;
}

bool CuteNews::showArticle(const QString &id) {
    if ((!id.isEmpty()) && (showWindow())) {
        emit articleRequested(id);
        return true;
    }
    
    return false;
}

bool CuteNews::showWindow() {
    if (m_window.isNull()) {
        m_window = new MainWindow;
    }
    
    if (!m_window.isNull()) {
        m_window->show();
        m_window->activateWindow();
        m_window->raise();
        return true;
    }
    
    return false;
}
