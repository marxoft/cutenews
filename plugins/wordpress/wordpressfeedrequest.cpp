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

#include "wordpressfeedrequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef WORDPRESS_DEBUG
#include <QDebug>
#endif

const int WordpressFeedRequest::MAX_REDIRECTS = 8;

const QByteArray WordpressFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

WordpressFeedRequest::WordpressFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString WordpressFeedRequest::errorString() const {
    return m_errorString;
}

void WordpressFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
#ifdef WORDPRESS_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "WordpressDebug::error." << e;
    }
#endif
}

QByteArray WordpressFeedRequest::result() const {
    return m_buffer.data();
}

void WordpressFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status WordpressFeedRequest::status() const {
    return m_status;
}

void WordpressFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool WordpressFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool WordpressFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(QByteArray());
    m_settings = settings;
    m_results = 0;
    getPage(settings.value("url").toString());
    return true;
}

void WordpressFeedRequest::getPage(const QString &url) {
#ifdef WORDPRESS_DEBUG
    qDebug() << "WordpressFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void WordpressFeedRequest::checkPage() {
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

    const QUrl url = reply->url();
    const QString baseUrl = url.scheme() + "://" + url.authority();
    QString page = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    fixRelativeUrls(page, baseUrl);
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    
    if (m_results == 0) {
        const QString redirect = getLatestPageUrl(html);

        if (!redirect.isEmpty()) {
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
#ifdef WORDPRESS_DEBUG
        qDebug() << "WordpressFeedRequest::checkPage(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(html);
        writeFeedUrl(baseUrl);
    }
    
    const QHtmlElementList items = getItems(html);

    if (items.isEmpty()) {
#ifdef WORDPRESS_DEBUG
        qDebug() << "WordpressFeedRequest::checkPage(). No items found. Writing end of feed";
#endif
        writeEndFeed();
        setStatus(Ready);
        emit finished(this);
        return;
    }
    
    const bool fetchFullArticles = m_settings.value("fetchFullArticles", false).toBool();
    const int max = m_settings.value("maxResults", 20).toInt();
    const int count = items.size();
    int i = 0;
    
    if (fetchFullArticles) {
        while ((i < count) && (m_results < max)) {
            const QHtmlElement &item = items.at(i);
            ++i;
            ++m_results;
            const QString articleUrl = item.firstElementByTagName("a", QHtmlAttributeMatch("rel", "bookmark"))
                                           .attribute("href");
#ifdef WORDPRESS_DEBUG
            qDebug() << "WordpressFeedRequest::checkPage(). Adding full article URL:" << articleUrl;
#endif
            if (!articleUrl.isEmpty()) {
                m_articleUrls << articleUrl;
            }
        }
    }
    else {
        while ((i < count) && (m_results < max)) {
            const QHtmlElement &item = items.at(i);
            ++i;
            ++m_results;
#ifdef WORDPRESS_DEBUG
            qDebug() << "WordpressFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
            writeStartItem();
            writeItemAuthor(item);
            writeItemBody(item);
            writeItemCategories(item);
            writeItemDate(item);
            writeItemTitle(item);
            writeItemUrl(item);
            writeEndItem();      
        }
    }

    if (m_results < max) {
        const QString next = getNextPageUrl(html);

        if (!next.isEmpty()) {
            getPage(next);
            return;
        }
    }
    
    if ((fetchFullArticles) && (!m_articleUrls.isEmpty())) {
        getArticle(m_articleUrls.takeFirst());
    }
    else {
        writeEndFeed();
#ifdef WORDPRESS_DEBUG
        qDebug() << "WordpressFeedRequest::checkPage(). Writing end of feed";
#endif
        setStatus(Ready);
        emit finished(this);
    }
}

void WordpressFeedRequest::getArticle(const QString &url) {
#ifdef WORDPRESS_DEBUG
    qDebug() << "WordpressFeedRequest::getArticle(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkArticle()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void WordpressFeedRequest::checkArticle() {
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
    reply->deleteLater();
    fixRelativeUrls(page, baseUrl);
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    QHtmlElement item = html.firstElementByTagName("article");
    
    if (item.isNull()) {
        item = html.firstElementByTagName("div", QHtmlAttributeMatch("id", "post-", QHtmlParser::MatchStartsWith));
    }
    
    if (!item.isNull()) {
#ifdef WORDPRESS_DEBUG
        qDebug() << "WordpressFeedRequest::checkArticle(). Writing item";
#endif
        writeStartItem();
        writeItemAuthor(item);
        writeItemBody(item);
        writeItemCategories(item);
        writeItemDate(item);
        writeItemTitle(item);
        writeItemUrl(url.toString());
        writeEndItem();
    }
#ifdef WORDPRESS_DEBUG
    else {
        qDebug() << "WordpressFeedRequest::checkArticle(). No item found";
    }
#endif
    if (!m_articleUrls.isEmpty()) {
        getArticle(m_articleUrls.takeFirst());
    }
    else {
        writeEndFeed();
#ifdef WORDPRESS_DEBUG
        qDebug() << "WordpressFeedRequest::checkArticle(). Writing end of feed";
#endif
        setStatus(Ready);
        emit finished(this);
    }
}

void WordpressFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef WORDPRESS_DEBUG
    qDebug() << "WordpressFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void WordpressFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString WordpressFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString WordpressFeedRequest::getLatestPageUrl(const QHtmlElement &element) {
    const QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatches()
                                                                  << QHtmlAttributeMatch("class", "wp-pagenavi")
                                                                  << QHtmlAttributeMatch("class", "pagination",
                                                                  QHtmlParser::MatchStartsWith),
                                                                  QHtmlParser::MatchAny);
    
    if (!pagination.isNull()) {
        const QHtmlElement first = pagination.firstElementByTagName("a", QHtmlAttributeMatches()
                                                                    << QHtmlAttributeMatch("class", "first")
                                                                    << QHtmlAttributeMatch("class", "page-numbers"),
                                                                    QHtmlParser::MatchAny);
        
        if (!first.isNull()) {
            if (!pagination.firstElementByTagName("a", QHtmlAttributeMatch("class", "prev page-numbers")).isNull()) {
                return first.attribute("href");
            }
        }
    }
    
    return QString();
}

