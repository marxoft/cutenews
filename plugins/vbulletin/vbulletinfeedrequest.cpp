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

#include "vbulletinfeedrequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#ifdef VBULLETIN_DEBUG
#include <QDebug>
#endif

const int VbulletinFeedRequest::MAX_REDIRECTS = 8;

const QByteArray VbulletinFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

VbulletinFeedRequest::VbulletinFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString VbulletinFeedRequest::errorString() const {
    return m_errorString;
}

void VbulletinFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
#ifdef VBULLETIN_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "VbulletinFeedRequest::error." << e;
    }
#endif
}

QByteArray VbulletinFeedRequest::result() const {
    return m_buffer.data();
}

void VbulletinFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status VbulletinFeedRequest::status() const {
    return m_status;
}

void VbulletinFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool VbulletinFeedRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool VbulletinFeedRequest::getFeed(const QVariantMap &settings) {
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
            login(QString("%1://%2/login.php?do=login").arg(url.scheme()).arg(url.authority()), username, password);
            return true;
        }
    }
    
    getPage(url);
    return true;
}

void VbulletinFeedRequest::login(const QUrl &url, const QString &username, const QString &password) {
#ifdef VBULLETIN_DEBUG
    qDebug() << "VbulletinFeedRequest::login(). URL:" << url;
#endif
    const QString data = QString("vb_login_username=%1&vb_login_password=%2&s=&securitytoken=guest&do=login&vb_login_md5password=&vb_login_md5password_utf=").arg(username).arg(password);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(checkLogin()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void VbulletinFeedRequest::checkLogin() {
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
#ifdef VBULLETIN_DEBUG
        qDebug() << "VbulletinFeedRequest::checkLogin(). OK";
#endif
        getPage(m_settings.value("url").toString());
        reply->deleteLater();
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        emit finished(this);
        break;
    default:
#ifdef VBULLETIN_DEBUG
        qDebug() << "VbulletinFeedRequest::checkLogin(). Error";
#endif
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        break;
    }
}

void VbulletinFeedRequest::getPage(const QUrl &url) {
#ifdef VBULLETIN_DEBUG
    qDebug() << "VbulletinFeedRequest::getPage(). URL:" << url;
#endif
    m_redirects = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkPage()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void VbulletinFeedRequest::checkPage() {
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
    const QString title = html.firstElementByTagName("title").text().section("- Page", 0, 0).trimmed();
    
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
#ifdef VBULLETIN_DEBUG
        qDebug() << "VbulletinFeedRequest::checkPage(). Writing start of feed";
#endif
        writeStartFeed();
        writeFeedTitle(title);
        writeFeedUrl(baseUrl);
    }
    
    const QHtmlElementList items = getItems(html);

    if (items.isEmpty()) {
#ifdef VBULLETIN_DEBUG
        qDebug() << "VbulletinFeedRequest::checkPage(). No items found. Writing end of feed";
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
#ifdef VBULLETIN_DEBUG
        qDebug() << "VbulletinFeedRequest::checkPage(). Writing item" << m_results << "of" << max;
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
    
    writeEndFeed();
#ifdef VBULLETIN_DEBUG
    qDebug() << "VbulletinFeedRequest::checkPage(). Writing end of feed";
#endif
    setStatus(Ready);
    emit finished(this);
}

void VbulletinFeedRequest::followRedirect(const QUrl &url, const char *slot) {
#ifdef VBULLETIN_DEBUG
    qDebug() << "VbulletinFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

void VbulletinFeedRequest::fixRelativeUrls(QString &page, const QString &baseUrl) {
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

QString VbulletinFeedRequest::getRedirect(const QNetworkReply *reply) {
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

QString VbulletinFeedRequest::getLatestPageUrl(const QHtmlElement &element) {
    QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagenav"));
    
    if (pagination.isNull()) {
        pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("id", "pagination_top"));
    }
    
    if (!pagination.isNull()) {
        const QHtmlElement next = pagination.firstElementByTagName("a", QHtmlAttributeMatch("rel", "next"));

        if (!next.isNull()) {
            const QHtmlElement anchor
                = pagination.lastElementByTagName("a", QHtmlAttributeMatch("title", "^(Last |Show )",
                                                  QHtmlParser::MatchRegExp));

            if (!anchor.isNull()) {
                return anchor.attribute("href");
            }
        }
    }
    
    return QString();
}

QString VbulletinFeedRequest::getNextPageUrl(const QHtmlElement &element) {
    QHtmlElement pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "pagenav"));

    if (pagination.isNull()) {
        pagination = element.firstElementByTagName("div", QHtmlAttributeMatch("id", "pagination_top"));
    }

    if (!pagination.isNull()) {
        const QHtmlElement next = pagination.firstElementByTagName("a", QHtmlAttributeMatch("rel", "prev"));

        if (!next.isNull()) {
            return next.attribute("href");
        }
    }

    return QString();
}

QStringList VbulletinFeedRequest::getCategories(const QHtmlElement &element) {
    QStringList categories;
    const QHtmlElement taglist = element.elementById("tag_list_cell");

    if (!taglist.isNull()) {
        foreach (const QHtmlElement &anchor, taglist.elementsByTagName("a")) {
            categories << anchor.text();
        }
    }

    return categories;
}

QHtmlElementList VbulletinFeedRequest::getItems(const QHtmlElement &element) {
    QHtmlElement posts = element.firstElementByTagName("div", QHtmlAttributeMatch("id", "posts"));

    if (!posts.isNull()) {
        QHtmlElementList items = posts.elementsByTagName("div", QHtmlAttributeMatch("class", "mfcomment"));

        if (items.isEmpty()) {
            items = posts.elementsByTagName("table", QHtmlAttributeMatches() << QHtmlAttributeMatch("class", "tborder")
                                            << QHtmlAttributeMatch("id", "post", QHtmlParser::MatchStartsWith));
        }

        return items;
    }

    posts = element.firstElementByTagName("ol", QHtmlAttributeMatch("id", "posts"));
    return posts.elementsByTagName("li", QHtmlAttributeMatch("id", "post_", QHtmlParser::MatchStartsWith));
}

QString VbulletinFeedRequest::unescape(const QString &text) {
    QString t(text);
    t.replace("&amp;", "&");
    t.replace("&apos;", "'");
    t.replace("&lt;", "<");
    t.replace("&gt;", ">");
    t.replace("&quot;", "\"");
    return t;
}

void VbulletinFeedRequest::writeStartFeed() {
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.setAutoFormatting(true);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("vBulletin thread posts"));
    m_buffer.close();
}

void VbulletinFeedRequest::writeEndFeed() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void VbulletinFeedRequest::writeFeedTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
    m_buffer.close();
}

