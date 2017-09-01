/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "bbcfeedrequest.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef BBC_DEBUG
#include <QDebug>
#endif

const int BbcFeedRequest::MAX_REDIRECTS = 8;

const QString BbcFeedRequest::BASE_URL("http://feeds.bbci.co.uk");
const QString BbcFeedRequest::ICON_URL("http://static.bbci.co.uk/news/1.207.11915/apple-touch-icon-72x72-precomposed.png");

const QByteArray BbcFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

BbcFeedRequest::BbcFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString BbcFeedRequest::errorString() const {
    return m_errorString;
}

void BbcFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray BbcFeedRequest::result() const {
    return m_buffer.data();
}

BbcFeedRequest::Status BbcFeedRequest::status() const {
    return m_status;
}

void BbcFeedRequest::setStatus(BbcFeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool BbcFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool BbcFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    m_settings = settings;
    m_results = 0;
    m_redirects = 0;
    QString url(BASE_URL);
    const QString section = m_settings.value("section").toString();

    if (!section.isEmpty()) {
        url.append("/");
        url.append(section);
    }

    url.append("/rss.xml");
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::getFeed(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkFeed()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void BbcFeedRequest::checkFeed() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        emit finished(this);
        return;
    }

    QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        reply->deleteLater();
        
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect, SLOT(checkFeed()));
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
        setErrorString(QString());
        setStatus(Canceled);
        emit finished(this);
        return;
    default:
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        return;
    }
    
    m_parser.setContent(reply->readAll());
    reply->deleteLater();

    if (m_parser.readChannel()) {
#ifdef BBC_DEBUG
        qDebug() << "BbcFeedRequest::checkFeed(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(m_parser.title());
        writeFeedUrl(m_parser.url());

        if ((m_parser.readNextArticle()) && (m_parser.date() > m_settings.value("lastUpdated").toDateTime())) {
            getPage(m_parser.url());
            return;
        }
#ifdef BBC_DEBUG
        qDebug() << "BbcFeedRequest::checkFeed(). Writing end of feed";
#endif
        writeEndFeed();
    }
    
    setErrorString(m_parser.errorString());
    setStatus(Error);
    emit finished(this);
}

void BbcFeedRequest::getPage(const QString &url) {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void BbcFeedRequest::checkPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        emit finished(this);
        return;
    }

    QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        reply->deleteLater();
        
        if (m_redirects < MAX_REDIRECTS) {
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
        setErrorString(QString());
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
    reply->deleteLater();
    fixRelativeUrls(page, baseUrl);
    const int max = m_settings.value("maxResults", 20).toInt();
    ++m_results;
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    writeStartItem();
    writeItemBody(html);
    writeItemDate(m_parser.date());
    writeItemTitle(m_parser.title());
    writeItemUrl(m_parser.url());
    writeEndItem();
    
    if ((m_results < max) && (m_parser.readNextArticle())
            && (m_parser.date() > m_settings.value("lastUpdated").toDateTime())) {
        getPage(m_parser.url());
        return;
    }
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::checkPage(). Writing end of feed";
#endif
    writeEndFeed();
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

void BbcFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::followRedirect(). URL:" << url;
#endif
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void BbcFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString BbcFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString BbcFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void BbcFeedRequest::writeStartFeed() {
    m_buffer.close();
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.setAutoFormatting(true);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("News articles from the BBC"));
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
}

void BbcFeedRequest::writeEndFeed() {
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void BbcFeedRequest::writeFeedTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void BbcFeedRequest::writeFeedUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

void BbcFeedRequest::writeStartItem() {
    m_writer.writeStartElement("item");
}

void BbcFeedRequest::writeEndItem() {
    m_writer.writeEndElement();
}

void BbcFeedRequest::writeItemBody(const QHtmlElement &element) {
    const QHtmlElement body = element.firstElementByTagName("div", QHtmlAttributeMatches()
        << QHtmlAttributeMatch("property", "articleBody") << QHtmlAttributeMatch("class", "main_article_text")
        << QHtmlAttributeMatch("id", "story-body") << QHtmlAttributeMatch("class", "vxp-media__summary"),
        QHtmlParser::MatchAny);

    QRegExp figure("<figure.*</figure>");
    figure.setMinimal(true);
    QRegExp caption("<figcaption.*</figcaption>");
    caption.setMinimal(true);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body.toString().remove(figure).remove(caption));
    m_writer.writeEndElement();
}

void BbcFeedRequest::writeItemDate(const QDateTime &date) {
    m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
}

void BbcFeedRequest::writeItemTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void BbcFeedRequest::writeItemUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

QNetworkAccessManager* BbcFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
