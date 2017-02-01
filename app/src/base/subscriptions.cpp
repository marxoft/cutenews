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

#include "subscriptions.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "download.h"
#include "feedparser.h"
#include "feedrequest.h"
#include "json.h"
#include "logger.h"
#include "opmlparser.h"
#include "pluginmanager.h"
#include "settings.h"
#include "subscription.h"
#include "transfers.h"
#include "utils.h"
#include <QProcess>
#include <QImage>
#include <QDir>
#ifdef DBUS_INTERFACE
#include <QDBusConnection>
#endif

#ifdef USE_FAVICONS
const QString Subscriptions::FAVICONS_URL("http://www.google.com/s2/favicons?domain=");
#endif

Subscriptions* Subscriptions::self = 0;

Subscriptions::Subscriptions() :
    QObject(),
    m_feedDownloader(0),
    m_iconDownloader(0),
    m_feedRequest(0),
    m_subscription(0),
    m_process(0),
    m_progress(0),
    m_status(Idle),
    m_total(0)
{
    m_updateTimer.setInterval(60000);
    
    connect(DBNotify::instance(), SIGNAL(subscriptionsAdded(QStringList)), this, SLOT(update(QStringList)));
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(getScheduledUpdates()));
#ifdef DBUS_INTERFACE
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews.subscriptions");
    connection.registerObject("/org/marxoft/cutenews/subscriptions", this, QDBusConnection::ExportScriptableSlots);
#endif
}

Subscriptions::~Subscriptions() {
    self = 0;
}

Subscriptions* Subscriptions::instance() {
    return self ? self : self = new Subscriptions;
}

QString Subscriptions::activeSubscription() const {
    return (status() == Active) && (m_subscription) ? m_subscription->id() : QString();
}

int Subscriptions::progress() const {
    return m_progress;
}

void Subscriptions::setProgress(int p) {
    if (p != progress()) {
        m_progress = p;
        emit progressChanged(p);
    }
}

bool Subscriptions::offlineModeEnabled() const {
    return !m_updateTimer.isActive();
}

void Subscriptions::setOfflineModeEnabled(bool enabled) {
    if (enabled != offlineModeEnabled()) {
        if (enabled) {
            m_updateTimer.stop();
        }
        else {
            m_updateTimer.start();
        }

        emit offlineModeEnabledChanged(enabled);
    }
}

int Subscriptions::scheduledUpdatesInterval() const {
    return qMax(1000, m_updateTimer.interval()) / 1000;
}

void Subscriptions::setScheduledUpdatesInterval(int interval) {
    interval = qMax(1, interval);
    
    if (interval != scheduledUpdatesInterval()) {
        m_updateTimer.setInterval(interval * 1000);
        emit scheduledUpdatesIntervalChanged(interval);
    }
}

void Subscriptions::getScheduledUpdates() {
    const uint lastChecked = QDateTime::currentDateTime().toTime_t() - scheduledUpdatesInterval();
    Logger::log("Subscriptions::getScheduledUpdates(). Fetching subscriptions due for update since " +
                QDateTime::fromTime_t(lastChecked).toString(Qt::ISODate), Logger::MediumVerbosity);
    DBConnection *conn = DBConnection::connection(this, SLOT(onSubscriptionIdsFetched(DBConnection*)));
    conn->exec(QString("SELECT id FROM subscriptions WHERE updateInterval > 0 AND lastUpdated < %1 - updateInterval ORDER BY rowid ASC").arg(lastChecked));
}

Subscriptions::Status Subscriptions::status() const {
    return m_status;
}

