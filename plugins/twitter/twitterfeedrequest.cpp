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

#include "twitterfeedrequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef TWITTER_DEBUG
#include <QDebug>
#endif

const int TwitterFeedRequest::MAX_REDIRECTS = 8;

const QString TwitterFeedRequest::BASE_URL("https://twitter.com");
const QString TwitterFeedRequest::HASHTAG_URL(BASE_URL + "/hashtag/");
const QString TwitterFeedRequest::USER_URL(BASE_URL + "/");
const QString TwitterFeedRequest::ICON_URL("https://abs.twimg.com/icons/apple-touch-icon-192x192.png");

const QByteArray TwitterFeedRequest::USER_AGENT("Mozilla/5.0 (X11; Linux x86_64; rv:53.0) Gecko/20100101 Firefox/53.0");

TwitterFeedRequest::TwitterFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString TwitterFeedRequest::errorString() const {
    return m_errorString;
}

void TwitterFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
#ifdef TWITTER_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "TwitterDebug::error." << e;
    }
#endif
}

QByteArray TwitterFeedRequest::result() const {
    return m_buffer.data();
}

void TwitterFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status TwitterFeedRequest::status() const {
    return m_status;
}

void TwitterFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool TwitterFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool TwitterFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(QByteArray());
    m_settings = settings;
    const QString query = settings.value("query", "#qt").toString();

    if (query.startsWith("@")) {
        getPage(USER_URL + query.mid(1));
    }
    else if (query.startsWith("#")) {
        getPage(HASHTAG_URL + query.mid(1));
    }
    else {
        getPage(HASHTAG_URL + query);
    }

    return true;
}

void TwitterFeedRequest::getPage(const QString &url) {
#ifdef TWITTER_DEBUG
    qDebug() << "TwitterFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void TwitterFeedRequest::checkPage() {
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
            followRedirect(redirect, SLOT(checkPage()));
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
    writeStartFeed(html);
    QList<Tweet> tweets;

    foreach (const QHtmlElement &item, getItems(html)) {
        if (!item.attribute("class").contains("promoted")) {
            Tweet tweet;
            tweet.author = getItemAuthor(item);
            tweet.body = getItemBody(item, includeImages);
            tweet.date = getItemDate(item);
            tweet.title = getItemTitle(item);
            tweet.url = getItemUrl(item);
            tweets << tweet;
        }
    }

    if (!tweets.isEmpty()) {
        qSort(tweets.begin(), tweets.end());
        writeTweets(tweets);
    }

    writeEndFeed();
    setStatus(Ready);
    emit finished(this);
}

void TwitterFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef TWITTER_DEBUG
    qDebug() << "TwitterFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void TwitterFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString TwitterFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QHtmlElementList TwitterFeedRequest::getItems(const QHtmlElement &element) {
    return element.elementsByTagName("div", QHtmlAttributeMatch("class", "js-stream-tweet",
                QHtmlParser::MatchContains));
}

QString TwitterFeedRequest::getItemAuthor(const QHtmlElement &element) {
    return QString("%1 (@%2)").arg(element.attribute("data-name").remove(QRegExp("[^\\w\\s-_@\\(\\)]")).simplified())
        .arg(element.attribute("data-screen-name"));
}

QString TwitterFeedRequest::getItemBody(const QHtmlElement &element, bool includeImages) {
    QString body;
    const QString user = QString("<a href='%1/%2'>%3<br>@%2</a>").arg(BASE_URL)
        .arg(element.attribute("data-screen-name"))
        .arg(element.attribute("data-name").remove(QRegExp("[^\\w\\s-_@\\(\\)]+")).simplified());
    QString reply = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "ReplyingToContextBelowAuthor"))
        .toString();

    if (reply.contains("<button")) {
        reply = reply.left(reply.lastIndexOf("</a>") + 4);
    }

    QString text = reply + element.firstElementByTagName("p", QHtmlAttributeMatch("class", "js-tweet-text",
        QHtmlParser::MatchContains)).toString();
    QString date;
    const QHtmlElement dateEl = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "timestamp",
                QHtmlParser::MatchContains));

    if (!dateEl.isNull()) {
        date = QDateTime::fromTime_t(dateEl.attribute("data-time").toUInt()).toString("dd MMM yyyy @ HH:mm");
    }

    if (includeImages) {
        QString avatar;
        const QHtmlElement avatarEl = element.firstElementByTagName("img", QHtmlAttributeMatch("class", "avatar",
                    QHtmlParser::MatchStartsWith));

        if (!avatarEl.isNull()) {
            avatar = avatarEl.attribute("src");
        }

        QString media;
        const QHtmlElement mediaEl = element.firstElementByTagName("div", QHtmlAttributeMatch("class",
                    "AdaptiveMediaOuterContainer"));

        if (!mediaEl.isNull()) {
            foreach (const QHtmlElement &image, mediaEl.elementsByTagName("img")) {
                media.append(QString("<img class='cutenews-tweet-media' src='%1'>").arg(image.attribute("src")));
            }
        }

        body = TWEET_HTML_INCLUDE_IMAGES.arg(avatar).arg(user).arg(text).arg(media).arg(tr("Posted on %1").arg(date));
    }
    else {
        text.remove(QRegExp("<img[^>]+>"));
        body = TWEET_HTML.arg(user).arg(text).arg(tr("Posted on %1").arg(date));
    }

    fixRelativeUrls(body, BASE_URL);

    return QString("<style>%1</style>%2").arg(includeImages ? TWEET_STYLESHEET_INCLUDE_IMAGES : TWEET_STYLESHEET)
        .arg(body);
}

QDateTime TwitterFeedRequest::getItemDate(const QHtmlElement &element) {
    const QHtmlElement date = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "timestamp",
                QHtmlParser::MatchContains));
    return QDateTime::fromTime_t(date.attribute("data-time").toUInt());
}

QString TwitterFeedRequest::getItemTitle(const QHtmlElement &element) {
    return tr("%1 (@%2) on Twitter")
        .arg(element.attribute("data-name").remove(QRegExp("[^\\w\\s-_@\\(\\)]+")).simplified())
        .arg(element.attribute("data-screen-name"));
}

QString TwitterFeedRequest::getItemUrl(const QHtmlElement &element) {
    return BASE_URL + element.attribute("data-permalink-path");
}

void TwitterFeedRequest::writeStartFeed(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("link", element.firstElementByTagName("link", QHtmlAttributeMatch("rel", "canonical"))
            .attribute("href"));
    m_writer.writeTextElement("title", element.firstElementByTagName("title").text()
            .remove(QRegExp("[^\\w\\s-_@\\(\\)]+")).simplified());
    m_writer.writeStartElement("description");
    m_writer.writeCDATA(element.firstElementByTagName("meta", QHtmlAttributeMatch("name", "description"))
                .attribute("content"));
    m_writer.writeEndElement();
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TwitterFeedRequest::writeEndFeed() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void TwitterFeedRequest::writeTweets(const QList<Tweet> &tweets) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const Tweet &tweet, tweets) {
        m_writer.writeStartElement("item");
        m_writer.writeTextElement("dc:creator", tweet.author);
        m_writer.writeStartElement("content:encoded");
        m_writer.writeCDATA(tweet.body);
        m_writer.writeEndElement();
        m_writer.writeTextElement("dc:date", tweet.date.toString(Qt::ISODate));
        m_writer.writeTextElement("title", tweet.title);
        m_writer.writeTextElement("link", tweet.url);
        m_writer.writeEndElement();
    }

    m_buffer.close();
}

QNetworkAccessManager* TwitterFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
