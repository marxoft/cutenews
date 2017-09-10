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

#include "xenforofeedrequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef XENFORO_DEBUG
#include <QDebug>
#endif

const int XenforoFeedRequest::MAX_REDIRECTS = 8;

const QByteArray XenforoFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

XenforoFeedRequest::XenforoFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString XenforoFeedRequest::errorString() const {
    return m_errorString;
}

void XenforoFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
#ifdef XENFORO_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "XenforoFeedRequest::error." << e;
    }
#endif
}

QByteArray XenforoFeedRequest::result() const {
    return m_buffer.data();
}

void XenforoFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status XenforoFeedRequest::status() const {
    return m_status;
}

void XenforoFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool XenforoFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool XenforoFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(QByteArray());
    m_settings = settings;
    m_results = 0;
    
    const QUrl url = settings.value("url").toString();
    
    if (settings.value("performLogin").toBool()) {
        const QString username = settings.value("username").toString();
        const QString password = settings.value("password").toString();
        
        if ((!username.isEmpty()) && (!password.isEmpty())) {
            login(QString("%1://%2/login/login").arg(url.scheme()).arg(url.authority()), username, password);
            return true;
        }
    }
    
    getPage(url);
    return true;
}

void XenforoFeedRequest::login(const QUrl &url, const QString &username, const QString &password) {
#ifdef XENFORO_DEBUG
    qDebug() << "XenforoFeedRequest::login(). URL:" << url;
#endif
    const QString data = QString("login=%1&password=%2&register=0&_xfToken=").arg(username).arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void XenforoFeedRequest::checkLogin() {
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
#ifdef XENFORO_DEBUG
        qDebug() << "XenforoFeedRequest::checkLogin(). OK";
#endif
        getPage(m_settings.value("url").toString());
        reply->deleteLater();
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        emit finished(this);
        break;
    default:
#ifdef XENFORO_DEBUG
        qDebug() << "XenforoFeedRequest::checkLogin(). Error";
#endif
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        break;
    }
}

void XenforoFeedRequest::getPage(const QUrl &url) {
#ifdef XENFORO_DEBUG
    qDebug() << "XenforoFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void XenforoFeedRequest::checkPage() {
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
    const QString title = html.firstElementByTagName("h1").text();
    
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
#ifdef XENFORO_DEBUG
        qDebug() << "XenforoFeedRequest::checkPage(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(title);
        writeFeedUrl(baseUrl);
    }
    
    const QHtmlElementList items = getItems(html);

    if (items.isEmpty()) {
#ifdef XENFORO_DEBUG
        qDebug() << "XenforoFeedRequest::checkPage(). No items found. Writing end of feed";
#endif
        writeEndFeed();
        setStatus(Ready);
        emit finished(this);
        return;
    }

    const QStringList categories = getCategories(html);
    const int max = m_settings.value("maxResults", 20).toInt();
    int i = items.size() - 1;
    
    while ((i >= 0) && (m_results < max)) {
        const QHtmlElement &item = items.at(i);
        --i;
        ++m_results;
#ifdef XENFORO_DEBUG
        qDebug() << "XenforoFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
        writeStartItem();
        writeItemAuthor(item);
        writeItemBody(item);
        writeItemCategories(categories);
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
#ifdef XENFORO_DEBUG
    qDebug() << "XenforoFeedRequest::checkPage(). Writing end of feed";
#endif
    writeEndFeed();
    setStatus(Ready);
    emit finished(this);
}

void XenforoFeedRequest::followRedirect(const QUrl &url, const char *slot) {
#ifdef XENFORO_DEBUG
    qDebug() << "XenforoFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void XenforoFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString XenforoFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString XenforoFeedRequest::getLatestPageUrl(const QHtmlElement &element) {
    const QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "PageNav"));
    const int pageCount = pagination.attribute("data-last").toInt();
    const int currentPage = pagination.attribute("data-page").toInt();
    
    if ((currentPage < pageCount) && (currentPage > 0) && (pageCount > 0)) {
        foreach (const QHtmlElement &anchor, pagination.elementsByTagName("a")) {
            if (anchor.text().toInt() == pageCount) {
                return anchor.attribute("href");
            }
        }
    }

    return QString();
}

QString XenforoFeedRequest::getNextPageUrl(const QHtmlElement &element) {
    const QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "PageNav"));
    const int currentPage = pagination.attribute("data-page").toInt();

    if (currentPage > 1) {
        return pagination.firstElementByTagName("a", QHtmlAttributeMatch("class", "text")).attribute("href");
    }
    
    return QString();
}

QStringList XenforoFeedRequest::getCategories(const QHtmlElement &element) {
    QStringList categories;
    const QHtmlElement taglist = element.firstElementByTagName("ul", QHtmlAttributeMatch("class", "tagList"));

    if (!taglist.isNull()) {
        foreach (const QHtmlElement &anchor, taglist.elementsByTagName("a", QHtmlAttributeMatch("class", "tag"))) {
            categories << anchor.text();
        }
    }

    return categories;
}

QHtmlElementList XenforoFeedRequest::getItems(const QHtmlElement &element) {
    return element.firstElementByTagName("ol", QHtmlAttributeMatch("class", "messageList")).elementsByTagName("li");
}

QString XenforoFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void XenforoFeedRequest::writeStartFeed() {
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("XenForo thread posts"));
    m_buffer.close();
}

void XenforoFeedRequest::writeEndFeed() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void XenforoFeedRequest::writeFeedTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
    m_buffer.close();
}

void XenforoFeedRequest::writeFeedUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

void XenforoFeedRequest::writeStartItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void XenforoFeedRequest::writeEndItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void XenforoFeedRequest::writeItemAuthor(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:creator", element.attribute("data-author"));
    m_buffer.close();
}

void XenforoFeedRequest::writeItemBody(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(element.firstElementByTagName("div", QHtmlAttributeMatch("class", "messageContent")).toString());
    m_writer.writeEndElement();
    m_buffer.close();
}

void XenforoFeedRequest::writeItemCategories(const QStringList &categories) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const QString &category, categories) {
        m_writer.writeTextElement("category", category);
    }

    m_buffer.close();
}

void XenforoFeedRequest::writeItemDate(const QHtmlElement &element) {
    QString dateString = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "DateTime")).attribute("title");

    if (dateString.isEmpty()) {
        dateString = element.firstElementByTagName("abbr", QHtmlAttributeMatch("class", "DateTime")).text();        
    }

    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:date", QDateTime::fromString(dateString, "MMM d, yyyy 'at' h:mm AP")
                                                              .toString(Qt::ISODate));
    m_buffer.close();
}

void XenforoFeedRequest::writeItemTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
    m_buffer.close();
}

void XenforoFeedRequest::writeItemUrl(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", element.firstElementByTagName("a", QHtmlAttributeMatch("class", "datePermalink"))
                                             .attribute("href"));
    m_buffer.close();
}

QNetworkAccessManager* XenforoFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
