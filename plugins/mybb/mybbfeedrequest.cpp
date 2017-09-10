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

#include "mybbfeedrequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef MYBB_DEBUG
#include <QDebug>
#endif

const int MybbFeedRequest::MAX_REDIRECTS = 8;

const QByteArray MybbFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

MybbFeedRequest::MybbFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString MybbFeedRequest::errorString() const {
    return m_errorString;
}

void MybbFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
#ifdef MYBB_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "MybbFeedRequest::error." << e;
    }
#endif
}

QByteArray MybbFeedRequest::result() const {
    return m_buffer.data();
}

void MybbFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status MybbFeedRequest::status() const {
    return m_status;
}

void MybbFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool MybbFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool MybbFeedRequest::getFeed(const QVariantMap &settings) {
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
            login(QString("%1://%2/member.php").arg(url.scheme()).arg(url.authority()), username, password);
            return true;
        }
    }
    
    getPage(url);
    return true;
}

void MybbFeedRequest::login(const QUrl &url, const QString &username, const QString &password) {
#ifdef MYBB_DEBUG
    qDebug() << "MybbFeedRequest::login(). URL:" << url;
#endif
    const QString data = QString("quick_username=%1&quick_password=%2&s=&action=do_login&quick_login=1&submit=Login").arg(username).arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void MybbFeedRequest::checkLogin() {
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
#ifdef MYBB_DEBUG
        qDebug() << "MybbFeedRequest::checkLogin(). OK";
#endif
        getPage(m_settings.value("url").toString());
        reply->deleteLater();
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        emit finished(this);
        break;
    default:
#ifdef MYBB_DEBUG
        qDebug() << "MybbFeedRequest::checkLogin(). Error";
#endif
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        break;
    }
}

void MybbFeedRequest::getPage(const QUrl &url) {
#ifdef MYBB_DEBUG
    qDebug() << "MybbFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void MybbFeedRequest::checkPage() {
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
#ifdef MYBB_DEBUG
        qDebug() << "MybbFeedRequest::checkPage(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(html);
        writeFeedUrl(baseUrl);
    }
    
    const QHtmlElementList items = getItems(html);

    if (items.isEmpty()) {
#ifdef MYBB_DEBUG
        qDebug() << "MybbFeedRequest::checkPage(). No items found. Writing end of feed";
#endif
        writeEndFeed();
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
#ifdef MYBB_DEBUG
        qDebug() << "MybbFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
#endif
        writeStartItem();
        writeItemAuthor(item);
        writeItemBody(item);
        writeItemDate(item);
        writeItemTitle(item);
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
#ifdef MYBB_DEBUG
    qDebug() << "MybbFeedRequest::checkPage(). Writing end of feed";
#endif
    writeEndFeed();
    setStatus(Ready);
    emit finished(this);
}

void MybbFeedRequest::followRedirect(const QUrl &url, const char *slot) {
#ifdef MYBB_DEBUG
    qDebug() << "MybbFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void MybbFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString MybbFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString MybbFeedRequest::getLatestPageUrl(const QHtmlElement &element) {
    const QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagination"));
    
    if (!pagination.isNull()) {
        QHtmlElement last = pagination.firstElementByTagName("a", QHtmlAttributeMatch("class", "pagination_last"));
        
        if (!last.isNull()) {
            return last.attribute("href");
        }

        const int count = pagination.firstElementByTagName("span", QHtmlAttributeMatch("class", "pages")).text()
                                    .section("(", 1, 1).section(")", 0, 0).toInt();
        const int current = pagination.firstElementByTagName("span", QHtmlAttributeMatch("class", "pagination_current"))
                                      .text().toInt();
        
        if ((count > current) && (count > 0) && (current > 0)) {
            last = pagination.lastElementByTagName("a", QHtmlAttributeMatch("class", "pagination_page"));

            if (!last.isNull()) {
                return last.attribute("href");
            }
        }
    }
    
    return QString();
}

QString MybbFeedRequest::getNextPageUrl(const QHtmlElement &element) {
    const QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagination"));
    
    if (!pagination.isNull()) {
        const QHtmlElement next = pagination.firstElementByTagName("a", QHtmlAttributeMatch("class", "pagination_previous"));

        if (!next.isNull()) {
            return next.attribute("href");
        }
    }
    
    return QString();
}

QHtmlElementList MybbFeedRequest::getItems(const QHtmlElement &element) {
    return element.firstElementByTagName("div", QHtmlAttributeMatch("id", "posts"))
                  .elementsByTagName("table", QHtmlAttributeMatch("id", "post_", QHtmlParser::MatchStartsWith));
}

QString MybbFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void MybbFeedRequest::writeStartFeed() {
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.setAutoFormatting(true);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("MyBB thread posts"));
    m_buffer.close();
}

void MybbFeedRequest::writeEndFeed() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void MybbFeedRequest::writeFeedTitle(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(element.firstElementByTagName("title").text()));
    m_writer.writeEndElement();
    m_buffer.close();
}

void MybbFeedRequest::writeFeedUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

void MybbFeedRequest::writeStartItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void MybbFeedRequest::writeEndItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void MybbFeedRequest::writeItemAuthor(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:creator",
                              element.firstElementByTagName("span", QHtmlAttributeMatch("class", "largetext"))
                              .firstElementByTagName("a").text(true));
    m_buffer.close();
}

void MybbFeedRequest::writeItemBody(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(element.nthElementByTagName(1, "div").toString());
    m_writer.writeEndElement();
    m_buffer.close();
}

void MybbFeedRequest::writeItemDate(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    QString dateString = element.nthElementByTagName(-1, "span", QHtmlAttributeMatch("class", "smalltext")).text().trimmed();
    
    if (dateString.isEmpty()) {
        m_writer.writeTextElement("dc:date", QDateTime::currentDateTime().toString(Qt::ISODate));
        m_buffer.close();
        return;
    }

    if (dateString.contains("last modified:")) {
        dateString = dateString.section("last modified:", 1, 1).section("by", 0, 0).trimmed();
    }
    
    if (dateString.contains("Today")) {
        QDateTime date = QDateTime::currentDateTime();
        const QRegExp time("(\\d+):(\\d+)");
        
        if (time.indexIn(dateString) != -1) {
            date.setTime(QTime(time.cap(1).toInt(), time.cap(2).toInt()));
        }

        m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
    }
    else if (dateString.contains("Yesterday")) {
        QDateTime date = QDateTime::fromTime_t(QDateTime::currentDateTime().toTime_t() - 86400);
        const QRegExp time("(\\d+):(\\d+)");
        
        if (time.indexIn(dateString) != -1) {
            date.setTime(QTime(time.cap(1).toInt(), time.cap(2).toInt()));
        }

        m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
    }
    else {
        m_writer.writeTextElement("dc:date", QDateTime::fromString(dateString, "dd-MM-yyyy HH:mm")
                                                                  .toString(Qt::ISODate));
    }

    m_buffer.close();
}

void MybbFeedRequest::writeItemTitle(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(element.nthElementByTagName(2, "span", QHtmlAttributeMatch("class", "smalltext"))
                                 .firstChildElement().text()));
    m_writer.writeEndElement();
    m_buffer.close();
}

void MybbFeedRequest::writeItemUrl(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", element.nthElementByTagName(1, "span", QHtmlAttributeMatch("class", "smalltext"))
                              .firstElementByTagName("a").attribute("href"));
    m_buffer.close();
}

QNetworkAccessManager* MybbFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
