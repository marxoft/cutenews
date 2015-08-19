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

#include "urlopenermodel.h"
#include "definitions.h"
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QProcess>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

class Opener : public QVariantMap
{

public:
    Opener(const QString &name, const QString &regExp, const QString &command) :
        QVariantMap()
    {
        insert("name", name);
        insert("regExp", regExp);
        insert("command", command);
    }
};

UrlOpenerModel::UrlOpenerModel(QObject *parent) :
    SelectionModel(parent)
{
}

bool UrlOpenerModel::open(const QString &url) {
    for (int i = 0; i < rowCount(); i++) {
        QVariantMap opener = data(i, "value").toMap();
        
        if (QRegExp(opener.value("regExp").toString()).indexIn(url) == 0) {
            QString command = opener.value("command").toString().replace("%URL%", url);
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

void UrlOpenerModel::load() {
    clear();
    QSettings settings(STORAGE_PATH + "urlopeners", QSettings::NativeFormat);

    foreach (QString group, settings.childGroups()) {
        settings.beginGroup(group);
        QString regExp(settings.value("regExp").toString());
        QString command(settings.value("command").toString());

        if ((!regExp.isEmpty()) && (!command.isEmpty())) {
            append(group, Opener(group, regExp, command));
#ifdef CUTENEWS_DEBUG
            qDebug() << "UrlOpenerModel::load: Opener added" << group << regExp << command;
#endif
        }
#ifdef CUTENEWS_DEBUG
        else {
            qDebug() << "UrlOpenerModel::load: Cannot add opener" << group << regExp << command;
        }
#endif
        settings.endGroup();
    }
}

void UrlOpenerModel::save() {
    QSettings settings(STORAGE_PATH + "urlopeners", QSettings::NativeFormat);
    settings.clear();
    
    for (int i = 0; i < rowCount(); i++) {
        const QString name = data(i, "name").toString();
        const QVariantMap opener = data(i, "value").toMap();
        
        settings.beginGroup(name);
        settings.setValue("regExp", opener.value("regExp"));
        settings.setValue("command", opener.value("command"));
        settings.endGroup();
    }
}
