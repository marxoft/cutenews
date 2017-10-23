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
    QUrl u(url);
    u.setHost("mobile.twitter.com");
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
    const QHtmlElement tweet = element.firstElementByTagName("table", QHtmlAttributeMatch("class", "main-tweet"));

    if (!tweet.isNull()) {
        QString author;
        const QHtmlElement fullname = tweet.firstElementByTagName("div", QHtmlAttributeMatch("class", "fullname"));

        if (!fullname.isNull()) {
            author = fullname.firstElementByTagName("strong").text();
        }

        const QHtmlElement username = tweet.firstElementByTagName("span", QHtmlAttributeMatch("class", "username"));

        if (!username.isNull()) {
            if (author.isEmpty()) {
                author = username.text(true).trimmed().remove("\n");
            }
            else {
                author.append(QString(" (%1)").arg(username.text(true).trimmed().remove("\n") ));
            }
        }

        m_result.author = author;
    }
}

void TwitterArticleRequest::writeArticleBody(const QHtmlElement &element, bool includeImages, bool includeReplies) {
    const QHtmlElement tweet = element.firstElementByTagName("table", QHtmlAttributeMatch("class", "main-tweet"));
    
    if (!tweet.isNull()) {
        writeTweetToArticleBody(tweet, includeImages);

        if (includeReplies) {
            const QHtmlElementList replies =
                element.firstElementByTagName("div", QHtmlAttributeMatch("class", "timeline replies"))
                        .elementsByTagName("table", QHtmlAttributeMatch("class", "tweet"));

            foreach (const QHtmlElement &reply, replies) {
                writeTweetToArticleBody(reply, includeImages);
            }
        }
    }
}

void TwitterArticleRequest::writeTweetToArticleBody(const QHtmlElement &tweet, bool includeImages) {
    QString user;
    QString media;
    QString text = QString("<p style=\"clear: both\">%1</p>")
        .arg(tweet.firstElementByTagName("div", QHtmlAttributeMatch("class", "tweet-text")).toString());
    const QHtmlElement info = tweet.firstElementByTagName("td", QHtmlAttributeMatch("class", "user-info"));

    if (!info.isNull()) {
        const QHtmlElement fullname = info.firstElementByTagName("strong");

        if (!fullname.isNull()) {
            user.append(fullname.text());
        }

        QHtmlElement username = info.firstElementByTagName("span", QHtmlAttributeMatch("class", "username"));

        if (!username.isNull()) {
            if (!user.isEmpty()) {
                user.append("<br>");
            }

            user.append(username.text(true).trimmed().remove("\n"));
        }
        else {
            username = info.firstElementByTagName("div", QHtmlAttributeMatch("class", "username"));

            if (!username.isNull()) {
                if (!user.isEmpty()) {
                    user.append("<br>");
                }

                user.append(username.text(true).trimmed().remove("\n"));
            }
        }
    }

    if (includeImages) {
        const QHtmlElement avatarEl = tweet.firstElementByTagName("td", QHtmlAttributeMatch("class", "avatar"))
            .firstElementByTagName("img");

        if (!avatarEl.isNull()) {
            user = QString("<div><img align=\"left\" src=\"%1\"><div>%2</div></div>")
                    .arg(avatarEl.attribute("src")).arg(user);
        }

        const QHtmlElement mediaEl = tweet.firstElementByTagName("div", QHtmlAttributeMatch("class", "media"));

        if (!mediaEl.isNull()) {
            foreach (const QHtmlElement &image, mediaEl.elementsByTagName("img")) {
                media.append(QString("<div style=\"clear: both\"><img style=\"margin-top: 8px\" src=\"%1\"></div>")
                        .arg(image.attribute("src")));
            }
        }
    }

    fixRelativeUrls(text, BASE_URL);
    m_result.body.append(QString("<div style=\"clear: both; margin-top: 16px;\">%1%2%3</div>")
            .arg(user).arg(text).arg(media));
}

void TwitterArticleRequest::writeArticleDate(const QHtmlElement &element) {
    const QHtmlElement tweet = element.firstElementByTagName("table", QHtmlAttributeMatch("class", "main-tweet"));

    if (!tweet.isNull()) {
        const QHtmlElement metadata = tweet.firstElementByTagName("div", QHtmlAttributeMatch("class", "metadata"));

        if (!metadata.isNull()) {
            m_result.date = QDateTime::fromString(metadata.firstElementByTagName("a").text(), "h:mm ap - dd MMM yyyy");
        }
    }
}

void TwitterArticleRequest::writeArticleTitle(const QHtmlElement &element) {
    const QHtmlElement tweet = element.firstElementByTagName("table", QHtmlAttributeMatch("class", "main-tweet"));

    if (!tweet.isNull()) {
        const QHtmlElement username = tweet.firstElementByTagName("span", QHtmlAttributeMatch("class", "username"));

        if (!username.isNull()) {
            m_result.title = tr("Tweet by %1").arg(username.text(true).trimmed().remove("\n"));
        }
    }
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
