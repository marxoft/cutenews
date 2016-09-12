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

#ifndef CUTENEWS_H
#define CUTENEWS_H

#include <QObject>
#include <QPointer>

class DBConnection;
class QDeclarativeEngine;

class CuteNews : public QObject
{
    Q_OBJECT
    
    Q_CLASSINFO("D-Bus Interface", "org.marxoft.cutenews")
    
public:
    ~CuteNews();
    
    static CuteNews* instance();
    
public Q_SLOTS:
    Q_SCRIPTABLE bool quit();
    Q_SCRIPTABLE bool showArticle(const QString &articleId);
    Q_SCRIPTABLE bool showWidget();
    Q_SCRIPTABLE bool showWindow();

Q_SIGNALS:
    void articleRequested(const QString &articleId);
    
private:
    CuteNews();
    
    void initEngine();
    QObject* createQmlObject(const QString &fileName);
    
    static CuteNews *self;
    
    static const QString WIDGET_FILENAME;
    static const QString WINDOW_FILENAME;
    
    DBConnection *m_connection;
    QDeclarativeEngine *m_engine;
    QPointer<QObject> m_widget;
    QPointer<QObject> m_window;    
};
    
#endif // CUTENEWS_H
