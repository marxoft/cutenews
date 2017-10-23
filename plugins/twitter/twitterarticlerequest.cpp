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

#include "twitterarticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef TWITTER_DEBUG
#include <QDebug>
#endif

const QString TwitterArticleRequest::BASE_URL("https://twitter.com");

const int TwitterArticleRequest::MAX_REDIRECTS = 8;

const QByteArray TwitterArticleRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

TwitterArticleRequest::TwitterArticleRequest(QObject *parent) :
    ArticleRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString TwitterArticleRequest::errorString() const {
    return m_errorString;
}

void TwitterArticleRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

ArticleResult TwitterArticleRequest::result() const {
    return m_result;
}

void TwitterArticleRequest::setResult(const ArticleResult &r) {
    m_result = r;
}

ArticleRequest::Status TwitterArticleRequest::status() const {
    return m_status;
}

void TwitterArticleRequest::setStatus(ArticleRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool TwitterArticleRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool TwitterArticleRequest::getArticle(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(ArticleResult());
    m_settings = settings;
    m_redirects = 0;
    QString u = url;
    u.replace("mobile.twitter.com", "twtter.com");
#ifdef TWITTER_DEBUG
    qDebug() << "TwitterArticleRequest::getArticle(). URL:" << u;
#endif
    QNetworkRequest request(u);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkArticle()));
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void TwitterArticleRequest::checkArticle() {
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

    const QHtmlDocument document(QString::fromUtf8(reply->readAll()));
    const QHtmlElement html = document.htmlElement();
    const bool includeImages = m_settings.value("includeImages", true).toBool();
    const bool includeReplies = m_settings.value("includeReplies", true).toBool();
    writeArticleAuthor(html);
    writeArticleBody(html, includeImages, includeReplies);
    writeArticleDate(html);
    writeArticleTitle(html);
    writeArticleUrl(reply->url().toString());
    setStatus(Ready);
    emit finished(this);
}

void TwitterArticleRequest::followRedirect(const QString &url, const char *slot) {
#ifdef TWITTER_DEBUG
    qDebug() << "TwitterArticleRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(ArticleRequest*)), reply, SLOT(deleteLater()));
}

void TwitterArticleRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString TwitterArticleRequest::getRedirect(const QNetworkReply *reply) {
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

void TwitterArticleRequest::writeArticleAuthor(const QHtmlElement &element) {
    const QHtmlElement tweet = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "original-tweet",
                QHtmlParser::MatchContains));

    if (!tweet.isNull()) {
        m_result.author = QString("%1 (@%2)").arg(tweet.attribute("data-name"))
            .arg(tweet.attribute("data-screen-name"));
    }
}

void TwitterArticleRequest::writeArticleBody(const QHtmlElement &element, bool includeImages, bool includeReplies) {
    const QHtmlElement tweet = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "original-tweet",
                QHtmlParser::MatchContains));
    
    if (!tweet.isNull()) {
        writeTweetToArticleBody(tweet, includeImages);

        if (includeReplies) {
            const QHtmlElementList replies = element.elementsByTagName("div",
                    QHtmlAttributeMatch("class", "descendant-tweet", QHtmlParser::MatchContains));

            foreach (const QHtmlElement &reply, replies) {
                if (!reply.attribute("class").contains("promoted")) {
                    writeTweetToArticleBody(reply, includeImages);
                }
            }
        }
    }
}

void TwitterArticleRequest::writeTweetToArticleBody(const QHtmlElement &tweet, bool includeImages) {
    QString body;
    const QString user = QString("%1<br>@%2").arg(tweet.attribute("data-name"))
        .arg(tweet.attribute("data-screen-name"));
    const QString text = tweet.firstElementByTagName("p", QHtmlAttributeMatch("class", "js-tweet-text",
                QHtmlParser::MatchContains)).toString();

    if (includeImages) {
        const QHtmlElement avatar = tweet.firstElementByTagName("img", QHtmlAttributeMatch("class", "avatar",
                    QHtmlParser::MatchStartsWith));

        if (!avatar.isNull()) {
            body.append(QString("<div><img align=\"left\" src=\"%1\"><div style=\"margin-left: 81px\">%2</div></div>")
                    .arg(avatar.attribute("src")).arg(user));
            body.append(QString("<div style=\"clear: both; margin-top: 8px;\">%1</div>").arg(text));
        }
        else {
            body.append(user);
            body.append(text);
        }

        const QHtmlElement media = tweet.firstElementByTagName("div", QHtmlAttributeMatch("class",
                    "AdaptiveMediaOuterContainer"));

        if (!media.isNull()) {
            foreach (const QHtmlElement &image, media.elementsByTagName("img")) {
                body.append(QString("<div style=\"clear: both\"><img style=\"margin-top: 8px\" src=\"%1\"></div>")
                        .arg(image.attribute("src")));
            }
        }
    }
    else {
        body.append(user);
        body.append(text);
    }

    fixRelativeUrls(body, BASE_URL);
    m_result.body.append(QString("<div style=\"clear: both; margin-top: 16px;\">%1</div>").arg(body));
}

void TwitterArticleRequest::writeArticleDate(const QHtmlElement &element) {
    const QHtmlElement tweet = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "original-tweet",
                QHtmlParser::MatchContains));

    if (!tweet.isNull()) {
        const QHtmlElement date = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "timestamp",
                    QHtmlParser::MatchContains));

        if (!date.isNull()) {
            m_result.date = QDateTime::fromTime_t(date.attribute("data-time").toUInt());
        }
    }
}

void TwitterArticleRequest::writeArticleTitle(const QHtmlElement &element) {
    m_result.title = element.firstElementByTagName("meta", QHtmlAttributeMatch("property", "og:title"))
        .attribute("content");
}

void TwitterArticleRequest::writeArticleUrl(const QString &url) {
    m_result.url = url;
}

QNetworkAccessManager* TwitterArticleRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
