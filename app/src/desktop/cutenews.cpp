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

#include "cutenews.h"
#include "mainwindow.h"
#include <QDBusConnection>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

CuteNews::CuteNews(QObject *parent) :
    QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews");
    connection.registerObject("/org/marxoft/cutenews", this, QDBusConnection::ExportScriptableSlots);
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