void Subscriptions::setStatus(Subscriptions::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString Subscriptions::statusText() const {
    return m_statusText;
}

void Subscriptions::setStatusText(const QString &t) {
    if (t != statusText()) {
        m_statusText = t;
        emit statusTextChanged(t);
    }
}

void Subscriptions::cancel() {
    if (status() != Active) {
        return;
    }
    
    m_queue.clear();
    
    if (m_feedDownloader) {
        m_feedDownloader->cancel();
    }
    
    if (m_feedRequest) {
        m_feedRequest->cancel();
    }   
}

QString Subscriptions::create(const QString &source, int sourceType, bool downloadEnclosures, int updateInterval) {
    const QString id = Utils::createId();
    Logger::log("Subscriptions::create(). Source: " + source, Logger::LowVerbosity);
    DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->addSubscription(QVariantList() 
                             << id << QString() << (downloadEnclosures ? 1 : 0)
                             << QString() << 0 << source << sourceType << tr("New subscription") << updateInterval
                             << QString());
    return id;
}

bool Subscriptions::importFromOpml(const QString &fileName, bool downloadEnclosures, int updateInterval) {
    Logger::log("Subscriptions::importFromOpml(). Filename: " + fileName, Logger::LowVerbosity);
    QFile file(fileName);
    OpmlParser parser(&file);
    
    if (!parser.readHead()) {
        return false;
    }
    
    QVariantList ids;
    QVariantList descriptions;
    QVariantList downloads;
    QVariantList icons;
    QVariantList dates;
    QVariantList sources;
    QVariantList sourceTypes;
    QVariantList titles;
    QVariantList intervals;
    QVariantList urls;
    
    while (parser.readNextSubscription()) {
        if (!parser.xmlUrl().isEmpty()) {
            ids << Utils::createId();
            descriptions << parser.description();
            downloads << (downloadEnclosures ? 1 : 0);
            icons << QString();
            dates << 0;
            sources << parser.xmlUrl();
            sourceTypes << Subscription::Url;
            titles << (parser.title().isEmpty() ? tr("New subscription") : parser.title());
            intervals << updateInterval;
            urls << parser.htmlUrl();
        }
    }
    
    if (ids.isEmpty()) {
        return false;
    }
    
    DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->addSubscriptions(QList<QVariantList>()
                             << ids << descriptions << downloads << icons << dates << sources << sourceTypes << titles
                             << intervals << urls);
    return true;
}

void Subscriptions::update(const QString &id) {
    Logger::log("Subscriptions::update(). ID: " + id, Logger::LowVerbosity);
    m_queue.enqueue(id);
    ++m_total;
    
    if (status() != Active) {
        next();
    }
}

void Subscriptions::update(const QStringList &ids) {
    foreach (const QString &id, ids) {
        update(id);
    }
}

bool Subscriptions::updateAll() {
    Logger::log("Subscriptions::updateAll()", Logger::LowVerbosity);
    
    if (status() != Active) {
        DBConnection::connection(this, SLOT(onSubscriptionIdsFetched(DBConnection*)))->exec("SELECT id FROM subscriptions ORDER BY rowid ASC");
        return true;
    }
    
    return false;
}

void Subscriptions::next() {
    setProgress((m_total - m_queue.size()) * 100 / m_total);
    
    if (m_queue.isEmpty()) {
        setStatusText(tr("Finished"));
        setStatus(Finished);
        emit activeSubscriptionChanged(QString());
        return;
    }
    
    setStatus(Active);
    subscription()->load(m_queue.dequeue());
}

void Subscriptions::update() {
    setStatusText(tr("Retrieving feed for %1").arg(subscription()->title()));
    
    if (subscription()->sourceType() == Subscription::Plugin) {
        if (!m_process) {
            m_process = new QProcess(this);
            connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int)));
            connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
        }
        
        const QVariantMap source = subscription()->source().toMap();
        FeedRequest *request = feedRequest(source.value("pluginId").toString());
        
        if (request) {
            request->getFeed(source.value("settings").toMap());
        }
        else {
            next();
        }
    }
    else {
        QString sourceString = subscription()->source().toString();
        
        if (subscription()->sourceType() == Subscription::Command) {
            Logger::log(QString("Subscription::update(). Updating feed '%1' using command '%2'")
                               .arg(subscription()->title()).arg(sourceString));
            process()->start(sourceString);
        }
        else {
            if (subscription()->sourceType() == Subscription::LocalFile) {
                if (sourceString.startsWith("/")) {
                    sourceString.prepend("file://");
                }
                else if (!sourceString.startsWith("file:/")) {
                    sourceString.prepend("file:///");
                }
            }
            
            Logger::log(QString("Subscription::update(). Updating feed '%1' using URL '%2'")
                               .arg(subscription()->title()).arg(sourceString));
            feedDownloader()->setUrl(sourceString);
            feedDownloader()->start();
        }
    }
}

static QString replaceImageUrls(const QString &body, const QString &cachePath) {
    QString result(body);
    const QRegExp src(" src=('|\")([^'\"]+)");
    int pos = 0;

    while ((pos = src.indexIn(body, pos)) != -1) {
        const QString url = src.cap(2);
        result.replace(url, cachePath + url.toUtf8().toBase64());
        pos += src.matchedLength();
    }

    return result;
}

