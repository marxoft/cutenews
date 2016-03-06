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

#include "utils.h"
#include <QRegExp>
#include <QDir>
#include <QDateTime>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

static bool RAND_SEEDED = false;

Utils::Utils(QObject *parent) :
    QObject(parent)
{
}

int Utils::createId() {
    if (!RAND_SEEDED) {
        qsrand(QDateTime::currentMSecsSinceEpoch());
    }
    
    return qrand();
}

QString Utils::formatBytes(qint64 bytes) {
    if (bytes <= 0) {
        return QString("0B");
    }
    
    double kb = 1024;
    double mb = kb * 1024;
    double gb = mb * 1024;

    QString size;

    if (bytes > gb) {
        size = QString::number(bytes / gb, 'f', 2) + "GB";
    }
    else if (bytes > mb) {
        size = QString::number(bytes / mb, 'f', 2) + "MB";
    }
    else if (bytes > kb) {
        size = QString::number(bytes / kb, 'f', 2) + "KB";
    }
    else {
        size = QString::number(bytes) + "B";
    }

    return size;
}

QString Utils::formatLargeNumber(qint64 num) {
    if (num < 1000) {
        return QString::number(num);
    }
    
    double k = 1000;
    double m = k * 1000;

    QString result;

    if (num > m) {
        result = QString::number(num / m, 'f', 1) + "m";
    }
    else {
        result = QString::number(num / k, 'f', 1) + "k";
    }

    return result;
}

QString Utils::formatMSecs(qint64 ms) {    
    return ms > 0 ? formatSecs(ms / 1000) : QString("--:--");
}

QString Utils::formatSecs(qint64 s) {    
    return s > 0 ? QString("%1:%2").arg(s / 60, 2, 10, QChar('0')).arg(s % 60, 2, 10, QChar('0')) : QString("--:--");
}

bool Utils::isLocalFile(const QUrl &url) {
    return (url.scheme() == "file") || (url.toString().startsWith("/"));
}

QString Utils::toRichText(QString s) {
    s.replace("&", "&amp;").replace("<", "&lt;").replace(QRegExp("[\n\r]"), "<br>");
    
    QRegExp re("((http(s|)://|[\\w-_\\.]+@)[^\\s<:\"']+)");
    int pos = 0;

    while ((pos = re.indexIn(s, pos)) != -1) {
        QString link = re.cap(1);
        s.replace(pos, link.size(), QString("<a href='%1'>%2</a>").arg(link.contains('@') ? "mailto:" + link : link).arg(link));
        pos += re.matchedLength() * 2 + 15;
    }

    return s;
}

QString Utils::unescape(const QString &s) {
    int unescapes = 0;
    QByteArray us = s.toUtf8();

    while ((us.contains('%')) && (unescapes < 10)) {
        us = QByteArray::fromPercentEncoding(us);
        unescapes++;
    }

    return QString::fromUtf8(us);
}

QList< QPair<QString, QString> > Utils::urlQueryItems(const QUrl &url) {
#if QT_VERSION >= 0x050000
    return QUrlQuery(url).queryItems();
#else
    return url.queryItems();
#endif
}

QVariantMap Utils::urlQueryItemMap(const QUrl &url) {
    QList< QPair<QString, QString> > queryItems = urlQueryItems(url);
    QVariantMap map;
    
    while (!queryItems.isEmpty()) {
        const QPair<QString, QString> &queryItem = queryItems.takeFirst();
        map[queryItem.first] = queryItem.second;
    }
    
    return map;
}

QString Utils::urlQueryItemValue(const QUrl &url, const QString &queryItem, const QString &defaultValue) {
#if QT_VERSION >= 0x050000
    const QUrlQuery query(url);
    return query.hasQueryItem(queryItem) ? query.queryItemValue(queryItem) : defaultValue;
#else
    return url.hasQueryItem(queryItem) ? url.queryItemValue(queryItem) : defaultValue;
#endif
}

QString Utils::urlQueryToSqlQuery(const QUrl &url) {
    if (!url.hasQuery()) {
        return QString();
    }
    
    QVariantMap map = urlQueryItemMap(url);
    int limit = 0;
    int offset = 0;
    QString orderBy("id");
    QString order("ASC");
    QString queryString;
    QString sqlQuery;
    
    if (map.contains("limit")) {
        limit = qMax(1, map.value("limit").toInt());
        map.remove("limit");
    }
    
    if (map.contains("offset")) {
        offset = qMax(0, map.value("offset").toInt());
        map.remove("offset");
    }
    
    if (map.contains("sort")) {
        orderBy = map.value("sort").toString();
        map.remove("sort");
    }
    
    if (map.contains("sortDescending")) {
        if (map.value("sortDescending").toBool()) {
            order = QString("DESC");
        }
        
        map.remove("sortDescending");
    }
    
    QMapIterator<QString, QVariant> iterator(map);
    
    while (iterator.hasNext()) {
        iterator.next();
        queryString.append(QString("%1 = '%2'").arg(iterator.key()).arg(iterator.value().toString()));
        
        if (iterator.hasNext()) {
            queryString.append(" AND ");
        }
    }
    
    if (!queryString.isEmpty()) {
        sqlQuery.append("WHERE " + queryString);
    }
    
    sqlQuery.append(QString(" ORDER BY %1 %2").arg(orderBy).arg(order));
    
    if (limit > 0) {
        sqlQuery.append(QString(" LIMIT %1, %2").arg(offset).arg(limit));
    }
    
    return sqlQuery;
}