QString WordpressFeedRequest::getNextPageUrl(const QHtmlElement &element) {
    const QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatches()
                                                                  << QHtmlAttributeMatch("class", "wp-pagenavi")
                                                                  << QHtmlAttributeMatch("class", "pagination",
                                                                  QHtmlParser::MatchStartsWith),
                                                                  QHtmlParser::MatchAny);
    
    if (!pagination.isNull()) {
        const QHtmlElement next = pagination.firstElementByTagName("a", QHtmlAttributeMatch("class", "next",
                                                                   QHtmlParser::MatchStartsWith));

        if (!next.isNull()) {
            return next.attribute("href");
        }
    }
    
    return QString();
}

QHtmlElementList WordpressFeedRequest::getItems(const QHtmlElement &element) {
    QHtmlElementList items = element.elementsByTagName("article");
    
    if (items.isEmpty()) {
        items = element.elementsByTagName("div", QHtmlAttributeMatch("id", "post-", QHtmlParser::MatchStartsWith));
    }
    
    return items;
}

QString WordpressFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void WordpressFeedRequest::writeStartFeed() {
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.setAutoFormatting(true);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("WordPress blog posts"));
    m_buffer.close();
}

void WordpressFeedRequest::writeEndFeed() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void WordpressFeedRequest::writeFeedTitle(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(element.firstElementByTagName("title").text()));
    m_writer.writeEndElement();
    m_buffer.close();
}

void WordpressFeedRequest::writeFeedUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

void WordpressFeedRequest::writeStartItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void WordpressFeedRequest::writeEndItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void WordpressFeedRequest::writeItemAuthor(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:creator", element.firstElementByTagName("span", QHtmlAttributeMatch("class", "author",
                              QHtmlParser::MatchStartsWith)).firstElementByTagName("a").text());
    m_buffer.close();
}

void WordpressFeedRequest::writeItemBody(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(element.firstElementByTagName("div", QHtmlAttributeMatch("class", "entry-(content|summary)",
                        QHtmlParser::MatchRegExp)).toString());
    m_writer.writeEndElement();
    m_buffer.close();
}

void WordpressFeedRequest::writeItemCategories(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const QHtmlElement &category, element.elementsByTagName("a", QHtmlAttributeMatch("rel", "^(category |)tag",
                                                                     QHtmlParser::MatchRegExp))) {
        m_writer.writeTextElement("category", category.text());
    }

    m_buffer.close();
}

void WordpressFeedRequest::writeItemDate(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    QHtmlElement date = element.firstElementByTagName("time", QHtmlAttributeMatch("class", "entry-date",
                                                      QHtmlParser::MatchStartsWith));

    if (!date.isNull()) {
        m_writer.writeTextElement("dc:date", date.attribute("datetime"));
        m_buffer.close();
        return;
    }
    
    date = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "updated"));
    
    if (!date.isNull()) {
        m_writer.writeTextElement("dc:date", date.attribute("title"));
        m_buffer.close();
        return;
    }
    
    date = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "entry-date",
                                         QHtmlParser::MatchStartsWith));
    
    if (!date.isNull()) {
        const QHtmlElement time = element.firstElementByTagName("a", QHtmlAttributeMatch("title", "\\d+:\\d+ (am|pm)",
                                                                QHtmlParser::MatchRegExp));
        
        if (!time.isNull()) {
            m_writer.writeTextElement("dc:date", QDateTime::fromString(date.text() + time.attribute("title"),
                                      "MMMM d, yyyyh:mm ap").toString(Qt::ISODate));
        }
        else {
            m_writer.writeTextElement("dc:date", QDateTime::fromString(date.text(), "MMMM d, yyyy")
                                                                      .toString(Qt::ISODate));
        }
        
        m_buffer.close();
        return;
    }
    
    date = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "date"));
    
    if (!date.isNull()) {
        m_writer.writeTextElement("dc:date", QDateTime::fromString(date.text(true).simplified(), "MMM ddyyyy")
                                                                       .toString(Qt::ISODate));
        m_buffer.close();
        return;
    }
    
    m_writer.writeTextElement("dc:date", QDateTime::currentDateTime().toString(Qt::ISODate));
    m_buffer.close();
}

void WordpressFeedRequest::writeItemTitle(const QHtmlElement &element) {
    QHtmlElement title = element.firstElementByTagName("h1", QHtmlAttributeMatch("class", "entry-title"));
    
    if (title.isNull()) {
        title = element.firstElementByTagName("h2", QHtmlAttributeMatch("class", "entry-title"));
    }

    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title.text(true)));
    m_writer.writeEndElement();
    m_buffer.close();
}

void WordpressFeedRequest::writeItemUrl(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", element.firstElementByTagName("a", QHtmlAttributeMatch("rel", "bookmark"))
                                             .attribute("href"));
    m_buffer.close();
}

void WordpressFeedRequest::writeItemUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

QNetworkAccessManager* WordpressFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
