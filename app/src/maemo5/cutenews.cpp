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
#include <QDBusConnection>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <qdeclarative.h>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

static const QString WIDGET_FILENAME("/opt/cutenews/qml/Widget.qml");
static const QString WINDOW_FILENAME("/opt/cutenews/qml/MainWindow.qml");

CuteNews::CuteNews(QDeclarativeEngine *parent) :
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

bool CuteNews::showWidget() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showWidget";
#endif
    if (m_widget.isNull()) {
        if (QObject *obj = createQmlObject(WIDGET_FILENAME)) {
            m_widget = obj;
            return true;
        }
    }
    
    return false;
}

bool CuteNews::showWindow() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "CuteNews::showWindow";
#endif
    if (m_window.isNull()) {
        m_window = createQmlObject(WINDOW_FILENAME);
    }
    
    if (!m_window.isNull()) {
        QMetaObject::invokeMethod(m_window, "activate");
        return true;
    }
    
    return false;
}

QObject* CuteNews::createQmlObject(const QString &fileName) {
    if (QDeclarativeEngine *engine = qobject_cast<QDeclarativeEngine*>(parent())) {
        QDeclarativeContext *context = new QDeclarativeContext(engine->rootContext());
        QDeclarativeComponent *component = new QDeclarativeComponent(engine, fileName, this);
        
        if (QObject *obj = component->create(context)) {
            context->setParent(obj);
            return obj;
        }
#ifdef CUTENEWS_DEBUG
        if (component->isError()) {
            foreach (QDeclarativeError error, component->errors()) {
                qDebug() << error.toString();
            }        
        }
#endif
        delete component;
        delete context;
    }
    
    return 0;
}
