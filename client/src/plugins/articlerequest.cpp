/*!
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

#include "articlerequest.h"
#include "json.h"
#include "logger.h"
#include "requests.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

ArticleRequest::ArticleRequest(QObject *parent) :
    QObject(parent),
    m_nam(0),
    m_status(Idle)
{
}

QString ArticleRequest::errorString() const {
    return m_errorString;
}

void ArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;

    if (!e.isEmpty()) {
        Logger::log("ArticleRequest::error: " + e);
    }
}

ArticleResult ArticleRequest::result() const {
    return m_result;
}

void ArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

QString ArticleRequest::resultAuthor() const {
    return m_result.author;
}

QString ArticleRequest::resultBody() const {
    return m_result.body;
}

QStringList ArticleRequest::resultCategories() const {
    return m_result.categories;
}

QDateTime ArticleRequest::resultDate() const {
    return m_result.date;
}

QString ArticleRequest::resultDateString() const {
    return m_result.date.toString("dd MMM yyyy HH:mm");
}

QVariantList ArticleRequest::resultEnclosures() const {
    return m_result.enclosures;
}

bool ArticleRequest::resultHasEnclosures() const {
    return !m_result.enclosures.isEmpty();
}

QString ArticleRequest::resultTitle() const {
    return m_result.title;
}

QString ArticleRequest::resultUrl() const {
    return m_result.url;
}

ArticleRequest::Status ArticleRequest::status() const {
    return m_status;
}

void ArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool ArticleRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool ArticleRequest::getArticle(const QString &url) {
    Logger::log("ArticleRequest::getArticle(). URL: " + url, Logger::MediumVerbosity);

    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(ArticleResult());
    QNetworkReply *reply = networkAccessManager()->get(buildRequest("/articles/" + url.toUtf8().toBase64()));
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void ArticleRequest::checkArticle(QNetworkReply *reply) {
    switch (reply->error()) {
        case QNetworkReply::NoError:
            break;
        case QNetworkReply::OperationCanceledError:
            setStatus(Canceled);
            emit finished(this);
            return;
        default:
            setErrorString(reply->errorString());
            setStatus(Error);
            emit finished(this);
            return;
    }

    const QVariantMap result = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toMap();

    if (result.isEmpty()) {
        setErrorString(tr("Article result is empty"));
        setStatus(Error);
    }
    else {
        m_result.author = result.value("author").toString();
        m_result.body = result.value("body").toString();
        m_result.categories = result.value("categories").toStringList();
        m_result.date = result.value("date").toDateTime();
        m_result.enclosures = result.value("enclosures").toList();
        m_result.title = result.value("title").toString();
        m_result.url = result.value("url").toString();
        setStatus(Ready);
    }

    emit finished(this);
}

QNetworkAccessManager* ArticleRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkArticle(QNetworkReply*)));
    }

    return m_nam;
}
