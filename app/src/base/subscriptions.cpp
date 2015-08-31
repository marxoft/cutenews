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
#include "feedparser.h"
#include "json.h"
#include "opmlparser.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionplugins.h"
#include "utils.h"
#include "transfers.h"
#include <QProcess>
#include <QImage>
#include <QDir>
#include <QDBusConnection>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

static const int REQUEST_ID = Utils::createId();
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
    connect(Database::instance(), SIGNAL(subscriptionsAdded(int)), this, SLOT(onSubscriptionsAdded(int)));
    
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews.subscriptions");
    connection.registerObject("/org/marxoft/cutenews/subscriptions", this, QDBusConnection::ExportScriptableSlots);
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
    if (status() != Active) {
        return;
    }
    
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

bool Subscriptions::create(const QString &source, int sourceType, bool downloadEnclosures) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::create" << source << sourceType << downloadEnclosures;
#endif
    return Database::addSubscription(QVariantList() << QVariant(QVariant::Int) << 0 << QString()
                                                    << (downloadEnclosures ? 1 : 0) << QString() << QString() << source
                                                    << sourceType << tr("New subscription") << 0 << QString());
}

bool Subscriptions::importFromOpml(const QString &fileName, bool downloadEnclosures) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::importFromOpml" << fileName << downloadEnclosures;
#endif
    QFile file(fileName);
    OpmlParser parser(&file);
    
    if (!parser.readHead()) {
        return false;
    }
    
    QVariantList ids;
    QVariantList cacheSizes;
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
            ids << QVariant(QVariant::Int);
            cacheSizes << 0;
            descriptions << parser.description();
            downloads << (downloadEnclosures ? 1 : 0);
            icons << QString();
            dates << QString();
            sources << parser.xmlUrl();
            sourceTypes << Subscription::Url;
            titles << (parser.title().isEmpty() ? tr("New subscription") : parser.title());
            intervals << 0;
            urls << parser.htmlUrl();
        }
    }
    
    return (!ids.isEmpty()) && (Database::addSubscriptions(QList<QVariantList>() << ids << cacheSizes << descriptions
                                                           << downloads << icons << dates << sources << sourceTypes
                                                           << titles << intervals << urls));
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

bool Subscriptions::updateAll() {
#ifdef CUTENEWS_DEBUG
    qDebug() << "Subscriptions::updateAll";
#endif
    if (status() != Active) {
        connect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                this, SLOT(onSubscriptionIdsFetched(QSqlQuery, int)));
        Database::execQuery(QString("SELECT id FROM subscriptions"), REQUEST_ID);
        return true;
    }
    
    return false;
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
    connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
    Database::fetchSubscription(m_queue.dequeue(), REQUEST_ID);
}

void Subscriptions::update() {
    const QVariant source = Database::subscriptionSource(m_query);
    const int sourceType = Database::subscriptionSourceType(m_query);
    setStatusText(tr("Retrieving feed for %1").arg(Database::subscriptionTitle(m_query)));
    
    if (sourceType == Subscription::Plugin) {
        if (!m_process) {
            m_process = new QProcess(this);
            connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int)));
            connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
        }
        
        const QVariantMap plugin = source.toMap();
        QString command = SubscriptionPlugins::command(plugin.value("pluginName").toString());
        
        if (plugin.contains("params")) {
            QMapIterator<QString, QVariant> iterator(plugin.value("params").toMap());
            
            while(iterator.hasNext()) {
                iterator.next();
                command.append(" -");
                command.append(iterator.key());
                command.append(" ");
                command.append(QtJson::Json::serialize(iterator.value()));
            }
        }
#ifdef CUTENEWS_DEBUG
        qDebug() << "Subscriptions::update: Updating subscription" << Database::subscriptionId(m_query)
                 << "using plugin:" << plugin.value("pluginName").toString() << "with command:" << command;
#endif
        m_process->start(command);
    }
    else {
        const QString sourceString = source.toString();
        
        if (sourceType == Subscription::Command) {
            if (!m_process) {
                m_process = new QProcess(this);
                connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int)));
                connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
            }
#ifdef CUTENEWS_DEBUG
            qDebug() << "Subscriptions::update: Updating subscription" << Database::subscriptionId(m_query)
                     << "using command:" << sourceString;
#endif
            m_process->start(sourceString);
        }
        else {
            if (!m_xmlDownload) {
                m_xmlDownload = new Transfer(this);
                connect(m_xmlDownload, SIGNAL(statusChanged()), this, SLOT(onXmlDownloadStatusChanged()));
            }
#ifdef CUTENEWS_DEBUG
            qDebug() << "Subscriptions::update: Updating subscription" << Database::subscriptionId(m_query)
                     << "using URI:" << sourceString;
#endif
            m_xmlDownload->setUrl(Utils::isLocalFile(sourceString) ? QUrl::fromLocalFile(sourceString)
                                                                   : QUrl(sourceString));
            m_xmlDownload->start();
        }
    }
}

