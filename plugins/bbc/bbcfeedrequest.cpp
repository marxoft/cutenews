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
#include "bbcarticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#ifdef BBC_DEBUG
#include <QDebug>
#endif

const int BbcFeedRequest::MAX_REDIRECTS = 8;

const QString BbcFeedRequest::BASE_URL("http://feeds.bbci.co.uk");
const QString BbcFeedRequest::ICON_URL("http://static.bbci.co.uk/news/1.207.11915/apple-touch-icon-72x72-precomposed.png");

const QByteArray BbcFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

BbcFeedRequest::BbcFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_request(0),
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
#ifdef BBC_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "BbcFeedRequest::error." << e;
    }
#endif
}

QByteArray BbcFeedRequest::result() const {
    return m_buffer.data();
}

void BbcFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status BbcFeedRequest::status() const {
    return m_status;
}

void BbcFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool BbcFeedRequest::cancel() {
    if (status() == Active) {
        if ((m_request) && (m_request->status() == ArticleRequest::Active)) {
            m_request->cancel();
        }
        else {
            setStatus(Canceled);
            emit finished(this);
        }
    }

    return true;
}

bool BbcFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setResult(QByteArray());
    setErrorString(QString());
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

    const QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            reply->deleteLater();
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

    if (m_parser.readChannel()) {
        writeStartFeed();
        writeFeedTitle(m_parser.title());
        writeFeedUrl(m_parser.url());

        if (m_parser.readNextArticle()) {
            if (m_parser.date() > m_settings.value("lastUpdated").toDateTime()) {
                reply->deleteLater();
                getArticle(m_parser.url());
            }
            else {
#ifdef BBC_DEBUG
                qDebug() << "BbcFeedRequest::checkFeed(). No new articles";
#endif
                writeEndFeed();
                setStatus(Ready);
                emit finished(this);
            }

            return;
        }

        writeEndFeed();
    }
    
    setErrorString(m_parser.errorString());
    setStatus(Error);
    emit finished(this);
}

void BbcFeedRequest::getArticle(const QString &url) {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::getArticle(). URL:" << url;
#endif
    articleRequest()->getArticle(url, m_settings);
}

void BbcFeedRequest::checkArticle(ArticleRequest *request) {
    if (request->status() == ArticleRequest::Canceled) {
        setStatus(Canceled);
        emit finished(this);
        return;
    }

    ++m_results;

    if (request->status() == ArticleRequest::Ready) {
        const ArticleResult article = request->result();
        writeStartItem();
        writeItemAuthor(article.author.isEmpty() ? m_parser.author() : article.author);
        writeItemBody(article.body.isEmpty() ? m_parser.description() : article.body);
        writeItemCategories(article.categories.isEmpty() ? m_parser.categories() : article.categories);
        writeItemDate(article.date.isNull() ? m_parser.date() : article.date);
        writeItemEnclosures(article.enclosures.isEmpty() ? m_parser.enclosures() : article.enclosures);
        writeItemTitle(article.title.isEmpty() ? m_parser.title() : article.title);
        writeItemUrl(article.url.isEmpty() ? m_parser.url() : article.url);
        writeEndItem();
    }
#ifdef BBC_DEBUG
    else {
        qDebug() << "BbcFeedRequest::checkArticle(). Error:" << request->errorString();
    }
#endif
    if (m_results < m_settings.value("maxResults", 20).toInt()) {
        if (!m_parser.readNextArticle()) {
            writeEndFeed();
            setErrorString(m_parser.errorString());
            setStatus(Error);
            emit finished(this);
            return;
        }

        if (m_parser.date() > m_settings.value("lastUpdated").toDateTime()) {
            getArticle(m_parser.url());
            return;
        }
#ifdef BBC_DEBUG
        qDebug() << "BbcFeedRequest::checkArticle(). No more new articles";
#endif
    }

    writeEndFeed();
    setStatus(Ready);
    emit finished(this);
}

void BbcFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
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
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::writeStartFeed()";
#endif
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("News articles from The BBC"));
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
    m_buffer.close();
}

void BbcFeedRequest::writeEndFeed() {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::writeEndFeed()";
#endif
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void BbcFeedRequest::writeFeedTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
    m_buffer.close();
}

void BbcFeedRequest::writeFeedUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

void BbcFeedRequest::writeStartItem() {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::writeStartItem(). Item" << m_results << "of"
        << m_settings.value("maxResults", 20).toInt();
#endif
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void BbcFeedRequest::writeEndItem() {
#ifdef BBC_DEBUG
    qDebug() << "BbcFeedRequest::writeEndItem(). Item" << m_results << "of"
        << m_settings.value("maxResults", 20).toInt();
#endif
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void BbcFeedRequest::writeItemAuthor(const QString &author) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:creator", author);
    m_buffer.close();
}

void BbcFeedRequest::writeItemBody(const QString &body) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body);
    m_writer.writeEndElement();
    m_buffer.close();
}

void BbcFeedRequest::writeItemCategories(const QStringList &categories) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const QString &category, categories) {
        m_writer.writeTextElement("category", category);
    }

    m_buffer.close();
}

void BbcFeedRequest::writeItemDate(const QDateTime &date) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
    m_buffer.close();
}

void BbcFeedRequest::writeItemEnclosures(const QVariantList &enclosures) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const QVariant &e, enclosures) {
        const QVariantMap enclosure = e.toMap();
        m_writer.writeStartElement("enclosure");
        m_writer.writeAttribute("url", enclosure.value("url").toString());
        m_writer.writeAttribute("type", enclosure.value("type").toString());
        m_writer.writeEndElement();
    }

    m_buffer.close();
}

void BbcFeedRequest::writeItemTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(unescape(title));
    m_writer.writeEndElement();
    m_buffer.close();
}

void BbcFeedRequest::writeItemUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

BbcArticleRequest* BbcFeedRequest::articleRequest() {
    if (!m_request) {
        m_request = new BbcArticleRequest(this);
        connect(m_request, SIGNAL(finished(ArticleRequest*)), this, SLOT(checkArticle(ArticleRequest*)));
    }

    return m_request;
}

QNetworkAccessManager* BbcFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