void Subscriptions::parseXml(const QByteArray &xml) {
    FeedParser parser(xml);
    
    if (!parser.readChannel()) {
        setStatusText(tr("Error parsing XML for %1").arg(subscription()->title()));
        setStatus(Error);
        next();
        return;
    }
    
    const QString channelTitle = parser.title();
    const QString channelDescription = parser.description();
    const QString channelUrl = parser.url();
    const QString channelIconUrl = parser.iconUrl();

    if ((!parser.readNextArticle()) || (!parser.date().isValid())) {
        setStatusText(tr("Error parsing XML for %1").arg(subscription()->title()));
        setStatus(Error);
        next();
        return;
    }
    
    const QString subscriptionId = subscription()->id();
    const QDateTime lastUpdated = subscription()->lastUpdated();
    
    QVariantMap sub;
    sub["title"] = channelTitle;
    sub["description"] = channelDescription;
    sub["url"] = channelUrl;
    sub["lastUpdated"] = QDateTime::currentDateTime().toTime_t();
    
    if (parser.date() <= lastUpdated) {
        DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->updateSubscription(subscriptionId,
                                                                                                      sub);
        
        if (subscription()->iconPath().isEmpty()) {
            if (!channelIconUrl.isEmpty()) {
                downloadIcon(channelIconUrl);
                return;
            }
#ifdef USE_FAVICONS
            if (!channelUrl.isEmpty()) {
                downloadIcon(FAVICONS_URL + QUrl(channelUrl).host());
                return;
            }
#endif
        }
    
        next();
        return;
    }

    QString id = Utils::createId();
    QVariantList enc = parser.enclosures();    
    QVariantList ids = QVariantList() << id;
    QVariantList authors = QVariantList() << parser.author();
    QVariantList bodies = QVariantList() << replaceImageUrls(parser.description(), QString("%1%2/%3/")
                                                             .arg(CACHE_PATH).arg(subscriptionId).arg(id));
    QVariantList categories = QVariantList() << parser.categories().join(", ");
    QVariantList dates = QVariantList() << parser.date().toTime_t();
    QVariantList enclosures = QVariantList() << QtJson::Json::serialize(enc);
    QVariantList favourites = QVariantList() << 0;
    QVariantList reads = QVariantList() << 0;
    QVariantList lastReads = QVariantList() << 0;
    QVariantList subscriptionIds = QVariantList() << subscriptionId;
    QVariantList titles = QVariantList() << parser.title();
    QVariantList urls = QVariantList() << parser.url();
    
    if (subscription()->downloadEnclosures()) {
        foreach (const QVariant &e, enc) {
            Transfers::instance()->addEnclosureDownload(e.toMap().value("url").toString());
        }
    }
    
    while ((parser.readNextArticle()) && (parser.date() > lastUpdated)) {
        id = Utils::createId();
        enc = parser.enclosures();
        ids << id;
        authors << parser.author();
        bodies << replaceImageUrls(parser.description(), QString("%1%2/%3/").arg(CACHE_PATH).arg(subscriptionId)
                                   .arg(id));
        categories << parser.categories().join(", ");
        dates << parser.date().toTime_t();
        enclosures << QtJson::Json::serialize(enc);
        favourites << 0;
        reads << 0;
        lastReads << 0;
        subscriptionIds << subscriptionId;
        titles << parser.title();
        urls << parser.url();

        if (subscription()->downloadEnclosures()) {
            foreach (const QVariant &e, enc) {
                Transfers::instance()->addEnclosureDownload(e.toMap().value("url").toString(), subscriptionId);
            }
        }
    }
    
    DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->addArticles(QList<QVariantList>()
                             << ids << authors << bodies << categories << dates << enclosures << favourites << reads
                             << lastReads << subscriptionIds << titles << urls, subscriptionId);
    DBConnection::connection(this, SLOT(onConnectionFinished(DBConnection*)))->updateSubscription(subscriptionId,
                             sub);
    
    if (subscription()->iconPath().isEmpty()) {
        if (!channelIconUrl.isEmpty()) {
            downloadIcon(channelIconUrl);
            return;
        }
#ifdef USE_FAVICONS
        if (!channelUrl.isEmpty()) {
            downloadIcon(FAVICONS_URL + QUrl(channelUrl).host());
            return;
        }
#endif
    }
    
    next();
}

void Subscriptions::downloadIcon(const QString &url) {
    iconDownloader()->setUrl(url);
    iconDownloader()->start();
}

Download* Subscriptions::feedDownloader() {
    if (!m_feedDownloader) {
        m_feedDownloader = new Download(this);
        m_feedDownloader->setId(Utils::createId());
        connect(m_feedDownloader, SIGNAL(finished(Transfer*)), this, SLOT(onFeedDownloadFinished()));
    }
    
    return m_feedDownloader;
}

Download* Subscriptions::iconDownloader() {
    if (!m_iconDownloader) {
        m_iconDownloader = new Download(this);
        m_iconDownloader->setId(Utils::createId());
        connect(m_iconDownloader, SIGNAL(finished(Transfer*)), this, SLOT(onIconDownloadFinished()));
    }
    
    return m_iconDownloader;
}

