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

#include "telegraphfeedrequest.h"
#include "telegrapharticlerequest.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#ifdef TELEGRAPH_DEBUG
#include <QDebug>
#endif

const int TelegraphFeedRequest::MAX_REDIRECTS = 8;

const QString TelegraphFeedRequest::BASE_URL("http://www.telegraph.co.uk");
const QString TelegraphFeedRequest::ICON_URL("http://www.telegraph.co.uk/etc/designs/telegraph/core/clientlibs/themes/cars/img/favicon/apple-touch-icon-72x72.png");

const QByteArray TelegraphFeedRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

TelegraphFeedRequest::TelegraphFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_request(0),
    m_nam(0),
    m_status(Idle),
    m_results(0),
    m_redirects(0)
{
}

QString TelegraphFeedRequest::errorString() const {
    return m_errorString;
}

void TelegraphFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
#ifdef TELEGRAPH_DEBUG
    if (!e.isEmpty()) {
        qDebug() << "TelegraphFeedRequest::error." << e;
    }
#endif
}

QByteArray TelegraphFeedRequest::result() const {
    return m_buffer.data();
}

void TelegraphFeedRequest::setResult(const QByteArray &r) {
    m_buffer.open(QBuffer::WriteOnly);
    m_buffer.write(r);
    m_buffer.close();
}

FeedRequest::Status TelegraphFeedRequest::status() const {
    return m_status;
}

void TelegraphFeedRequest::setStatus(FeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool TelegraphFeedRequest::cancel() {
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

bool TelegraphFeedRequest::getFeed(const QVariantMap &settings) {
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
    const QString section = m_settings.value("section", "news").toString();

    if (!section.isEmpty()) {
        url.append("/");
        url.append(section);
    }

    url.append("/rss.xml");
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::getFeed(). URL:" << url;
#endif
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkFeed()));
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void TelegraphFeedRequest::checkFeed() {
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
        writeFeedTitle(tr("The Telegraph - %1").arg(m_parser.title()));
        writeFeedUrl(m_parser.url());
        const bool fetchFullArticle = m_settings.value("fetchFullArticle", true).toBool();
        const QDateTime lastUpdated = m_settings.value("lastUpdated").toDateTime();

        if (fetchFullArticle) {
            if (m_parser.readNextArticle()) {
                if (m_parser.date() > lastUpdated) {
                    reply->deleteLater();
                    getArticle(m_parser.url());
                }
                else {
                    writeEndFeed();
                    setStatus(Ready);
                    emit finished(this);
                }

                return;
            }

            writeEndFeed();
        }
        else {
            const int max = m_settings.value("maxResults", 20).toInt();

            while((m_results < max) && (m_parser.readNextArticle()) && (m_parser.date() > lastUpdated)) {
                ++m_results;
                writeStartItem();
                writeItemAuthor(m_parser.author());
                writeItemBody(m_parser.description());
                writeItemCategories(m_parser.categories());
                writeItemDate(m_parser.date());
                writeItemEnclosures(m_parser.enclosures());
                writeItemTitle(m_parser.title());
                writeItemUrl(m_parser.url());
                writeEndItem();
            }

            writeEndFeed();
            setStatus(Ready);
            emit finished(this);
            return;
        }
    }
    
    setErrorString(m_parser.errorString());
    setStatus(Error);
    emit finished(this);
}

void TelegraphFeedRequest::getArticle(const QString &url) {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::getArticle(). URL:" << url;
#endif
    articleRequest()->getArticle(url, m_settings);
}

void TelegraphFeedRequest::checkArticle(ArticleRequest *request) {
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
#ifdef TELEGRAPH_DEBUG
    else {
        qDebug() << "TelegraphFeedRequest::checkArticle(). Error:" << request->errorString();
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
#ifdef TELEGRAPH_DEBUG
        qDebug() << "TelegraphFeedRequest::checkArticle(). No more new articles";
#endif
    }

    writeEndFeed();
    setStatus(Ready);
    emit finished(this);
}

void TelegraphFeedRequest::followRedirect(const QString &url, const char *slot) {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(FeedRequest*)), reply, SLOT(deleteLater()));
}

QString TelegraphFeedRequest::getRedirect(const QNetworkReply *reply) {
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

void TelegraphFeedRequest::writeStartFeed() {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::writeStartFeed()";
#endif
    m_buffer.open(QBuffer::WriteOnly);
    m_writer.setDevice(&m_buffer);
    m_writer.writeStartDocument();
    m_writer.writeStartElement("rss");
    m_writer.writeAttribute("version", "2.0");
    m_writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    m_writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
    m_writer.writeStartElement("channel");
    m_writer.writeTextElement("description", tr("News articles from The Telegraph"));
    m_writer.writeStartElement("image");
    m_writer.writeTextElement("url", ICON_URL);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TelegraphFeedRequest::writeEndFeed() {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::writeEndFeed()";
#endif
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
    m_buffer.close();
}

void TelegraphFeedRequest::writeFeedTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(title);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TelegraphFeedRequest::writeFeedUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

void TelegraphFeedRequest::writeStartItem() {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::writeStartItem(). Item" << m_results << "of"
        << m_settings.value("maxResults", 20).toInt();
#endif
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("item");
    m_buffer.close();
}

void TelegraphFeedRequest::writeEndItem() {
#ifdef TELEGRAPH_DEBUG
    qDebug() << "TelegraphFeedRequest::writeEndItem(). Item" << m_results << "of"
        << m_settings.value("maxResults", 20).toInt();
#endif
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TelegraphFeedRequest::writeItemAuthor(const QString &author) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:creator", author);
    m_buffer.close();
}

void TelegraphFeedRequest::writeItemBody(const QString &body) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("content:encoded");
    m_writer.writeCDATA(body);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TelegraphFeedRequest::writeItemCategories(const QStringList &categories) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);

    foreach (const QString &category, categories) {
        m_writer.writeTextElement("category", category);
    }

    m_buffer.close();
}

void TelegraphFeedRequest::writeItemDate(const QDateTime &date) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("dc:date", date.toString(Qt::ISODate));
    m_buffer.close();
}

void TelegraphFeedRequest::writeItemEnclosures(const QVariantList &enclosures) {
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

void TelegraphFeedRequest::writeItemTitle(const QString &title) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeStartElement("title");
    m_writer.writeCDATA(title);
    m_writer.writeEndElement();
    m_buffer.close();
}

void TelegraphFeedRequest::writeItemUrl(const QString &url) {
    m_buffer.open(QBuffer::WriteOnly | QBuffer::Append);
    m_writer.writeTextElement("link", url);
    m_buffer.close();
}

TelegraphArticleRequest* TelegraphFeedRequest::articleRequest() {
    if (!m_request) {
        m_request = new TelegraphArticleRequest(this);
        connect(m_request, SIGNAL(finished(ArticleRequest*)), this, SLOT(checkArticle(ArticleRequest*)));
    }

    return m_request;
}

QNetworkAccessManager* TelegraphFeedRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
