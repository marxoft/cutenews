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

#include "phpbbfeedrequest.h"
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef PHPBB_DEBUG
#include <QDebug>
#endif

const int PhpbbFeedRequest::MAX_REDIRECTS = 8;

const QByteArray PhpbbFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

PhpbbFeedRequest::PhpbbFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString PhpbbFeedRequest::errorString() const {
    return m_errorString;
}

void PhpbbFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray PhpbbFeedRequest::result() const {
    return m_buffer.data();
}

PhpbbFeedRequest::Status PhpbbFeedRequest::status() const {
    return m_status;
}

void PhpbbFeedRequest::setStatus(PhpbbFeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool PhpbbFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool PhpbbFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    m_settings = settings;
    m_results = 0;
    
    const QUrl url = settings.value("url").toString();
    
    if (settings.value("performLogin").toBool()) {
        const QString username = settings.value("username").toString();
        const QString password = settings.value("password").toString();
        
        if ((!username.isEmpty()) && (!password.isEmpty())) {
            login(QString("%1://%2/login.php").arg(url.scheme()).arg(url.authority()), username, password);
            return true;
        }
    }
    
    getPage(url);
    return true;
}

void PhpbbFeedRequest::login(const QUrl &url, const QString &username, const QString &password) {
#ifdef PHPBB_DEBUG
    qDebug() << "PhpbbFeedRequest::login(). URL:" << url << "Username:" << username << "Password:" << password;
#endif
    const QString data = QString("username=%1&password=%2&login=Log+in").arg(username).arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void PhpbbFeedRequest::checkLogin() {
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
            followRedirect(redirect, SLOT(checkLogin()));
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
#ifdef PHPBB_DEBUG
        qDebug() << "PhpbbFeedRequest::checkLogin(). OK";
#endif
        getPage(m_settings.value("url").toString());
        reply->deleteLater();
        break;
    case QNetworkReply::OperationCanceledError:
        setErrorString(QString());
        setStatus(Canceled);
        emit finished(this);
        break;
    default:
#ifdef PHPBB_DEBUG
        qDebug() << "PhpbbFeedRequest::checkLogin(). Error";
#endif
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        break;
    }
}

void PhpbbFeedRequest::getPage(const QUrl &url) {
#ifdef PHPBB_DEBUG
    qDebug() << "PhpbbFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void PhpbbFeedRequest::checkPage() {
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
    const QHtmlDocument document(page);
    const QHtmlElement html = document.htmlElement();
    QString title = html.firstElementByTagName("title").text();
    
    if (title.startsWith("title - ")) {
        title.remove(0, 8);
    }

    const int pi = title.lastIndexOf(" - Page ");
    
    if (pi > 0) {
        title.remove(pi, title.size() - pi);
    }
    
    if (m_results == 0) {
        QString redirect = getLatestPageUrl(html);

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
#ifdef PHPBB_DEBUG
        qDebug() << "PhpbbFeedRequest::checkPage(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(title);
        writeFeedUrl(baseUrl);
    }
    
    const QHtmlElementList items = getItems(html);

    if (items.isEmpty()) {
#ifdef PHPBB_DEBUG
        qDebug() << "PhpbbFeedRequest::checkPage(). No items found. Writing end of feed";
#endif
        writeEndFeed();
        setErrorString(QString());
        setStatus(Ready);
        emit finished(this);
        return;
    }
    
    const int max = m_settings.value("maxResults", 20).toInt();
    int i = items.size() - 1;
    
    while ((i >= 0) && (m_results < max)) {
        const QHtmlElement &item = items.at(i);
        --i;
        ++m_results;
#ifdef PHPBB_DEBUG
        qDebug() << "PhpbbFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
        writeStartItem();
        writeItemAuthor(item);
        writeItemBody(item);
        writeItemDate(item);
        writeItemTitle(title);
        writeItemUrl(item);
        writeEndItem();      
    }

    if (m_results < max) {
        const QString next = getNextPageUrl(html);

        if (!next.isEmpty()) {
            getPage(next);
            return;
        }
    }
    
    writeEndFeed();
#ifdef PHPBB_DEBUG
    qDebug() << "PhpbbFeedRequest::checkPage(). Writing end of feed. Result:";
    qDebug() << result();
#endif
    setErrorString(QString());
    setStatus(Ready);
    emit finished(this);
}

