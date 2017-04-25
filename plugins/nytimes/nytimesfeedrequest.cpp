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

#include "nytimesfeedrequest.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef NYTIMES_DEBUG
#include <QDebug>
#endif

const int NytimesFeedRequest::MAX_REDIRECTS = 8;

const QString NytimesFeedRequest::BASE_URL("http://rss.nytimes.com/services/xml/rss/nyt/");
const QString NytimesFeedRequest::ICON_URL("https://static01.nyt.com/images/icons/ios-ipad-144x144.png");

const QByteArray NytimesFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

NytimesFeedRequest::NytimesFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString NytimesFeedRequest::errorString() const {
    return m_errorString;
}

void NytimesFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray NytimesFeedRequest::result() const {
    return m_buffer.data();
}

NytimesFeedRequest::Status NytimesFeedRequest::status() const {
    return m_status;
}

void NytimesFeedRequest::setStatus(NytimesFeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool NytimesFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool NytimesFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    m_settings = settings;
    m_results = 0;
    m_redirects = 0;
    const QString url(BASE_URL + m_settings.value("section").toString() + ".xml");
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesFeedRequest::getFeed(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkFeed()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void NytimesFeedRequest::checkFeed() {
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
#ifdef NYTIMES_DEBUG
        qDebug() << "NytimesFeedRequest::checkFeed(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(m_parser.title());
        writeFeedUrl(m_parser.url());

        if ((m_parser.readNextArticle()) && (m_parser.date() > m_settings.value("lastUpdated").toDateTime())) {
            getPage(m_parser.url().replace("www.nytimes.com", "mobile.nytimes.com"));
            return;
        }
#ifdef NYTIMES_DEBUG
        qDebug() << "NytimesFeedRequest::checkFeed(). Writing end of feed";
#endif
        writeEndFeed();
    }
    
    setErrorString(m_parser.errorString());
    setStatus(Error);
    emit finished(this);
}

void NytimesFeedRequest::getPage(const QString &url) {
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void NytimesFeedRequest::checkPage() {
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
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    writeStartItem();
    writeItemAuthor(m_parser.author());
    writeItemBody(html);
    writeItemCategories(m_parser.categories());
    writeItemDate(m_parser.date());
    writeItemTitle(m_parser.title());
    writeItemUrl(m_parser.url());
    writeEndItem();
    
    if ((m_results < max) && (m_parser.readNextArticle())
            && (m_parser.date() > m_settings.value("lastUpdated").toDateTime())) {
        getPage(m_parser.url().replace("www.nytimes.com", "mobile.nytimes.com"));
        return;
    }
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesFeedRequest::checkPage(). Writing end of feed";
#endif
    writeEndFeed();
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

void NytimesFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesFeedRequest::followRedirect(). URL:" << url;
#endif
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void NytimesFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString NytimesFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString NytimesFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void NytimesFeedRequest::writeStartFeed() {
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
    m_writer.writeTextElement("description", tr("News articles from The New York Times"));
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
}

void NytimesFeedRequest::writeEndFeed() {
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void NytimesFeedRequest::writeFeedTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void NytimesFeedRequest::writeFeedUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

void NytimesFeedRequest::writeStartItem() {
    m_writer.writeStartElement("item");
}

void NytimesFeedRequest::writeEndItem() {
    m_writer.writeEndElement();
}

void NytimesFeedRequest::writeItemAuthor(const QString &author) {
    m_writer.writeTextElement("dc:creator", author);
}

void NytimesFeedRequest::writeItemBody(const QHtmlElement &element) {
    const QHtmlElement bodyElement = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "article-body"));
    QString body = bodyElement.toString();
    
    foreach (const QHtmlElement &div, bodyElement.elementsByTagName("div")) {
        body.remove(div.toString());
    }
    
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body);
    m_writer.writeEndElement();
}

void NytimesFeedRequest::writeItemCategories(const QStringList &categories) {
    foreach (const QString &category, categories) {
        m_writer.writeTextElement("category", category);
    }
}

void NytimesFeedRequest::writeItemDate(const QDateTime &date) {
    m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
}

void NytimesFeedRequest::writeItemTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void NytimesFeedRequest::writeItemUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

QNetworkAccessManager* NytimesFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
