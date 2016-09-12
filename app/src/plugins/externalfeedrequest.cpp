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

#include "externalfeedrequest.h"
#include "logger.h"
#include <QProcess>
#include <QVariantMap>

ExternalFeedRequest::ExternalFeedRequest(const QString &id, const QString &fileName, QObject *parent) :
    FeedRequest(parent),
    m_process(0),
    m_fileName(fileName),
    m_id(id)
{
}

QString ExternalFeedRequest::fileName() const {
    return m_fileName;
}

QString ExternalFeedRequest::id() const {
    return m_id;
}

QString ExternalFeedRequest::errorString() const {
    return m_errorString;
}

void ExternalFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray ExternalFeedRequest::result() const {
    return m_result;
}

void ExternalFeedRequest::setResult(const QByteArray &r) {
    m_result = r;
}

FeedRequest::Status ExternalFeedRequest::status() const {
    return m_status;
}

void ExternalFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QProcess* ExternalFeedRequest::process() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onRequestError()));
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRequestFinished(int)));
    }
    
    return m_process;
}

bool ExternalFeedRequest::cancel() {
    return false;
}

bool ExternalFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }
    
    setStatus(Active);
    QString command = QString("\"%1\" getFeed").arg(fileName());
    QMapIterator<QString, QVariant> iterator(settings);
    
    while (iterator.hasNext()) {
        iterator.next();
        command.append(" -");
        command.append(iterator.key());
        command.append(" ");
        command.append("\"");
        command.append(iterator.value().toString());
        command.append("\"");
    }
    
    QProcess *pr = process();
    pr->start(command);
    
    switch (pr->state()) {
    case QProcess::Starting:
    case QProcess::Running:
        Logger::log("ExternalFeedRequest::getFeed(). Invoking command: " + command, Logger::MediumVerbosity);
        return true;
    default:
        Logger::log("ExternalFeedRequest::getFeed(). Error invoking command: " + command);
        return false;
    }
}

void ExternalFeedRequest::onRequestError() {
    const QString errorString = m_process->errorString();
    Logger::log("ExternalFeedRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(QByteArray());
    setStatus(Error);
    emit finished(this);
}

void ExternalFeedRequest::onRequestFinished(int exitCode) {    
    if (exitCode == 0) {
        Logger::log("ExternalFeedRequest::onRequestFinished(). Exit code 0", Logger::MediumVerbosity);
        setErrorString(QString());
        setResult(m_process->readAllStandardOutput());
        setStatus(Ready);
    }
    else {
        setErrorString(m_process->errorString());        
        Logger::log(QString("ExternalFeedRequest::onRequestFinished(). Exit code: %1, Error: %2")
                           .arg(exitCode).arg(errorString()));
        setResult(QByteArray());
        setStatus(Error);
    }
    
    emit finished(this);
}