void PhpbbFeedRequest::followRedirect(const QUrl &url, const char *slot) {
#ifdef PHPBB_DEBUG
    qDebug() << "PhpbbFeedRequest::followRedirect(). URL:" << url;
#endif
    m_redirects++;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void PhpbbFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString PhpbbFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString PhpbbFeedRequest::getLatestPageUrl(const QHtmlElement &element) {
    QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagination"));

    if (!pagination.isNull()) {
        const QHtmlElementList anchors = pagination.elementsByTagName("a", QHtmlAttributeMatch("role", "button"));

        if (anchors.size() > 1) {
            if (anchors.last().attribute("rel") == "next") {
                return anchors.at(anchors.size() - 2).attribute("href");
            }
        }

        return QString();
    }

    pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("id", "pagecontent"));

    if (!pagination.isNull()) {
        const QHtmlElementList anchors = pagination.firstElementByTagName("table").elementsByTagName("a");

        if (anchors.size() > 1) {
            if (anchors.last().text() == "Next") {
                return anchors.at(anchors.size() - 2).attribute("href");
            }
        }

        return QString();
    }
    
    pagination = element.nthElementByTagName(2, "span", QHtmlAttributeMatch("class", "nav"));
    
    if (!pagination.isNull()) {
        const QHtmlElementList anchors = pagination.elementsByTagName("a");
        
        if (anchors.size() > 1) {
            if (anchors.last().text() == "Next") {
                return anchors.at(anchors.size() - 2).attribute("href");
            }
        }
        
        return QString();
    }
    
    pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagenav",
                                                                          QHtmlParser::MatchStartsWith));
    
    if (!pagination.isNull()) {
        const QHtmlElement current = pagination.firstElementByTagName("a", QHtmlAttributeMatch("class",
                                                                      "btn btn-default active"));
        
        if (!current.isNull()) {
            const QHtmlElement input = pagination.firstElementByTagName("input", QHtmlAttributeMatch("name",
                                                                        "page-number"));
            
            if (!input.isNull()) {
                const QString max = input.attribute("max");
                
                if ((!max.isEmpty()) && (current.text() == max)) {
                    return QString();
                }
            }
        }
        
        const QHtmlElement anchor = pagination.lastElementByTagName("a", QHtmlAttributeMatch("class",
                                                                    "btn btn-default"));
        
        if (!anchor.isNull()) {            
            return anchor.attribute("href");
        }
    }
    
    return QString();
}

QString PhpbbFeedRequest::getNextPageUrl(const QHtmlElement &element) {
    QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagination"));

    if (!pagination.isNull()) {
        const QHtmlElement anchor = pagination.firstElementByTagName("a", QHtmlAttributeMatch("rel", "prev"));

        if (!anchor.isNull()) {
            return anchor.attribute("href");
        }

        return QString();
    }

    pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("id", "pagecontent"));

    if (!pagination.isNull()) {
        const QHtmlElementList anchors = pagination.firstElementByTagName("table").elementsByTagName("a");

        foreach (const QHtmlElement &anchor, anchors) {
            if (anchor.text() == "Previous") {
                return anchor.attribute("href");
            }
        }
        
        return QString();
    }
    
    pagination = element.nthElementByTagName(2, "span", QHtmlAttributeMatch("class", "nav"));
    
    if (!pagination.isNull()) {
        const QHtmlElementList anchors = pagination.elementsByTagName("a");
        
        foreach (const QHtmlElement &anchor, anchors) {
            if (anchor.text() == "Previous") {
                return anchor.attribute("href");
            }
        }
        
        return QString();
    }
    
    pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagenav",
                                                                          QHtmlParser::MatchStartsWith));
    
    if (!pagination.isNull()) {
        const QHtmlElement anchor = pagination.firstElementByTagName("a", QHtmlAttributeMatch("class",
                                                                     "btn btn-default last_item"));
        
        if (!anchor.isNull()) {
            return anchor.attribute("href");
        }
    }
    
    return QString();
}

