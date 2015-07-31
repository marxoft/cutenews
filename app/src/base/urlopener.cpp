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

#include "urlopener.h"
#include "definitions.h"
#include <QSettings>
#include <QProcess>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

UrlOpener* UrlOpener::self = 0;

UrlOpener::UrlOpener() :
    QObject()
{
}

UrlOpener::~UrlOpener() {
    self = 0;
}

UrlOpener* UrlOpener::instance() {
    return self ? self : self = new UrlOpener;
}

void UrlOpener::load() {
    m_openers.clear();
    QSettings settings(STORAGE_PATH + "urlopeners", QSettings::NativeFormat);

    foreach (QString group, settings.childGroups()) {
        settings.beginGroup(group);
        QRegExp re(settings.value("regExp").toString());
        QString command(settings.value("command").toString());

        if ((re.isValid()) && (!command.isEmpty())) {
            Opener opener;
            opener.name = group;
            opener.regExp = re;
            opener.command = command;
            m_openers.append(opener);
#ifdef CUTENEWS_DEBUG
            qDebug() << "UrlOpener::loadOpeners: Opener added" << group << re.pattern() << command;
#endif
        }
#ifdef CUTENEWS_DEBUG
        else {
            qDebug() << "UrlOpener::loadOpeners: Cannot add opener" << group << re.pattern() << command;
        }
#endif
        settings.endGroup();
    }
}

bool UrlOpener::addOpener(const QString &name, const QString &regExp, const QString &command) {
    QRegExp re(regExp);

    if ((re.isValid()) && (!command.isEmpty())) {
        Opener opener;
        opener.name = name;
        opener.regExp = re;
        opener.command = command;
        m_openers.append(opener);

        QSettings settings(STORAGE_PATH + "urlopeners", QSettings::NativeFormat);
        settings.beginGroup(name);
        settings.setValue("regExp", regExp);
        settings.setValue("command", command);
        settings.endGroup();
#ifdef CUTENEWS_DEBUG
        qDebug() << "UrlOpener::addOpener: Opener added" << name << re.pattern() << command;
#endif
        return true;
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "UrlOpener::addOpener: Cannot add opener" << name << re.pattern() << command;
#endif
    return false;
}

void UrlOpener::removeOpener(const QString &name) {
    QSettings(STORAGE_PATH + "urlopeners", QSettings::NativeFormat).remove(name);
}

QList<Opener> UrlOpener::openers() const {
    return m_openers;
}

QString UrlOpener::opener(const QString &url) const {
    foreach (Opener opener, m_openers) {
        if (opener.regExp.indexIn(url) == 0) {
            return opener.name;
        }
    }

    return QString();
}

bool UrlOpener::canOpen(const QString &url) const {
    return !opener(url).isEmpty();
}

bool UrlOpener::open(const QString &url) {
    foreach (Opener opener, m_openers) {
        if (opener.regExp.indexIn(url) == 0) {
            QString command = QString(opener.command).replace("%URL%", url);
#ifdef CUTENEWS_DEBUG
            qDebug() << "UrlOpener::open: Opening" << url << "with command" << command;
#endif
            QProcess *process = new QProcess(this);
            connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
            process->start(command);
            return true;
        }
    }
#ifdef CUTENEWS_DEBUG
    qDebug() << "UrlOpener::open: No opener found for" << url;
#endif
    return false;
}