FeedRequest* Subscriptions::feedRequest(const QString &pluginId) {
    if (m_feedRequest) {
        delete m_feedRequest;
    }

    m_feedRequest = PluginManager::instance()->feedRequest(pluginId, this);

    if (m_feedRequest) {
        connect(m_feedRequest, SIGNAL(finished(FeedRequest*)), this, SLOT(onFeedRequestFinished(FeedRequest*)));
    }

    return m_feedRequest;
}

Subscription* Subscriptions::subscription() {
    if (!m_subscription) {
        m_subscription = new Subscription(this);
        connect(m_subscription, SIGNAL(finished(Subscription*)), this, SLOT(onSubscriptionFetched(Subscription*)));
    }
    
    return m_subscription;
}

QProcess* Subscriptions::process() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int)));
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
    }
    
    return m_process;
}

void Subscriptions::onFeedDownloadFinished() {
    switch (m_feedDownloader->status()) {
    case Download::Completed: {
        const QByteArray response = m_feedDownloader->readAll();
        
        if (!response.isEmpty()) {
            parseXml(response);
            return;
        }
        
        break;
    }
    case Download::Canceled:
        setStatusText(tr("Canceled"));
        setStatus(Canceled);
        return;
    case Download::Failed:
        setStatusText(tr("Error retrieving feed for %1: %2").arg(subscription()->title())
                                                            .arg(m_feedDownloader->errorString()));
        break;
    default:
        return;
    }
    
    next();
}

void Subscriptions::onIconDownloadFinished() {
    switch (m_iconDownloader->status()) {
    case Download::Completed: {    
        QImage image = QImage::fromData(m_iconDownloader->readAll());
        
        if (!image.isNull()) {
            if (image.height() > ICON_SIZE) {
                image = image.scaledToHeight(ICON_SIZE, Qt::SmoothTransformation);
            }
            
            if (QDir().mkpath(CACHE_PATH + subscription()->id())) {
                const QString fileName = QString("%1%2/icon.png").arg(CACHE_PATH).arg(subscription()->id());
            
                if ((!image.isNull()) && (image.save(fileName))) {
                    QVariantMap properties;
                    properties["iconPath"] = fileName;
                    DBConnection::connection(this,
                    SLOT(onConnectionFinished(DBConnection*)))->updateSubscription(subscription()->id(), properties);
                }
            }
        }
        
        break;
    }
    case Download::Canceled:
        setStatusText(tr("Canceled"));
        setStatus(Canceled);
        emit activeSubscriptionChanged(QString());
        return;
    case Download::Failed:
        break;
    default:
        return;
    }

    next();
}

void Subscriptions::onFeedRequestFinished(FeedRequest *request) {    
    switch (request->status()) {
    case FeedRequest::Ready:
        parseXml(request->result());
        return;
    case FeedRequest::Canceled:
        setStatusText(tr("Canceled"));
        setStatus(Canceled);
        emit activeSubscriptionChanged(QString());
        return;
    case FeedRequest::Error:
        setStatusText(tr("Error retrieving feed for %1: %2").arg(subscription()->title())
                                                            .arg(request->errorString()));
        break;
    default:
        break;
    }
    
    next();
}

void Subscriptions::onProcessError() {
    Logger::log("Subscriptions::onProcessError(). Error: " + m_process->errorString());
    setStatusText(tr("Error retrieving feed for %1: %2").arg(subscription()->title()).arg(m_process->errorString()));
    next();
}

void Subscriptions::onProcessFinished(int exitCode) {
    Logger::log("Subscriptions::onProcessFinished(). Exit code: "+ QString::number(exitCode), Logger::LowVerbosity);
    
    if (exitCode == 0) {
        const QByteArray response = m_process->readAllStandardOutput();
        
        if (!response.isEmpty()) {
            parseXml(response);
            return;
        }
    }
    
    setStatusText(tr("Error retrieving feed for %1: %2").arg(subscription()->title())
                                                        .arg(m_process->errorString()));
    next();
}

void Subscriptions::onSubscriptionFetched(Subscription *subscription) {
    if (subscription->status() == Subscription::Ready) {
        emit activeSubscriptionChanged(activeSubscription());
        update();
    }
}

void Subscriptions::onSubscriptionIdsFetched(DBConnection *connection) {    
    if (connection->status() == DBConnection::Ready) {        
        while (connection->nextRecord()) {
            const QString id = connection->value(0).toString();

            if (!m_queue.contains(id)) {
                update(id);
            }
        }
    }
    
    connection->deleteLater();
}

void Subscriptions::onConnectionFinished(DBConnection *connection) {
    connection->deleteLater();
}
