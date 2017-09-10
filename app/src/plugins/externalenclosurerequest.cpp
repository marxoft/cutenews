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

#include "externalenclosurerequest.h"
#include "json.h"
#include "logger.h"
#include <QProcess>

ExternalEnclosureRequest::ExternalEnclosureRequest(const QString &id, const QString &fileName, QObject *parent) :
    EnclosureRequest(parent),
    m_process(0),
    m_fileName(fileName),
    m_id(id)
{
}

QString ExternalEnclosureRequest::fileName() const {
    return m_fileName;
}

QString ExternalEnclosureRequest::id() const {
    return m_id;
}

QString ExternalEnclosureRequest::errorString() const {
    return m_errorString;
}

void ExternalEnclosureRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

EnclosureResult ExternalEnclosureRequest::result() const {
    return m_result;
}

void ExternalEnclosureRequest::setResult(const EnclosureResult &r) {
    m_result = r;
}

EnclosureRequest::Status ExternalEnclosureRequest::status() const {
    return m_status;
}

void ExternalEnclosureRequest::setStatus(EnclosureRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QProcess* ExternalEnclosureRequest::process() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onRequestError()));
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRequestFinished(int)));
    }
    
    return m_process;
}

bool ExternalEnclosureRequest::cancel() {
    return false;
}

bool ExternalEnclosureRequest::getEnclosure(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }
    
    setStatus(Active);
    QString command = QString("\"%1\" getEnclosure -url \"%2\"").arg(fileName()).arg(url);
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
        Logger::log("ExternalEnclosureRequest::getEnclosure(). Invoking command: " + command, Logger::MediumVerbosity);
        return true;
    default:
        Logger::log("ExternalEnclosureRequest::getEnclosure(). Error invoking command: " + command);
        return false;
    }
}

void ExternalEnclosureRequest::onRequestError() {
    const QString errorString = m_process->errorString();
    Logger::log("ExternalEnclosureRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(EnclosureResult());
    setStatus(Error);
    emit finished(this);
}

void ExternalEnclosureRequest::onRequestFinished(int exitCode) {    
    if (exitCode == 0) {
        Logger::log("ExternalEnclosureRequest::onRequestFinished(). Exit code 0", Logger::MediumVerbosity);
        const QVariantMap map = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput())).toMap();

        if (!map.isEmpty()) {
            const QString fileName = map.value("fileName").toString();
            const QVariantMap request = map.value("request").toMap();
            const QUrl url = request.value("url").toString();

            if ((!fileName.isEmpty()) && (!url.isEmpty())) {
                QNetworkRequest req(url);

                if (request.contains("headers")) {
                    QMapIterator<QString, QVariant> iterator(request.value("headers").toMap());

                    while (iterator.hasNext()) {
                        iterator.next();
                        req.setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
                    }
                }

                m_result.fileName = fileName;
                m_result.request = req;
                m_result.operation = map.value("operation", "GET").toByteArray();
                m_result.data = map.value("data").toByteArray();

                setErrorString(QString());
                setStatus(Ready);
            }
            else {
                setErrorString(tr("Filename or URL is empty"));
                setResult(EnclosureResult());
                setStatus(Error);
            }
        }
        else {
            setErrorString(tr("Invalid response"));
            setResult(EnclosureResult());
            setStatus(Error);
        }        
    }
    else {
        setErrorString(m_process->errorString());        
        Logger::log(QString("ExternalEnclosureRequest::onRequestFinished(). Exit code: %1, Error: %2")
                           .arg(exitCode).arg(errorString()));
        setResult(EnclosureResult());
        setStatus(Error);
    }
    
    emit finished(this);
}
