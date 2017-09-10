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

#include "telegrapharticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef TELEGRAPH_DEBUG
#include <QDebug>
#endif

const int TelegraphArticleRequest::MAX_REDIRECTS = 8;

const QByteArray TelegraphArticleRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

TelegraphArticleRequest::TelegraphArticleRequest(QObject *parent) :
    ArticleRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString TelegraphArticleRequest::errorString() const {
    return m_errorString;
}

void TelegraphArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult TelegraphArticleRequest::result() const {
    return m_result;
}

void TelegraphArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status TelegraphArticleRequest::status() const {
    return m_status;
}

void TelegraphArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool TelegraphArticleRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool TelegraphArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(ArticleResult());
    m_settings = settings;
    m_redirects = 0;
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphArticleRequest::getArticle(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkArticle()));
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void TelegraphArticleRequest::checkArticle() {
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

void TelegraphArticleRequest::followRedirect(const QString &url, const char *slot) {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphArticleRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
}

void TelegraphArticleRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString TelegraphArticleRequest::getRedirect(const QNetworkReply *reply) {
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

void TelegraphArticleRequest::remove(QString &in, const QString &s) {
    const int index = in.indexOf(s);
    const int size = s.size();

    if ((index >= 0) && (size > 0)) {
        in.remove(index, size);
    }
}

void TelegraphArticleRequest::replace(QString &in, const QString &s, const QString &r) {
    const int index = in.indexOf(s);
    const int size = s.size();

    if ((index >= 0) && (size > 0)) {
        in.replace(index, size, r);
    }
}

void TelegraphArticleRequest::unescape(QString &s) {
    s.replace("&amp;", "&");
    s.replace("&apos;", "'");
    s.replace("&lt;", "<");
    s.replace("&gt;", ">");
    s.replace("&quot;", "\"");
}

void TelegraphArticleRequest::writeArticleAuthor(const QHtmlElement &element) {
    m_result.author = element.firstElementByTagName("meta", QHtmlAttributeMatch("name", "DCSext.author"))
        .attribute("content");
}

void TelegraphArticleRequest::writeArticleBody(const QHtmlElement &element) {
    const QHtmlElement bodyEl = element.firstElementByTagName("article", QHtmlAttributeMatch("itemprop",
                "articleBody"));
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

    foreach (const QHtmlElement &aside, bodyEl.elementsByTagName("aside")) {
        remove(m_result.body, aside.toString());
    }

    foreach (const QHtmlElement &share, bodyEl.elementsByTagName("div", QHtmlAttributeMatches()
                << QHtmlAttributeMatch("class", "videoPlayer", QHtmlParser::MatchContains)
                << QHtmlAttributeMatch("class", "advert", QHtmlParser::MatchContains)
                << QHtmlAttributeMatch("class", "apester-media"), QHtmlParser::MatchAny)) {
        remove(m_result.body, share.toString());
    }

    foreach (const QHtmlElement &script, bodyEl.elementsByTagName("script")) {
        remove(m_result.body, script.toString());
    }
}

void TelegraphArticleRequest::writeArticleCategories(const QHtmlElement &element) {
    m_result.categories = element.firstElementByTagName("meta", QHtmlAttributeMatch("name", "keywords"))
        .attribute("content").split(",");
}

void TelegraphArticleRequest::writeArticleDate(const QHtmlElement &element) {
    QHtmlElement dateEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("itemprop", "dateModified"));

    if (dateEl.isNull()) {
        dateEl = element.firstElementByTagName("meta", QHtmlAttributeMatch("itemprop", "datePublished"));
    }

    m_result.date = QDateTime::fromString(dateEl.attribute("content"), Qt::ISODate);
}

void TelegraphArticleRequest::writeArticleEnclosures(const QHtmlElement &) {}

void TelegraphArticleRequest::writeArticleTitle(const QHtmlElement &element) {
    m_result.title = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "og:title"))
        .attribute("content");
    unescape(m_result.title);
}

void TelegraphArticleRequest::writeArticleUrl(const QString &url) {
    m_result.url = url;
}

QNetworkAccessManager* TelegraphArticleRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
