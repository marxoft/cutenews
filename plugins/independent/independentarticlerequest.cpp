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

#include "independentarticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef INDEPENDENT_DEBUG
#include <QDebug>
#endif

const int IndependentArticleRequest::MAX_REDIRECTS = 8;

const QByteArray IndependentArticleRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

IndependentArticleRequest::IndependentArticleRequest(QObject *parent) :
    ArticleRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString IndependentArticleRequest::errorString() const {
    return m_errorString;
}

void IndependentArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult IndependentArticleRequest::result() const {
    return m_result;
}

void IndependentArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status IndependentArticleRequest::status() const {
    return m_status;
}

void IndependentArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool IndependentArticleRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool IndependentArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(ArticleResult());
    m_settings = settings;
    m_redirects = 0;
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentArticleRequest::getArticle(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkArticle()));
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void IndependentArticleRequest::checkArticle() {
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

void IndependentArticleRequest::followRedirect(const QString &url, const char *slot) {
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentArticleRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
}

void IndependentArticleRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString IndependentArticleRequest::getRedirect(const QNetworkReply *reply) {
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

void IndependentArticleRequest::remove(QString &in, const QString &s) {
    const int index = in.indexOf(s);
    const int size = s.size();

    if ((index >= 0) && (size > 0)) {
        in.remove(index, size);
    }
}

void IndependentArticleRequest::replace(QString &in, const QString &s, const QString &r) {
    const int index = in.indexOf(s);
    const int size = s.size();

    if ((index >= 0) && (size > 0)) {
        in.replace(index, size, r);
    }
}

void IndependentArticleRequest::unescape(QString &s) {
    s.replace("&amp;", "&");
    s.replace("&apos;", "'");
    s.replace("&lt;", "<");
    s.replace("&gt;", ">");
    s.replace("&quot;", "\"");
}

void IndependentArticleRequest::writeArticleAuthor(const QHtmlElement &element) {
    m_result.author = element.firstElementByTagName("meta", QHtmlAttributeMatch("name", "article:author_name"))
        .attribute("content");
}

void IndependentArticleRequest::writeArticleBody(const QHtmlElement &element) {
    const QHtmlElement bodyEl = element.firstElementByTagName("div", QHtmlAttributeMatch("itemprop", "articleBody"));
    m_result.body = bodyEl.toString();

    if (m_settings.value("includeImages", false).toBool()) {
        foreach (const QHtmlElement &figure, bodyEl.elementsByTagName("figure")) {
            replace(m_result.body, figure.toString(), figure.firstElementByTagName("img").toString());
        }
    }
    else {
        foreach (const QHtmlElement &figure, bodyEl.elementsByTagName("figure")) {
            remove(m_result.body, figure.toString());
        }
    }

    foreach (const QHtmlElement &child, bodyEl.childElements()) {
        if (child.tagName() == "div") {
            remove(m_result.body, child.toString());
        }
    }

    foreach (const QHtmlElement &button, bodyEl.elementsByTagName("a", QHtmlAttributeMatch("class",
                    "syndication-btn"))) {
        remove(m_result.body, button.toString());
    }

    foreach (const QHtmlElement &script, bodyEl.elementsByTagName("script")) {
        remove(m_result.body, script.toString());
    }
}

void IndependentArticleRequest::writeArticleCategories(const QHtmlElement &element) {
    m_result.categories = element.firstElementByTagName("meta", QHtmlAttributeMatch("name", "keywords"))
        .attribute("content").split(",");
}

void IndependentArticleRequest::writeArticleDate(const QHtmlElement &element) {
    QHtmlElement dateEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("property",
                "article:modified_time"));

    if (dateEl.isNull()) {
        dateEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "article:published_time"));
    }

    m_result.date = QDateTime::fromString(dateEl.attribute("content"), Qt::ISODate);
}

void IndependentArticleRequest::writeArticleEnclosures(const QHtmlElement &) {}

void IndependentArticleRequest::writeArticleTitle(const QHtmlElement &element) {
    m_result.title = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "og:title"))
        .attribute("content");
    unescape(m_result.title);
}

void IndependentArticleRequest::writeArticleUrl(const QString &url) {
    m_result.url = url;
}

QNetworkAccessManager* IndependentArticleRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