void VbulletinFeedRequest::writeFeedUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

void VbulletinFeedRequest::writeStartItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void VbulletinFeedRequest::writeEndItem() {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void VbulletinFeedRequest::writeItemAuthor(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    QHtmlElement author = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "username"));

    if (!author.isNull()) {
        m_writer.writeTextElement("dc:creator", author.firstElementByTagName("a").text());
    }
    else {
        author = element.firstElementByTagName("a", QHtmlAttributeMatch("class", "bigusername"));

        if (!author.isNull()) {
            m_writer.writeTextElement("dc:creator", author.text(true));
        }
        else {
            author = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "username_container"));

            if (!author.isNull()) {
                m_writer.writeTextElement("dc:creator", author.firstElementByTagName("strong").text());
            }
        }
    }

    m_buffer.close();
}

void VbulletinFeedRequest::writeItemBody(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    QHtmlElement post = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "postdetails"));
    
    if (!post.isNull()) {
        const QHtmlElement content = post.firstElementByTagName("div", QHtmlAttributeMatch("class", "content"));
        
        if (!content.isNull()) {
            m_writer.writeCDATA(content.toString());
        }
        else {
            m_writer.writeCDATA(post.toString());
        }
    }
    else {
        post = element.firstElementByTagName("div", QHtmlAttributeMatch("id", "post_message_",
                                             QHtmlParser::MatchStartsWith));
        
        if (!post.isNull()) {
            m_writer.writeCDATA(post.toString());
        }
    }
    
    m_writer.writeEndElement();
    m_buffer.close();
}