void Subscriptions::parseXml(const QByteArray &xml) {
    FeedParser parser(xml);
    
    if (!parser.readChannel()) {
        setStatusText(tr("Error parsing XML for %1").arg(Database::subscriptionTitle(m_query)));
        setStatus(Error);
        return;
    }
    
    const QString channelTitle = parser.title();
    const QString channelDescription = parser.description();
    const QUrl channelUrl = parser.url();
    const QString channelIconUrl = parser.iconUrl();

    if (!parser.readNextArticle()) {
        setStatusText(tr("Error parsing XML for %1").arg(Database::subscriptionTitle(m_query)));
        setStatus(Error);
        return;
    }
    
    const int subscriptionId = Database::subscriptionId(m_query);
    const QDateTime lastUpdated = Database::subscriptionLastUpdated(m_query);
    
    QVariantMap subscription;
    subscription["title"] = channelTitle;
    subscription["description"] = channelDescription;
    subscription["url"] = channelUrl;
    
    if (parser.date() <= lastUpdated) {
        Database::updateSubscription(subscriptionId, subscription);
        
        if (Database::subscriptionIconPath(m_query).isEmpty()) {
            if (!channelIconUrl.isEmpty()) {
                downloadIcon(channelIconUrl);
                return;
            }
#ifdef USE_FAVICONS
            if (!channelUrl.isEmpty()) {
                downloadIcon(FAVICONS_URL + channelUrl.host());
                return;
            }
#endif
        }
    
        next();
        return;
    }
        
    QVariantList ids = QVariantList() << QVariant(QVariant::Int);
    QVariantList authors = QVariantList() << parser.author();
    QVariantList bodies = QVariantList() << parser.description();
    QVariantList categories = QVariantList() << parser.categories().join(", ");
    QVariantList dates = QVariantList() << parser.date();
    QVariantList enclosures = QVariantList() << QtJson::Json::serialize(parser.enclosures());
    QVariantList favourites = QVariantList() << 0;
    QVariantList reads = QVariantList() << 0;
    QVariantList subscriptionIds = QVariantList() << subscriptionId;
    QVariantList titles = QVariantList() << parser.title();
    QVariantList urls = QVariantList() << parser.url();
    
    while ((parser.readNextArticle()) && (parser.date() > lastUpdated)) {
        ids << QVariant(QVariant::Int);
        authors << parser.author();
        bodies << parser.description();
        categories << parser.categories().join(", ");
        dates << parser.date();
        enclosures << QtJson::Json::serialize(parser.enclosures());
        favourites << 0;
        reads << 0;
        subscriptionIds << subscriptionId;
        titles << parser.title();
        urls << parser.url();
    }

    subscription["lastUpdated"] = dates.first();

    Database::addArticles(QList<QVariantList>() << ids << authors << bodies << categories << dates << enclosures
                          << favourites << reads << subscriptionIds << titles << urls,
                          subscriptionId);
    Database::updateSubscription(subscriptionId, subscription);
    
    if (Database::subscriptionIconPath(m_query).isEmpty()) {
        if (!channelIconUrl.isEmpty()) {
            downloadIcon(channelIconUrl);
            return;
        }
#ifdef USE_FAVICONS
        if (!channelUrl.isEmpty()) {
            downloadIcon(FAVICONS_URL + channelUrl.host());
            return;
        }
#endif
    }
    
    next();
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
        setStatusText(tr("Error retrieving feed for %1: %2").arg(Database::subscriptionTitle(m_query))
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
                const QString fileName = QString("%1%2.png").arg(ICON_PATH).arg(Database::subscriptionId(m_query));
            
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
    setStatusText(tr("Error retrieving feed for %1: %2").arg(Database::subscriptionTitle(m_query))
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
    
    setStatusText(tr("Error retrieving feed for %1: %2").arg(Database::subscriptionTitle(m_query))
                                                        .arg(m_process->errorString()));
    next();
}

void Subscriptions::onSubscriptionsAdded(int count) {
    if (status() != Active) {
        connect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                this, SLOT(onSubscriptionIdsFetched(QSqlQuery, int)));
        Database::execQuery(QString("SELECT id FROM subscriptions ORDER BY id DESC LIMIT %1").arg(count), REQUEST_ID);
    }
}

void Subscriptions::onSubscriptionFetched(const QSqlQuery &query, int requestId) {
    if (requestId == REQUEST_ID) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "Subscriptions::onSubscriptionFetched" << Database::subscriptionId(query);
#endif
        disconnect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
        m_query = query;
        emit activeSubscriptionChanged(activeSubscription());
        update();
    }    
}

void Subscriptions::onSubscriptionIdsFetched(QSqlQuery query, int requestId) {
    if (requestId == REQUEST_ID) {
#ifdef CUTENEWS_DEBUG
        qDebug() << "Subscriptions::onSubscriptionIdsFetched";
#endif
        disconnect(Database::instance(), SIGNAL(queryReady(QSqlQuery, int)),
                   this, SLOT(onSubscriptionIdsFetched(QSqlQuery, int)));
        
        while (query.next()) {
            update(query.value(0).toInt());
        }
    }
}
