/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "subscriptions.h"
#include "database.h"
#include "definitions.h"
#include "json.h"
#include "settings.h"
#include "subscription.h"
#include "utils.h"
#include "transfers.h"
#include <QProcess>
#include <QImage>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

static const int REQUEST_ID = 1001;
#ifdef USE_FAVICONS
static const QString FAVICONS_URL("http://www.google.com/s2/favicons?domain=");
#endif

Subscriptions* Subscriptions::self = 0;

Subscriptions::Subscriptions() :
    QObject(),
    m_xmlDownload(0),
    m_iconDownload(0),
    m_process(0),
    m_progress(0),
    m_status(Idle),
    m_total(0)
{
    connect(Database::instance(), SIGNAL(subscriptionAdded(int)), this, SLOT(update(int)));
    connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
}

Subscriptions::~Subscriptions() {
    self = 0;
}

Subscriptions* Subscriptions::instance() {
    return self ? self : self = new Subscriptions;
}

int Subscriptions::activeSubscription() const {
    return status() == Active ? Database::subscriptionId(m_query) : -1000;
}

int Subscriptions::progress() const {
    return m_progress;
}

void Subscriptions::setProgress(int p) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::setProgress" << p;
#endif
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

Subscriptions::Status Subscriptions::status() const {
    return m_status;
}

void Subscriptions::setStatus(Subscriptions::Status s) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::setStatus" << s;
#endif
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString Subscriptions::statusText() const {
    return m_statusText;
}

void Subscriptions::setStatusText(const QString &t) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::setStatusText" << t;
#endif
    if (t != statusText()) {
        m_statusText = t;
        emit statusTextChanged(t);
    }
}

void Subscriptions::cancel() {
    m_queue.clear();
    
    if (m_xmlDownload) {
        m_xmlDownload->cancel();
    }
    
    if (m_process) {
        m_process->kill();
    }
    
    setStatusText(tr("Canceled"));
    setStatus(Canceled);
}

void Subscriptions::update(int id) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::update" << id;
#endif
    m_queue << id;
    m_total++;
    
    if (status() != Active) {
        next();
    }
}

void Subscriptions::next() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::next";
#endif
    setProgress((m_total - m_queue.size()) * 100 / m_total);
    
    if (m_queue.isEmpty()) {
        setStatusText(tr("Finished"));
        setStatus(Finished);
        emit activeSubscriptionChanged(activeSubscription());
        return;
    }
    
    setStatus(Active);
    Database::fetchSubscription(m_queue.dequeue(), REQUEST_ID);
}

void Subscriptions::update() {
    const QString &source = Database::subscriptionSource(m_query);
    const int sourceType = Database::subscriptionSourceType(m_query);
    setStatusText(tr("Retrieving feed for %1").arg(source));
    
    if (sourceType == Subscription::Command) {
        if (!m_process) {
            m_process = new QProcess(this);
            connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int)));
            connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
        }
#ifdef CUTENEWS_DEBUG
        qDebug() << "Subscriptions::update: Updating subscription" << Database::subscriptionId(m_query)
                 << "using command" << source;
#endif
        m_process->start(source);
    }
    else {
        if (!m_xmlDownload) {
            m_xmlDownload = new Transfer(this);
            connect(m_xmlDownload, SIGNAL(statusChanged()), this, SLOT(onXmlDownloadStatusChanged()));
        }
#ifdef CUTENEWS_DEBUG
        qDebug() << "Subscriptions::update: Updating subscription" << Database::subscriptionId(m_query)
                 << "using URI" << source;
#endif
        m_xmlDownload->setUrl(Utils::isLocalFile(source) ? QUrl::fromLocalFile(source) : QUrl(source));
        m_xmlDownload->start();
    }
}

