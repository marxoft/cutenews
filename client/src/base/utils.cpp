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
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QUuid>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

Utils::Utils(QObject *parent) :
    QObject(parent)
{
}

QString Utils::createId() {
    const QString uuid = QUuid::createUuid().toString();
    return uuid.mid(1, uuid.size() - 2);
}

QDateTime Utils::dateTimeFromMSecs(qint64 msecs) {
    return QDateTime::fromMSecsSinceEpoch(msecs);
}

QDateTime Utils::dateTimeFromSecs(uint secs) {
    return QDateTime::fromTime_t(secs);
}

qint64 Utils::dateTimeToMSecs(const QDateTime &dt) {
    return dt.toMSecsSinceEpoch();
}

uint Utils::dateTimeToSecs(const QDateTime &dt) {
    return dt.toTime_t();
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

QString Utils::getSanitizedFileName(const QString &fileName) {
    return QString(fileName).replace(QRegExp("[\\/\\\\\\|]"), "_");
}

bool Utils::isLocalFile(const QUrl &url) {
    return (url.scheme() == "file") || (url.toString().startsWith("/"));
}

bool Utils::removeDirectory(const QString &directory) {
#if QT_VERSION >= 0x050000
    return QDir(directory).removeRecursively();
#else
    QDir dir(directory);
    bool ok = false;

    if (dir.exists()) {
        foreach (const QFileInfo &info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden
                                                          | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                ok = removeDirectory(info.absoluteFilePath());
            }
            else {
                ok = QFile::remove(info.absoluteFilePath());
            }

            if (!ok) {
                return ok;
            }
        }

        ok = dir.rmdir(directory);
    }

    return ok;
#endif
}

QString Utils::replaceSrcPaths(const QString &s, const QString &path) {
    QString result(s);
    const QRegExp src(" src=('|\")([^'\"]+)");
    int pos = 0;

    while ((pos = src.indexIn(result, pos)) != -1) {
        const QString url = src.cap(2);
        result.replace(url, path + url.toUtf8().toBase64());
        pos += src.matchedLength();
    }

    return result;
}

QString Utils::toRichText(const QString &s) {
    QString result(s);
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(QRegExp("[\n\r]"), "<br>");
    
    QRegExp re("((http(s|)://|[\\w-_\\.]+@)[^\\s<:\"']+)");
    int pos = 0;

    while ((pos = re.indexIn(result, pos)) != -1) {
        QString link = re.cap(1);
        result.replace(pos, link.size(), QString("<a href='%1'>%2</a>")
                .arg(link.contains('@') ? "mailto:" + link : link).arg(link));
        pos += re.matchedLength() * 2 + 15;
    }

    return result;
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

QString Utils::unescapeHtml(const QString &html) {
    QString s(html);
    s.replace("&amp;", "&");
    s.replace("&apos;", "'");
    s.replace("&lt;", "<");
    s.replace("&gt;", ">");
    s.replace("&quot;", "\"");
    return s;
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
