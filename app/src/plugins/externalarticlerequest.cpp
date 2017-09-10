/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "externalarticlerequest.h"
#include "json.h"
#include "logger.h"
#include <QProcess>

ExternalArticleRequest::ExternalArticleRequest(const QString &id, const QString &fileName, QObject *parent) :
    ArticleRequest(parent),
    m_process(0),
    m_fileName(fileName),
    m_id(id)
{
}

QString ExternalArticleRequest::fileName() const {
    return m_fileName;
}

QString ExternalArticleRequest::id() const {
    return m_id;
}

QString ExternalArticleRequest::errorString() const {
    return m_errorString;
}

void ExternalArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult ExternalArticleRequest::result() const {
    return m_result;
}

void ExternalArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status ExternalArticleRequest::status() const {
    return m_status;
}

void ExternalArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QProcess* ExternalArticleRequest::process() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onRequestError()));
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRequestFinished(int)));
    }
    
    return m_process;
}

bool ExternalArticleRequest::cancel() {
    return false;
}

bool ExternalArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }
    
    setStatus(Active);
    QString command = QString("\"%1\" getArticle -url \"%2\"").arg(fileName()).arg(url);
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
        Logger::log("ExternalArticleRequest::getArticle(). Invoking command: " + command, Logger::MediumVerbosity);
        return true;
    default:
        Logger::log("ExternalArticleRequest::getArticle(). Error invoking command: " + command);
        return false;
    }
}

void ExternalArticleRequest::onRequestError() {
    const QString errorString = m_process->errorString();
    Logger::log("ExternalArticleRequest::onRequestError(): " + errorString);
    setErrorString(errorString);
    setResult(ArticleResult());
    setStatus(Error);
    emit finished(this);
}

void ExternalArticleRequest::onRequestFinished(int exitCode) {    
    if (exitCode == 0) {
        Logger::log("ExternalArticleRequest::onRequestFinished(). Exit code 0", Logger::MediumVerbosity);
        const QVariantMap map = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput())).toMap();

        if (!map.isEmpty()) {
            m_result.author = map.value("author").toString();
            m_result.body = map.value("body").toString();
            m_result.categories = map.value("categories").toStringList();
            m_result.date = map.value("date").toDateTime();
            m_result.enclosures = map.value("enclosures").toList();
            m_result.title = map.value("title").toString();
            m_result.url = map.value("url").toString();
            setErrorString(QString());
            setStatus(Ready);
        }
        else {
            setErrorString(tr("Invalid response"));
            setResult(ArticleResult());
            setStatus(Error);
        }        
    }
    else {
        setErrorString(m_process->errorString());        
        Logger::log(QString("ExternalArticleRequest::onRequestFinished(). Exit code: %1, Error: %2")
                           .arg(exitCode).arg(errorString()));
        setResult(ArticleResult());
        setStatus(Error);
    }
    
    emit finished(this);
}
