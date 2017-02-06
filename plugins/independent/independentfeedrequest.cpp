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

#include "independentfeedrequest.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef INDEPENDENT_DEBUG
#include <QDebug>
#endif

const int IndependentFeedRequest::MAX_REDIRECTS = 8;

const QString IndependentFeedRequest::BASE_URL("http://www.independent.co.uk");
const QString IndependentFeedRequest::ICON_URL("http://www.independent.co.uk/sites/all/themes/ines_themes/independent_theme/img/apple-icon-72x72.png");

const QByteArray IndependentFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

IndependentFeedRequest::IndependentFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString IndependentFeedRequest::errorString() const {
    return m_errorString;
}

void IndependentFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray IndependentFeedRequest::result() const {
    return m_buffer.data();
}

IndependentFeedRequest::Status IndependentFeedRequest::status() const {
    return m_status;
}

void IndependentFeedRequest::setStatus(IndependentFeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool IndependentFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool IndependentFeedRequest::getFeed(const QVariantMap &settings) {
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

    url.append("/rss");
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentFeedRequest::getFeed(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkFeed()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void IndependentFeedRequest::checkFeed() {
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
#ifdef INDEPENDENT_DEBUG
        qDebug() << "IndependentFeedRequest::checkFeed(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(m_parser.title());
        writeFeedUrl(m_parser.url());

        if (m_parser.readNextArticle()) {
            getPage(m_parser.url());
            return;
        }
#ifdef INDEPENDENT_DEBUG
        qDebug() << "IndependentFeedRequest::checkFeed(). Parser error. Writing end of feed";
#endif
        writeEndFeed();
    }
    
    setErrorString(m_parser.errorString());
    setStatus(Error);
    emit finished(this);
}

void IndependentFeedRequest::getPage(const QString &url) {
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void IndependentFeedRequest::checkPage() {
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
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    writeStartItem();
    writeItemAuthor(m_parser.author());
    writeItemBody(html);
    writeItemDate(m_parser.date());
    writeItemTitle(m_parser.title());
    writeItemUrl(m_parser.url());
    writeEndItem();
    
    if (m_results < max) {
        if (m_parser.readNextArticle()) {
            getPage(m_parser.url());
            return;
        }
    }
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentFeedRequest::checkPage(). Writing end of feed";
#endif
    writeEndFeed();
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

void IndependentFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef INDEPENDENT_DEBUG
    qDebug() << "IndependentFeedRequest::followRedirect(). URL:" << url;
#endif
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void IndependentFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString IndependentFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString IndependentFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void IndependentFeedRequest::writeStartFeed() {
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
    m_writer.writeTextElement("description", tr("News articles from The Independent"));
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
}

void IndependentFeedRequest::writeEndFeed() {
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void IndependentFeedRequest::writeFeedTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void IndependentFeedRequest::writeFeedUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

void IndependentFeedRequest::writeStartItem() {
    m_writer.writeStartElement("item");
}

void IndependentFeedRequest::writeEndItem() {
    m_writer.writeEndElement();
}

void IndependentFeedRequest::writeItemAuthor(const QString &author) {
    m_writer.writeTextElement("dc:creator", author);
}

void IndependentFeedRequest::writeItemBody(const QHtmlElement &element) {
    const QHtmlElement bodyElement = element.firstElementByTagName("div",
                                                                   QHtmlAttributeMatch("itemprop", "articleBody"));
    QString body = bodyElement.toString();
    
    foreach (const QHtmlElement &child, bodyElement.childElements()) {
        if (child.tagName() == "div") {
            body.remove(child.toString());
        }
    }

    foreach (const QHtmlElement &figure, bodyElement.elementsByTagName("figure")) {
        body.remove(figure.toString());
    }

    foreach (const QHtmlElement &image, bodyElement.elementsByTagName("img")) {
        body.remove(image.toString());
    }
    
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body);
    m_writer.writeEndElement();
}

void IndependentFeedRequest::writeItemDate(const QDateTime &date) {
    m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
}

void IndependentFeedRequest::writeItemTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void IndependentFeedRequest::writeItemUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

QNetworkAccessManager* IndependentFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