void Subscriptions::parseXml(const QByteArray &xml) {
    QDomDocument doc;
    
    if (!doc.setContent(xml)) {
        setStatusText(tr("Error parsing XML from %1").arg(Database::subscriptionSource(m_query)));
        setStatus(Error);
        return;
    }
    
    const QDomElement channel = doc.firstChildElement("rss").firstChildElement("channel");
    const QString title = channel.firstChildElement("title").text();
    const QString description = channel.firstChildElement("description").text();
    const QUrl url = channel.firstChildElement("link").text();
#ifndef USE_FAVICONS
    QString iconUrl;
    
    if (Database::subscriptionIconPath(m_query).isEmpty()) {
        QDomElement iconElement = channel.firstChildElement("image");
    
        if (iconElement.isNull()) {
            iconElement = channel.firstChildElement("itunes:image");
        
            if (!iconElement.isNull()) {
                iconUrl = iconElement.attribute("href");
            }
        }
        else {
            if (iconElement.hasAttribute("href")) {
                iconUrl = iconElement.attribute("href");
            }
            else if (iconElement.hasAttribute("src")) {
                iconUrl = iconElement.attribute("src");
            }
            else {
                iconUrl = iconElement.firstChildElement("url").text();
            }
        }
    }
#endif
    QVariantMap subscription;
    
    if (Database::subscriptionTitle(m_query) != title) {
        subscription["title"] = title;
    }
    
    if (Database::subscriptionDescription(m_query) != description) {
        subscription["description"] = description;
    }
    
    if (Database::subscriptionUrl(m_query) != url) {
        subscription["url"] = url;
    }
    
    const QDateTime lastUpdated = Database::subscriptionLastUpdated(m_query);
    const QDomNodeList items = channel.elementsByTagName("item");
    const QDateTime latest = items.isEmpty() ? QDateTime()
                                             : QDateTime::fromString(items.at(0).firstChildElement("pubDate").text()
                                               .section(' ', 0, -2), "ddd, dd MMM yyyy HH:mm:ss");

#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::parseXml:" << items.size() << "articles found. Latest article published at" << latest;
#endif
    if (latest <= lastUpdated) {
        if (!subscription.isEmpty()) {
            Database::updateSubscription(Database::subscriptionId(m_query), subscription);
        }
#ifdef USE_FAVICONS
        if ((!url.isEmpty()) && (Database::subscriptionIconPath(m_query).isEmpty())) {
            downloadIcon(FAVICONS_URL + url.host());
        }
        else {
            next();
        }
#else
        if (!iconUrl.isEmpty()) {
            downloadIcon(iconUrl);
        }
        else {
            next();
        }
#endif       
        return;
    }
        
    QVariantList ids;
    QVariantList authors;
    QVariantList bodies;
    QVariantList categories;
    QVariantList dates;
    QVariantList enclosures;
    QVariantList favourites;
    QVariantList reads;
    QVariantList subscriptionIds;
    QVariantList titles;
    QVariantList urls;
    
    for (int i = 0; i < items.size(); i++) {
        const QDomElement item = items.at(i).toElement();
        const QDateTime date = QDateTime::fromString(item.firstChildElement("pubDate").text().section(' ', 0, -2),
                                                     "ddd, dd MMM yyyy HH:mm:ss");
        
        if (date <= lastUpdated) {
            break;
        }
        
        dates << date;
        ids << QVariant(QVariant::Int);
        titles << item.firstChildElement("title").text();
        urls << item.firstChildElement("link").text();
        
        QDomElement authorElement = item.firstChildElement("dc:creator");
        
        if (authorElement.isNull()) {
            authorElement = item.firstChildElement("itunes:author");
        }
        
        authors << authorElement.text();
        
        QDomElement descriptionElement = item.firstChildElement("content:encoded");
        
        if (descriptionElement.isNull()) {
            descriptionElement = item.firstChildElement("description");
        }
        
        bodies << descriptionElement.text();
        
        QStringList itemCategories;
        QDomNodeList categoryNodes = item.elementsByTagName("category");
        
        if (categoryNodes.isEmpty()) {
            categoryNodes = item.elementsByTagName("itunes:keywords");
        }
        
        for (int i = 0; i < categoryNodes.size(); i++) {
            itemCategories << categoryNodes.at(i).toElement().text();
        }
        
        QVariantList itemEnclosures;
        QDomNodeList enclosureNodes = item.elementsByTagName("enclosure");
        
        if (enclosureNodes.isEmpty()) {
            enclosureNodes = item.elementsByTagName("media:content");
        }
        
        for (int i = 0; i < enclosureNodes.size(); i++) {
            const QDomElement enclosureElement = enclosureNodes.at(i).toElement();
            const int enclosureSize = enclosureElement.hasAttribute("length")
                                      ? enclosureElement.attribute("length").toInt()
                                      : enclosureElement.attribute("fileSize").toInt();
            
            if (enclosureSize > 0) {
                const QString enclosureUrl = enclosureElement.attribute("url");
                const QString enclosureType = enclosureElement.attribute("type");
                
                QVariantMap enclosure;
                enclosure["url"] = enclosureUrl;
                enclosure["type"] = enclosureType;
                enclosure["length"] = enclosureSize;            
                itemEnclosures << enclosure;
                
                if (Database::subscriptionDownloadEnclosures(m_query)) {
                    Transfers::instance()->addDownloadTransfer(enclosureUrl);
                }
            }
        }
        
        categories << itemCategories.join(", ");
        enclosures << QtJson::Json::serialize(itemEnclosures);
        favourites << 0;
        reads << 0;
        subscriptionIds << Database::subscriptionId(m_query);
    }
    
    subscription["lastUpdated"] = latest;

    Database::addArticles(QList<QVariantList>() << ids << authors << bodies << categories << dates << enclosures
                          << favourites << reads << subscriptionIds << titles << urls,
                          Database::subscriptionId(m_query));
    Database::updateSubscription(Database::subscriptionId(m_query), subscription);
#ifdef USE_FAVICONS
    if ((!url.isEmpty()) && (Database::subscriptionIconPath(m_query).isEmpty())) {
        downloadIcon(FAVICONS_URL + url.host());
    }
    else {
        next();
    }
#else
    if (!iconUrl.isEmpty()) {
        downloadIcon(iconUrl);
    }
    else {
        next();
    }
#endif
}

