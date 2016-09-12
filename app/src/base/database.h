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

#ifndef DATABASE_H
#define DATABASE_H

#include "definitions.h"
#include "logger.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

bool initDatabase() {
    if (!DATABASE_PATH.isEmpty()) {
        if (!QDir().mkpath(DATABASE_PATH)) {
            Logger::log("initDatabase(). Unable to make path " + DATABASE_PATH);
            return false;
        }
    }
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(DATABASE_NAME);
    
    if (!db.isOpen()) {
        db.open();
    }
    
    QSqlQuery query = db.exec("CREATE TABLE IF NOT EXISTS subscriptions (id TEXT PRIMARY KEY NOT NULL, \
    description TEXT, downloadEnclosures INTEGER, iconPath TEXT, lastUpdated INTEGER, source TEXT, \
    sourceType INTEGER, title TEXT, updateInterval INTEGER, url TEXT)");
    QSqlError error = query.lastError();
    
    if (error.isValid()) {
        Logger::log("initDatabase(). Error: " +  error.text());
        db.close();
        return false;
    }
    
    query = db.exec("CREATE TABLE IF NOT EXISTS articles (id TEXT PRIMARY KEY NOT NULL, author TEXT, body TEXT, \
    categories TEXT, date INTEGER, enclosures TEXT, isFavourite INTEGER, isRead INTEGER, lastRead INTEGER, \
    subscriptionId TEXT REFERENCES subscriptions(id) ON DELETE CASCADE, title TEXT, url TEXT)");
    error = query.lastError();
    
    if (error.isValid()) {
        Logger::log("initDatabase(). Error: " +  error.text());
        db.close();
        return false;
    }
    
    Logger::log("initDatabase(). OK", Logger::LowVerbosity);
    db.close();
    return true;
}

#endif // DATABASE_H
