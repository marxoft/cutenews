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

#ifndef CUTENEWS_H
#define CUTENEWS_H

#include <QObject>
#include <QPointer>

class MainWindow;

class CuteNews : public QObject
{
    Q_OBJECT
    
    Q_CLASSINFO("D-Bus Interface", "org.marxoft.cutenews")
    
public:
    explicit CuteNews(QObject *parent = 0);
    
public Q_SLOTS:
    Q_SCRIPTABLE bool showArticle(int articleId);
    Q_SCRIPTABLE bool showWindow();

Q_SIGNALS:
    void articleRequested(int articleId);

private:
    QPointer<MainWindow> m_window;
};
    
#endif // CUTENEWS_H
