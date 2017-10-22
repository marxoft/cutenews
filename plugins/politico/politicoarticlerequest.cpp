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

#include "politicoarticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef POLITICO_DEBUG
#include <QDebug>
#endif

const int PoliticoArticleRequest::MAX_REDIRECTS = 8;

const QByteArray PoliticoArticleRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

PoliticoArticleRequest::PoliticoArticleRequest(QObject *parent) :
    ArticleRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString PoliticoArticleRequest::errorString() const {
    return m_errorString;
}

void PoliticoArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult PoliticoArticleRequest::result() const {
    return m_result;
}

void PoliticoArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status PoliticoArticleRequest::status() const {
    return m_status;
}

void PoliticoArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool PoliticoArticleRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool PoliticoArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(ArticleResult());
    m_settings = settings;
    m_redirects = 0;
#ifdef POLITICO_DEBUG
    qDebug() << "PoliticoArticleRequest::getArticle(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkArticle()));
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void PoliticoArticleRequest::checkArticle() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        emit finished(this);
        return;
    }

    const QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            reply->deleteLater();
            followRedirect(redirect, SLOT(checkArticle()));
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Error);
            emit finished(this);
        }
        
        return;
    }

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

    const QUrl url = reply->url();
    const QString baseUrl = url.scheme() + "://" + url.authority();
    QString page = QString::fromUtf8(reply->readAll());
    fixRelativeUrls(page, baseUrl);
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    writeArticleAuthor(html);
    writeArticleBody(html);
    writeArticleCategories(html);
    writeArticleDate(html);
    writeArticleEnclosures(html);
    writeArticleTitle(html);
    writeArticleUrl(url.toString());
    setStatus(Ready);
    emit finished(this);
}

void PoliticoArticleRequest::followRedirect(const QString &url, const char *slot) {
#ifdef POLITICO_DEBUG
    qDebug() << "PoliticoArticleRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
}

void PoliticoArticleRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
    const QString scheme = baseUrl.left(baseUrl.indexOf("/"));
    const QRegExp re("( href=| src=)('|\")(?!http)");
    int pos = 0;
    
    while ((pos = re.indexIn(page, pos)) != -1) {
        const int i = re.pos(2) + 1;
        const QString u = page.mid(i, 2);

        if (u == "//") {
            page.insert(i, scheme);
            pos += scheme.size();
        }
        else if (u.startsWith("/")) {
            page.insert(i, baseUrl);
            pos += baseUrl.size();
        }
        else {
            page.insert(i, baseUrl + "/");
            pos += baseUrl.size() + 1;
        }

        pos += re.matchedLength();
    }
}

QString PoliticoArticleRequest::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));

    if ((!redirect.isEmpty()) && (!redirect.startsWith("http"))) {
        const QUrl url = reply->url();
        
        if (redirect.startsWith("/")) {
            redirect.prepend(url.scheme() + "://" + url.authority());
        }
        else {
            redirect.prepend(url.scheme() + "://" + url.authority() + "/");
        }
    }
    
    return redirect;
}

void PoliticoArticleRequest::writeArticleAuthor(const QHtmlElement &element) {
    QHtmlElement author = element.firstElementByTagName("div", QHtmlAttributeMatch("itemprop", "author"));

    if (!author.isNull()) {
        m_result.author = author.firstElementByTagName("meta", QHtmlAttributeMatch("itemprop", "name"))
            .attribute("content");
        return;
    }

    author = element.firstElementByTagName("a", QHtmlAttributeMatch("rel", "author"));

    if (!author.isNull()) {
        m_result.author = author.text();
    }
}

void PoliticoArticleRequest::writeArticleBody(const QHtmlElement &element) {
    const QHtmlElement article = element.firstElementByTagName("div",
            QHtmlAttributeMatch("class", "story-text", QHtmlParser::MatchStartsWith));

    foreach (const QHtmlElement &child, article.childElements()) {
        if ((child.tagName() == "p") && (child.attribute("class").isEmpty())) {
            m_result.body.append(child.toString());
        }
    }
}

void PoliticoArticleRequest::writeArticleCategories(const QHtmlElement &element) {
    const QHtmlElement list = element.firstElementByTagName("ul", QHtmlAttributeMatch("class", "categories-list"));

    foreach (const QHtmlElement &a, list.elementsByTagName("a")) {
        m_result.categories << a.text();
    }
}

void PoliticoArticleRequest::writeArticleDate(const QHtmlElement &element) {
    QHtmlElement date = element.firstElementByTagName("span", QHtmlAttributeMatch("itemprop", "dateModified"));

    if (!date.isNull()) {
        m_result.date = QDateTime::fromString(date.text(), Qt::ISODate);
        return;
    }

    date = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "article:modified_time"));

    if (!date.isNull()) {
        m_result.date = QDateTime::fromString(date.attribute("content"), Qt::ISODate);
        return;
    }

    date = element.firstElementByTagName("time", QHtmlAttributeMatch("itemprop", "datePublished"));

    if (!date.isNull()) {
        m_result.date = QDateTime::fromString(date.attribute("datetime"), Qt::ISODate);
        return;
    }
    
    date = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "article:published_time"));

    if (!date.isNull()) {
        m_result.date = QDateTime::fromString(date.attribute("content"), Qt::ISODate);
    }
}

void PoliticoArticleRequest::writeArticleEnclosures(const QHtmlElement &) {}

void PoliticoArticleRequest::writeArticleTitle(const QHtmlElement &element) {
    m_result.title = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "og:title"))
        .attribute("content");
}

void PoliticoArticleRequest::writeArticleUrl(const QString &url) {
    m_result.url = url;
}

QNetworkAccessManager* PoliticoArticleRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
