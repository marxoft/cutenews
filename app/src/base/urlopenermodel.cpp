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

#include "urlopenermodel.h"
#include "definitions.h"
#include "logger.h"
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QProcess>

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
        const QVariantMap opener = data(i, "value").toMap();
        
        if (QRegExp(opener.value("regExp").toString()).indexIn(url) == 0) {
            const QString command = opener.value("command").toString().replace("%u", url);
            Logger::log(QString("UrlOpener::open(). URL: %1, Command: %2").arg(url).arg(command), Logger::LowVerbosity);
            return QProcess::startDetached(command);
        }
    }
    
    Logger::log("UrlOpener::open(). No opener found for URL: " + url, Logger::LowVerbosity);
    return false;
}

void UrlOpenerModel::load() {
    clear();
    QSettings settings(APP_CONFIG_PATH + "urlopeners", QSettings::IniFormat);

    foreach (const QString &group, settings.childGroups()) {
        settings.beginGroup(group);
        QString regExp(settings.value("regExp").toString());
        QString command(settings.value("command").toString());

        if ((!regExp.isEmpty()) && (!command.isEmpty())) {
            append(group, Opener(group, regExp, command));
            Logger::log(QString("UrlOpenerModel::load(). Opener added. Name: %1, RegExp: %2, Command: %3")
                               .arg(group).arg(regExp).arg(command), Logger::LowVerbosity);
        }
        else {
            Logger::log(QString("UrlOpenerModel::load(). Cannot add opener. Name: %1, RegExp: %2, Command: %3")
                               .arg(group).arg(regExp).arg(command));
        }
        settings.endGroup();
    }
}

void UrlOpenerModel::save() {
    QSettings settings(APP_CONFIG_PATH + "urlopeners", QSettings::IniFormat);
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