QHtmlElementList PhpbbFeedRequest::getItems(const QHtmlElement &element) {
    QHtmlElementList elements = element.elementsByTagName("table", QHtmlAttributeMatch("class", "forumline"));
    
    if (elements.size() > 1) {
        QHtmlElementList rows = elements.at(elements.size() - 2).elementsByTagName("tr");

        for (int i = rows.size() - 1; i >= 0; i--) {
            if (rows.at(i).firstElementByTagName("span").attribute("class") != "name") {
                rows.removeAt(i);
            }
        }

        return rows;
    }

    elements = element.elementsByTagName("table", QHtmlAttributeMatch("class", "tablebg"));

    if (elements.size() > 5) {
        return elements.mid(2, elements.size() - 5);
    }    
    
    elements = element.elementsByTagName("div", QHtmlAttributeMatch("class", "post has-profile bg2"));
    
    if (!elements.isEmpty()) {
        return elements;
    }
    
    const QHtmlElement topic = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "view-topic",
                                                                                        QHtmlParser::MatchEndsWith));
    
    if (!topic.isNull()) {
        elements = topic.elementsByTagName("div", QHtmlAttributeMatch("class", "row-wrap"));
        
        if (!elements.isEmpty()) {
            for (int i = elements.size() - 1; i >= 0; i--) {
                const QHtmlElement body = elements.at(i).firstElementByTagName("div", QHtmlAttributeMatch("class",
                                                                               "postbody", QHtmlParser::MatchEndsWith));
                
                if (body.isNull()) {
                    elements.removeAt(i);
                }
            }            
        }
    }
    
    return elements;
}

void PhpbbFeedRequest::writeStartFeed() {
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
    m_writer.writeTextElement("description", tr("phpBB thread posts"));
}

void PhpbbFeedRequest::writeEndFeed() {
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void PhpbbFeedRequest::writeFeedTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(title);
    m_writer.writeEndElement();
}

void PhpbbFeedRequest::writeFeedUrl(const QString &url) {
    m_writer.writeTextElement("link", url);
}

void PhpbbFeedRequest::writeStartItem() {
    m_writer.writeStartElement("item");
}

void PhpbbFeedRequest::writeEndItem() {
    m_writer.writeEndElement();
}

void PhpbbFeedRequest::writeItemAuthor(const QHtmlElement &element) {
    QHtmlElement author = element.firstElementByTagName("a", QHtmlAttributeMatch("class", "username"));

    if (author.isNull()) {
        author = element.firstElementByTagName("b");
        
        if (author.isNull()) {
            author = element.firstElementByTagName("span");
        }
    }
    
    m_writer.writeTextElement("dc:creator", author.text(true));
}

void PhpbbFeedRequest::writeItemBody(const QHtmlElement &element) {
    m_writer.writeStartElement("content:encoded");
    QHtmlElement body = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "postbody"));
    
    if (body.isNull()) {
        body = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "postbody"));
        
        if (body.isNull()) {
            body = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "content"));
            
            if (body.isNull()) {
                body = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "post-text",
                                                                                QHtmlParser::MatchEndsWith));
            }
        }
    }
    
    m_writer.writeCDATA(body.toString());
    m_writer.writeEndElement();
}

void PhpbbFeedRequest::writeItemDate(const QHtmlElement &element) {
    QString dateString = element.nthElementByTagName(1, "span", QHtmlAttributeMatch("class", "postdetails")).text();
    
    if (!dateString.isEmpty()) {
        dateString = dateString.left(dateString.indexOf("&")).trimmed();
        m_writer.writeTextElement("dc:date", QDateTime::fromString(dateString, "'Posted: 'ddd MMM dd, yyyy h:mm ap")
                                                                  .toString(Qt::ISODate));
        return;
    }

    dateString = element.firstElementByTagName("p", QHtmlAttributeMatch("class", "author")).text();

    if (!dateString.isEmpty()) {
        m_writer.writeTextElement("dc:date", QDateTime::fromString(dateString.trimmed(), "ddd MMM dd, yyyy h:mm ap")
                                                                  .toString(Qt::ISODate));
        return;
    }

    dateString = element.nthElementByTagName(1, "div").text();
    
    if (!dateString.isEmpty()) {
        dateString = dateString.left(dateString.indexOf("&")).trimmed();
        m_writer.writeTextElement("dc:date", QDateTime::fromString(dateString, "ddd MMM dd, yyyy h:mm ap")
                                                                  .toString(Qt::ISODate));
    }    
}

void PhpbbFeedRequest::writeItemTitle(const QString &title) {
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(title);
    m_writer.writeEndElement();
}

void PhpbbFeedRequest::writeItemUrl(const QHtmlElement &element) {
    QHtmlElement el = element.firstElementByTagName("p", QHtmlAttributeMatch("class", "author"));

    if (el.isNull()) {
        el = element.firstElementByTagName("table");
        
        if (el.isNull()) {
            el = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "postbody",
                                                                          QHtmlParser::MatchEndsWith));
        }
    }
    
    m_writer.writeTextElement("link", el.firstElementByTagName("a", QHtmlAttributeMatch("href", "http",
                                                               QHtmlParser::MatchStartsWith)).attribute("href"));
}

QNetworkAccessManager* PhpbbFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