void Subscriptions::downloadIcon(const QUrl &url) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::downloadIcon" << url;
#endif
    if (!m_iconDownload) {
        m_iconDownload = new Transfer(this);
        connect(m_iconDownload, SIGNAL(statusChanged()), this, SLOT(onIconDownloadStatusChanged()));
    }
    
    m_iconDownload->setUrl(url);
    m_iconDownload->start();
}

void Subscriptions::onXmlDownloadStatusChanged() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::onXmlDownloadStatusChanged" << m_xmlDownload->statusString();
#endif
    switch (m_xmlDownload->status()) {
    case Transfer::Completed: {
        const QByteArray response = m_xmlDownload->readAll();
        
        if (!response.isEmpty()) {
            parseXml(response);
            return;
        }
        
        break;
    }
    case Transfer::Canceled:
        break;
    case Transfer::Failed:
        setStatusText(tr("Error retrieving feed from %1: %2").arg(Database::subscriptionSource(m_query))
                                                             .arg(m_xmlDownload->errorString()));
        break;
    default:
        return;
    }
    
    next();
}

void Subscriptions::onIconDownloadStatusChanged() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::onIconDownloadStatusChanged" << m_iconDownload->statusString();
#endif
    switch (m_iconDownload->status()) {
    case Transfer::Completed: {    
        QImage image = QImage::fromData(m_iconDownload->readAll());
        
        if (!image.isNull()) {
            if (image.height() > ICON_SIZE) {
                image = image.scaledToHeight(ICON_SIZE, Qt::SmoothTransformation);
            }
            
            if (QDir().mkpath(ICON_PATH)) {
                const QString ext = m_iconDownload->url().path().section('.', -1);
                const QString fileName = QString("%1%2.%3").arg(ICON_PATH).arg(Database::subscriptionId(m_query))
                                                           .arg(ext.isEmpty() ? QString("png") : ext);
            
                if ((!image.isNull()) && (image.save(fileName))) {
                    QVariantMap subscription;
                    subscription["iconPath"] = fileName;
                    Database::updateSubscription(Database::subscriptionId(m_query), subscription);
                }
            }
        }
        
        break;
    }
    case Transfer::Canceled:
        break;
    case Transfer::Failed:
        break;
    default:
        return;
    }

    next();
}

void Subscriptions::onProcessError() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::onProcessError" << m_process->errorString();
#endif
    setStatusText(tr("Error retrieving feed from %1: %2").arg(Database::subscriptionSource(m_query))
                                                         .arg(m_process->errorString()));
    next();
}

void Subscriptions::onProcessFinished(int exitCode) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::onProcessFinished" << exitCode;
#endif
    if (exitCode == 0) {
        const QByteArray response = m_process->readAllStandardOutput();
        
        if (!response.isEmpty()) {
            parseXml(response);
            return;
        }
    }
    
    setStatusText(tr("Error retrieving feed from %1: %2").arg(Database::subscriptionSource(m_query))
                                                         .arg(m_process->errorString()));
    next();
}

void Subscriptions::onSubscriptionFetched(const QSqlQuery &query, int requestId) {
    if (requestId == REQUEST_ID) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "Subscriptions::onSubscriptionFetched" << Database::subscriptionId(query);
#endif
        m_query = query;
        emit activeSubscriptionChanged(activeSubscription());
        update();
    }    
}
