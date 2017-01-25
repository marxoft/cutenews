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
#include <QProcess>
#include <QRegExp>
#include <QSettings>

UrlOpenerModel* UrlOpenerModel::self = 0;

UrlOpenerModel::UrlOpenerModel() :
    SelectionModel()
{
}

UrlOpenerModel::~UrlOpenerModel() {
    self = 0;
}

UrlOpenerModel* UrlOpenerModel::instance() {
    return self ? self : self = new UrlOpenerModel;
}

QVariant UrlOpenerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("RegExp");
    case 1:
        return tr("Command");
    default:
        return QVariant();
    }
}

void UrlOpenerModel::append(const QString &regExp, const QVariant &command) {
    const int i = match(0, "name", regExp);

    if (i == -1) {
        SelectionModel::append(regExp, command);
    }
    else {
        setData(i, command, "value");
    }
}

void UrlOpenerModel::insert(int row, const QString &regExp, const QVariant &command) {
    const int i = match(0, "name", regExp);

    if (i == -1) {
        SelectionModel::insert(row, regExp, command);
    }
    else {
        setData(i, command, "value");
    }
}

void UrlOpenerModel::load() {
    clear();
    QSettings settings(APP_CONFIG_PATH + "urlopeners", QSettings::IniFormat);
    const int size = settings.beginReadArray("urlopeners");

    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        const QString regExp = settings.value("regExp").toString();
        const QString command = settings.value("command").toString();

        if ((!regExp.isEmpty()) && (!command.isEmpty())) {
            append(regExp, command);
            Logger::log(QString("UrlOpenerModel::load(). Opener added. RegExp: %2, Command: %3")
                               .arg(regExp).arg(command), Logger::LowVerbosity);
        }
        else {
            Logger::log(QString("UrlOpenerModel::load(). Cannot add opener. RegExp: %2, Command: %3")
                               .arg(regExp).arg(command));
        }
    }

    settings.endArray();
}

void UrlOpenerModel::save() {
    QSettings settings(APP_CONFIG_PATH + "urlopeners", QSettings::IniFormat);
    settings.beginWriteArray("urlopeners");
    
    for (int i = 0; i < rowCount(); i++) {        
        settings.setArrayIndex(i);
        settings.setValue("regExp", data(i, "name"));
        settings.setValue("command", data(i, "value"));
    }

    settings.endArray();
}

bool UrlOpenerModel::open(const QString &url) {
    for (int i = 0; i < rowCount(); i++) {
        const QRegExp re = QRegExp(data(i, "name").toString());
        
        if (re.indexIn(url) == 0) {
            const QString command = data(i, "value").toString().replace("%u", url);
            Logger::log(QString("UrlOpener::open(). URL: %1, Command: %2").arg(url).arg(command), Logger::LowVerbosity);
            return QProcess::startDetached(command);
        }
    }
    
    Logger::log("UrlOpener::open(). No opener found for URL: " + url, Logger::LowVerbosity);
    return false;
}
