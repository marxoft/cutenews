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

#include "bbcarticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef BBC_DEBUG
#include <QDebug>
#endif

const int BbcArticleRequest::MAX_REDIRECTS = 8;

const QByteArray BbcArticleRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

BbcArticleRequest::BbcArticleRequest(QObject *parent) :
    ArticleRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString BbcArticleRequest::errorString() const {
    return m_errorString;
}

void BbcArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult BbcArticleRequest::result() const {
    return m_result;
}

void BbcArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status BbcArticleRequest::status() const {
    return m_status;
}

void BbcArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool BbcArticleRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool BbcArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(ArticleResult());
    m_settings = settings;
    m_redirects = 0;
#ifdef BBC_DEBUG
    qDebug() << "BbcArticleRequest::getArticle(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkArticle()));
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void BbcArticleRequest::checkArticle() {
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

void BbcArticleRequest::followRedirect(const QString &url, const char *slot) {
#ifdef BBC_DEBUG
    qDebug() << "BbcArticleRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
}

void BbcArticleRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString BbcArticleRequest::getRedirect(const QNetworkReply *reply) {
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

void BbcArticleRequest::remove(QString &in, const QString &s) {
    const int index = in.indexOf(s);
    const int size = s.size();

    if ((index >= 0) && (size > 0)) {
        in.remove(index, size);
    }
}

void BbcArticleRequest::replace(QString &in, const QString &s, const QString &r) {
    const int index = in.indexOf(s);
    const int size = s.size();

    if ((index >= 0) && (size > 0)) {
        in.replace(index, size, r);
    }
}

void BbcArticleRequest::writeArticleAuthor(const QHtmlElement &element) {
    m_result.author = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "article:author"))
        .attribute("content");
}

void BbcArticleRequest::writeArticleBody(const QHtmlElement &element) {
    const QHtmlElement bodyEl = element.firstElementByTagName("div", QHtmlAttributeMatches()
        << QHtmlAttributeMatch("property", "articleBody") << QHtmlAttributeMatch("class", "main_article_text")
        << QHtmlAttributeMatch("id", "story-body") << QHtmlAttributeMatch("class", "vxp-media__summary"),
        QHtmlParser::MatchAny);

    m_result.body = bodyEl.toString();

    if (m_settings.value("includeImages", false).toBool()) {
        foreach (const QHtmlElement &figure, bodyEl.elementsByTagName("figure")) {
            replace(m_result.body, figure.toString(), figure.firstElementByTagName("img").toString());
        }
    }
    else {
        foreach (const QHtmlElement &div, bodyEl.elementsByTagName("div", QHtmlAttributeMatch("class", "ns_datapic",
                        QHtmlParser::MatchContains))) {
            remove(m_result.body, div.toString());
        }

        foreach (const QHtmlElement &figure, bodyEl.elementsByTagName("figure")) {
            remove(m_result.body, figure.toString());
        }

        foreach (const QHtmlElement &style, bodyEl.elementsByTagName("style")) {
            remove(m_result.body, style.toString());
        }
    }

    foreach (const QHtmlElement &div, bodyEl.elementsByTagName("div", QHtmlAttributeMatch("class", "bbccom",
                    QHtmlParser::MatchStartsWith))) {
        remove(m_result.body, div.toString());
    }

    foreach (const QHtmlElement &link, bodyEl.elementsByTagName("link")) {
        remove(m_result.body, link.toString());
    }

    foreach (const QHtmlElement &script, bodyEl.elementsByTagName("script")) {
        remove(m_result.body, script.toString());
    }
}

void BbcArticleRequest::writeArticleCategories(const QHtmlElement &element) {
    const QHtmlElement catEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("property",
                "article:section"));

    if (!catEl.isNull()) {
        m_result.categories << catEl.attribute("content");
    }
}

void BbcArticleRequest::writeArticleDate(const QHtmlElement &element) {
    QHtmlElement dateEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("property",
                "rnews:datePublished"));

    if (!dateEl.isNull()) {
        m_result.date = QDateTime::fromString(dateEl.attribute("content"), "yyyy/MM/dd HH:mm:ss");
    }
    else {
        dateEl = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "date date--v2"));
        
        if (!dateEl.isNull()) {
            m_result.date = QDateTime::fromTime_t(dateEl.attribute("data-seconds").toInt());
        }
    }
}

void BbcArticleRequest::writeArticleEnclosures(const QHtmlElement &) {}

void BbcArticleRequest::writeArticleTitle(const QHtmlElement &element) {
    QHtmlElement titleEl = element.firstElementByTagName("h1", QHtmlAttributeMatch("class", "story-body__h1"));

    if (!titleEl.isNull()) {
        m_result.title = titleEl.text();
    }
    else {
        titleEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "og:title"));

        if (!titleEl.isNull()) {
            m_result.title = titleEl.attribute("content");
        }
    }
}

void BbcArticleRequest::writeArticleUrl(const QString &url) {
    m_result.url = url;
}

QNetworkAccessManager* BbcArticleRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
