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
#include "database.h"
#include "mainwindow.h"
#include "transfers.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QThread>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

CuteNews* CuteNews::self = 0;

CuteNews::CuteNews() :
    QObject()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews");
    connection.registerObject("/org/marxoft/cutenews", this, QDBusConnection::ExportScriptableSlots);
}

CuteNews::~CuteNews() {
    self = 0;
}

CuteNews* CuteNews::instance() {
    return self ? self : self = new CuteNews;
}

bool CuteNews::quit() {
    QThread *dbThread = Database::instance()->thread();

    if ((dbThread) && (dbThread != QThread::currentThread())) {
        if (dbThread->isRunning()) {
#ifdef CUTENEWS_DEBUG
            qDebug() << "Database thread still running. Calling QThread::quit()";
#endif
            connect(dbThread, SIGNAL(finished()), this, SLOT(quit()), Qt::UniqueConnection);
            dbThread->quit();
            return false;
        }
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "Database thread finished. Quitting the application";
#endif
    Transfers::instance()->save();
    QCoreApplication::quit();
    return true;
}

bool CuteNews::showArticle(int articleId) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showArticle" << articleId;
#endif
    if ((articleId) && (showWindow())) {
        emit articleRequested(articleId);
        return true;
    }
    
    return false;
}

bool CuteNews::showWindow() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showWindow";
#endif
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
