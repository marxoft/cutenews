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

#include "guardianfeedrequest.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef GUARDIAN_DEBUG
#include <QDebug>
#endif

const int GuardianFeedRequest::MAX_REDIRECTS = 8;

const QString GuardianFeedRequest::BASE_URL("https://www.theguardian.com");
const QString GuardianFeedRequest::ICON_URL("https://assets.guim.co.uk/images/favicons/b5050517955e7cf1e493ccc53e64ca05/72x72.png");

const QByteArray GuardianFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

GuardianFeedRequest::GuardianFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString GuardianFeedRequest::errorString() const {
    return m_errorString;
}

void GuardianFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray GuardianFeedRequest::result() const {
    return m_buffer.data();
}

GuardianFeedRequest::Status GuardianFeedRequest::status() const {
    return m_status;
}

void GuardianFeedRequest::setStatus(GuardianFeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool GuardianFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool GuardianFeedRequest::getFeed(const QVariantMap &settings) {
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
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkFeed()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void GuardianFeedRequest::checkFeed() {
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
#ifdef GUARDIAN_DEBUG
        qDebug() << "GuardianFeedRequest::checkFeed(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(m_parser.title());
        writeFeedUrl(m_parser.url());

        if ((m_parser.readNextArticle()) && (m_parser.date() > m_settings.value("lastUpdated").toDateTime())) {
            getPage(m_parser.url());
            return;
        }
#ifdef GUARDIAN_DEBUG
        qDebug() << "GuardianFeedRequest::checkFeed(). Writing end of feed";
#endif
        writeEndFeed();
    }
    
    setErrorString(m_parser.errorString());
    setStatus(Error);
    emit finished(this);
}

void GuardianFeedRequest::getPage(const QString &url) {
#ifdef GUARDIAN_DEBUG
    qDebug() << "GuardianFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void GuardianFeedRequest::checkPage() {
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
#ifdef GUARDIAN_DEBUG
    qDebug() << "GuardianFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
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
        getPage(m_parser.url());
        return;
    }
#ifdef GUARDIAN_DEBUG
    qDebug() << "GuardianFeedRequest::checkPage(). Writing end of feed";
#endif
    writeEndFeed();
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

void GuardianFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef GUARDIAN_DEBUG
    qDebug() << "GuardianFeedRequest::followRedirect(). URL:" << url;
#endif
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void GuardianFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString GuardianFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString GuardianFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void GuardianFeedRequest::writeStartFeed() {
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
    m_writer.writeTextElement("description", tr("News articles from The Guardian"));
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
}

void GuardianFeedRequest::writeEndFeed() {
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void GuardianFeedRequest::writeFeedTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void GuardianFeedRequest::writeFeedUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

void GuardianFeedRequest::writeStartItem() {
    m_writer.writeStartElement("item");
}

void GuardianFeedRequest::writeEndItem() {
    m_writer.writeEndElement();
}

void GuardianFeedRequest::writeItemAuthor(const QString &author) {
    m_writer.writeTextElement("dc:creator", author);
}

void GuardianFeedRequest::writeItemBody(const QHtmlElement &element) {
    QRegExp figure("(<figure .*</figure>|<aside .*</aside>|<div class=\"(block-|)share.*</div>|<button .*</button>)");
    figure.setMinimal(true);
    QString body = element.firstElementByTagName("div", QHtmlAttributeMatches()
                                                 << QHtmlAttributeMatch("itemprop", "articleBody")
                                                 << QHtmlAttributeMatch("data-test-id", "article-review-body"),
                                                 QHtmlParser::MatchAny).toString();
    body.remove(figure);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body);
    m_writer.writeEndElement();
}

void GuardianFeedRequest::writeItemCategories(const QStringList &categories) {
    foreach (const QString &category, categories) {
        m_writer.writeTextElement("category", category);
    }
}

void GuardianFeedRequest::writeItemDate(const QDateTime &date) {
    m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
}

void GuardianFeedRequest::writeItemTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
}

void GuardianFeedRequest::writeItemUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

QNetworkAccessManager* GuardianFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
