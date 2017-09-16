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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QVariantMap>

class Utils : public QObject
{
    Q_OBJECT
        
public:
    explicit Utils(QObject *parent = 0);
        
    Q_INVOKABLE static QString createId();
    
    Q_INVOKABLE static QDateTime dateTimeFromMSecs(qint64 msecs);
    Q_INVOKABLE static QDateTime dateTimeFromSecs(uint secs);
    Q_INVOKABLE static qint64 dateTimeToMSecs(const QDateTime &dt);
    Q_INVOKABLE static uint dateTimeToSecs(const QDateTime &dt);
        
    Q_INVOKABLE static QString formatBytes(qint64 bytes);
    
    Q_INVOKABLE static QString formatLargeNumber(qint64 num);
    
    Q_INVOKABLE static QString formatMSecs(qint64 ms);
    Q_INVOKABLE static QString formatSecs(qint64 s);
    
    Q_INVOKABLE static QString getSanitizedFileName(const QString &fileName);

    Q_INVOKABLE static bool isLocalFile(const QUrl &url);

    Q_INVOKABLE static bool removeDirectory(const QString &directory);
    
    Q_INVOKABLE static QString toRichText(QString s);  
    
    Q_INVOKABLE static QString unescape(const QString &s);
    Q_INVOKABLE static QString unescapeHtml(const QString &html);
    
    static QList< QPair<QString, QString> > urlQueryItems(const QUrl &url);
    Q_INVOKABLE static QVariantMap urlQueryItemMap(const QUrl &url);
    Q_INVOKABLE static QString urlQueryItemValue(const QUrl &url, const QString &queryItem,
                                                 const QString &defaultValue = QString());
};

#endif // UTILS_H
