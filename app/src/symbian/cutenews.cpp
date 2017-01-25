/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "transfers.h"
#include <QCoreApplication>
#include <QThread>

CuteNews* CuteNews::self = 0;

CuteNews::CuteNews() :
    QObject()
{
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
    
    Transfers::instance()->save();
    Logger::log("CuteNews::quit(). Quitting the application", Logger::LowVerbosity);
    QCoreApplication::quit();
    return true;
}
