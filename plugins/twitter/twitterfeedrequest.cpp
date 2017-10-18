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
#include <QDateTime>
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
    else {
        getPage(HASHTAG_URL + query.mid(1));
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

    foreach (const QHtmlElement &item, getItems(html)) {
        writeStartItem();
        writeItemAuthor(item);
        writeItemBody(item, includeImages);
        writeItemCategories(item);
        writeItemDate(item);
        writeItemEnclosures(item);
        writeItemTitle(item);
        writeItemUrl(item);
        writeEndItem();      
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
    QHtmlElementList items = element.elementsByTagName("div", QHtmlAttributeMatch("class", "js-stream-tweet",
                QHtmlParser::MatchContains));

    for (int i = items.size() - 1; i >= 0; i--) {
        if (items.at(i).attribute("class").contains("promoted-tweet")) {
            items.removeAt(i);
        }
    }

    return items;
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
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(element.firstElementByTagName("title").text());
    m_writer.writeEndElement();
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

void TwitterFeedRequest::writeStartItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void TwitterFeedRequest::writeEndItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TwitterFeedRequest::writeItemAuthor(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:creator", QString("%1 @%2").arg(element.attribute("data-name"))
            .arg(element.attribute("data-screen-name")));
    m_buffer.close();
}

void TwitterFeedRequest::writeItemBody(const QHtmlElement &element, bool includeImages) {
    QString body;
    const QHtmlElement retweet = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "js-retweet-text"));

    if (!retweet.isNull()) {
        body.append(QString("<p>%1</p>").arg(retweet.toString()));
    }

    const QString tweet = element.firstElementByTagName("p", QHtmlAttributeMatch("class", "js-tweet-text",
                QHtmlParser::MatchContains)).toString();

    if (includeImages) {
        const QHtmlElement avatar = element.firstElementByTagName("img", QHtmlAttributeMatch("class", "avatar",
                    QHtmlParser::MatchStartsWith));

        if (!avatar.isNull()) {
            body.append(QString("<div><img align=\"left\" src=\"%1\"><div style=\"margin-left: 81px\">%2</div></div>")
                    .arg(avatar.attribute("src")).arg(tweet));
        }
        else {
            body.append(tweet);
        }

        const QHtmlElement media = element.firstElementByTagName("div", QHtmlAttributeMatch("class",
                    "AdaptiveMediaOuterContainer"));

        if (!media.isNull()) {
            foreach (const QHtmlElement &image, media.elementsByTagName("img")) {
                body.append(QString("<div style=\"clear: both\"><img style=\"margin-top: 8px\" src=\"%1\"></div>")
                        .arg(image.attribute("src")));
            }
        }
    }
    else {
        body.append(tweet);
    }

    fixRelativeUrls(body, BASE_URL);
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TwitterFeedRequest::writeItemCategories(const QHtmlElement &) {}

void TwitterFeedRequest::writeItemDate(const QHtmlElement &element) {
    const QHtmlElement date = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "timestamp",
                QHtmlParser::MatchContains));

    if (!date.isNull()) {
        m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
        m_writer.writeTextElement("dc:date", QDateTime::fromTime_t(date.attribute("data-time").toUInt())
                .toString(Qt::ISODate));
        m_buffer.close();
    }
}

void TwitterFeedRequest::writeItemEnclosures(const QHtmlElement &) {}

void TwitterFeedRequest::writeItemTitle(const QHtmlElement &element) {
    QString author = element.attribute("data-retweeter");
    QString title;

    if (!author.isEmpty()) {
        title = tr("Retweet from @%1").arg(author);
    }
    else {
        author = element.attribute("data-screen-name");
        title = tr("Tweet from @%1").arg(author);
    }

    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(title);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TwitterFeedRequest::writeItemUrl(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", BASE_URL + element.attribute("data-permalink-path"));
    m_buffer.close();
}

QNetworkAccessManager* TwitterFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}