void VbulletinFeedRequest::writeItemCategories(const QStringList &categories) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const QString &category, categories) {
        m_writer.writeTextElement("category", category);
    }

    m_buffer.close();
}

void VbulletinFeedRequest::writeItemDate(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    QString dateString = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "postdate")).text().trimmed();
    QString format("yyyy-MM-dd");
    
    if (dateString.isEmpty()) {
        dateString = element.firstElementByTagName("span", QHtmlAttributeMatch("class", "date")).text(true).trimmed();
        format = QString("MM-dd-yyyy");
        
        if (dateString.isEmpty()) {
            dateString = element.firstElementByTagName("td").text().trimmed();
        }
    }
        
    if (!dateString.isEmpty()) {
        if (dateString.startsWith("Today")) {
            QDateTime dt = QDateTime::currentDateTime();
            const QRegExp time("(\\d+):(\\d+)( AM| PM|$)");
            
            if (time.indexIn(dateString) != -1) {
                if (time.cap(3) == " PM") {
                    dt.setTime(QTime(time.cap(1).toInt() + 12, time.cap(2).toInt()));
                }
                else {
                    dt.setTime(QTime(time.cap(1).toInt(), time.cap(2).toInt()));
                }
            }
            
            m_writer.writeTextElement("dc:date", dt.toString(Qt::ISODate));
        }
        else if (dateString.startsWith("Yesterday")) {
            QDateTime dt = QDateTime::fromTime_t(QDateTime::currentDateTime().toTime_t() - 86400);
            const QRegExp time("(\\d+):(\\d+)( AM| PM|$)");
            
            if (time.indexIn(dateString) != -1) {
                if (time.cap(3) == " PM") {
                    dt.setTime(QTime(time.cap(1).toInt() + 12, time.cap(2).toInt()));
                }
                else {
                    dt.setTime(QTime(time.cap(1).toInt(), time.cap(2).toInt()));
                }
            }
            
            m_writer.writeTextElement("dc:date", dt.toString(Qt::ISODate));
        }
        else {
            const QRegExp date("\\d+-\\d+-\\d+");
            
            if (date.indexIn(dateString) != -1) {
                QDateTime dt = QDateTime::fromString(date.cap(), format);
                const QRegExp time("(\\d+):(\\d+)( AM| PM|$)");
                
                if (time.indexIn(dateString) != -1) {
                    if (time.cap(3) == " PM") {
                        dt.setTime(QTime(time.cap(1).toInt() + 12, time.cap(2).toInt()));
                    }
                    else {
                        dt.setTime(QTime(time.cap(1).toInt(), time.cap(2).toInt()));
                    }
                }
                
                m_writer.writeTextElement("dc:date", dt.toString(Qt::ISODate));
            }
            else {
                const QRegExp word("[a-z]+");
                const int pos = word.indexIn(dateString);
                m_writer.writeTextElement("dc:date", QDateTime::fromString(dateString.left(pos)
                                          + dateString.mid(pos + word.matchedLength()),
                                          "d MMMM yyyy, HH:mm").toString(Qt::ISODate));
            }
        }
    }

    m_buffer.close();
}

void VbulletinFeedRequest::writeItemTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
    m_buffer.close();
}

void VbulletinFeedRequest::writeItemUrl(const QHtmlElement &element) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    const QHtmlElement report = element.firstElementByTagName("div", QHtmlAttributeMatch("class", "reportthis"));

    if (!report.isNull()) {
        m_writer.writeTextElement("link", report.firstElementByTagName("a").attribute("href"));
    }
    else {
        m_writer.writeTextElement("link", element.firstElementByTagName("a",
                                  QHtmlAttributeMatch("href", "http", QHtmlParser::MatchStartsWith)).attribute("href"));
    }

    m_buffer.close();
}

QNetworkAccessManager* VbulletinFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
