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

#include "youtubefeedrequest.h"
#include <qyoutube/resourcesrequest.h>
#include <QXmlStreamWriter>
#include <QSettings>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

const QString YouTubeFeedRequest::API_KEY("AIzaSyDhIlkLzHJKDCNr6thsjlQpZrkY3lO_Uu4");
const QString YouTubeFeedRequest::DEFAULT_ICON("http://www.google.com/s2/favicons?domain=youtube.com");
#if QT_VERSION >= 0x050000
const QString YouTubeFeedRequest::CONFIG_FILE(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                                              + "/.config/cutenews/plugins/cutenews-youtube");
#else
const QString YouTubeFeedRequest::CONFIG_FILE(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                                              + "/.config/cutenews/plugins/cutenews-youtube");
#endif

YouTubeFeedRequest::YouTubeFeedRequest(QObject *parent) :
    FeedRequest(parent),
    m_request(0),
    m_status(Idle)
{
}

QString YouTubeFeedRequest::errorString() const {
    return m_errorString;
}

void YouTubeFeedRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

QByteArray YouTubeFeedRequest::result() const {
    return m_result;
}

void YouTubeFeedRequest::setResult(const QByteArray &r) {
    m_result = r;
}

YouTubeFeedRequest::Status YouTubeFeedRequest::status() const {
    return m_status;
}

void YouTubeFeedRequest::setStatus(YouTubeFeedRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool YouTubeFeedRequest::cancel() {
    if (status() == Active) {
        if (m_request) {
            m_request->cancel();
        }

        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool YouTubeFeedRequest::getFeed(const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(QByteArray());
    m_settings = settings;
    
    if (settings.value("type") == "channel") {
        QSettings config(CONFIG_FILE, QSettings::IniFormat);
        config.beginGroup("channels");
        config.beginGroup(settings.value("query").toString());
        const QString id = config.value("id").toString();

        if (!id.isEmpty()) {
            m_settings["icon"] = config.value("icon");
            getChannelVideos(id);
        }
        else {
            getChannels();
        }

        config.endGroup();
        config.endGroup();
    }
    else {
        m_settings["icon"] = DEFAULT_ICON;
        searchVideos();
    }

    return true;
}

QYouTube::ResourcesRequest* YouTubeFeedRequest::request(const char *slot) {
    if (!m_request) {
        m_request = new QYouTube::ResourcesRequest(this);
        m_request->setApiKey(API_KEY);
    }
    else {
        disconnect(m_request, SIGNAL(finished()), this, 0);
    }

    connect(m_request, SIGNAL(finished()), this, slot);
    return m_request;
}

void YouTubeFeedRequest::getChannels() {
    QVariantMap params;
    params["q"] = m_settings.value("query");
    params["type"] = "channel";
    params["maxResults"] = 20;
    params["order"] = "relevance";
    request(SLOT(checkChannels()))->list("/search", QStringList() << "snippet", QVariantMap(), params);
}

void YouTubeFeedRequest::checkChannels() {
    if (m_request->status() == QYouTube::ResourcesRequest::Ready) {
        const QVariantList items = m_request->result().toMap().value("items").toList();
        const QString query = m_settings.value("query").toString();
        const QString titleQuery = query.toLower();

        foreach (const QVariant &item, items) {
            const QVariantMap channel = item.toMap();
            const QVariantMap snippet = channel.value("snippet").toMap();
            const QString id = channel.value("id").toMap().value("channelId").toString();
            const QString title = snippet.value("title").toString().toLower();

            if ((id == query) || (title == titleQuery)) {
                const QString icon = snippet.value("thumbnails").toMap().value("default").toMap()
                                            .value("url").toString();
                m_settings["icon"] = icon;
                QSettings settings(CONFIG_FILE, QSettings::IniFormat);
                settings.beginGroup("channels");
                settings.beginGroup(query);
                settings.setValue("id", id);
                settings.setValue("icon", icon);
                settings.endGroup();
                settings.endGroup();
                getChannelVideos(id);
                return;
            }
        }

        setErrorString(tr("Channel not found"));
        setStatus(Error);
        emit finished(this);
    }
    else if (m_request->status() == QYouTube::ResourcesRequest::Failed) {
        setErrorString(m_request->errorString());
        setStatus(Error);
        emit finished(this);
    }
}

void YouTubeFeedRequest::getChannelVideos(const QString &id) {
    QVariantMap params;
    params["channelId"] = id;
    params["maxResults"] = m_settings.value("maxResults", 20);
    params["order"] = "date";
    params["safeSearch"] = m_settings.value("safeSearch", "none");
    params["type"] = "video";
    request(SLOT(checkVideos()))->list("/search", QStringList() << "snippet", QVariantMap(), params);
}

void YouTubeFeedRequest::searchVideos() {
    QVariantMap params;
    params["maxResults"] = m_settings.value("maxResults", 20);
    params["order"] = "date";
    params["q"] = m_settings.value("query");
    params["safeSearch"] = m_settings.value("safeSearch", "none");
    params["type"] = "video";
    request(SLOT(checkVideos()))->list("/search", QStringList() << "snippet", QVariantMap(), params);
}

void YouTubeFeedRequest::checkVideos() {
    if (m_request->status() == QYouTube::ResourcesRequest::Ready) {
        QByteArray ba;
        QXmlStreamWriter writer(&ba);        
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("rss");
        writer.writeAttribute("version", "2.0");
        writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
        writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
        writer.writeStartElement("channel");
        writer.writeTextElement("title", m_settings.value("query").toString());
        writer.writeTextElement("description", tr("YouTube videos"));
        writer.writeTextElement("link", "https://www.youtube.com");
        writer.writeStartElement("image");
        writer.writeTextElement("url", m_settings.value("icon").toString());
        writer.writeEndElement(); // image
        
        const QVariantList items = m_request->result().toMap().value("items").toList();

        foreach (const QVariant &item, items) {
            const QVariantMap video = item.toMap();
            const QVariantMap snippet = video.value("snippet").toMap();
            const QString videoUrl = QString("https://www.youtube.com/watch?v=%1")
                                            .arg(video.value("id").toMap().value("videoId").toString());
            writer.writeStartElement("item");
            writer.writeTextElement("title", snippet.value("title").toString());
            writer.writeTextElement("link", videoUrl);
            writer.writeTextElement("dc:date", snippet.value("publishedAt").toString());
            writer.writeTextElement("dc:creator", snippet.value("channelTitle").toString());
            writer.writeStartElement("content:encoded");
            writer.writeCDATA(QString("<a href=\"%1\"><img src=\"%2\" width=\"480\" height=\"360\" /></a><p>%3")
                                     .arg(videoUrl).arg(snippet.value("thumbnails").toMap()
                                     .value("high").toMap().value("url").toString())
                                     .arg(snippet.value("description").toString()));
            writer.writeEndElement(); // content:encoded
            writer.writeTextElement("description", snippet.value("description").toString());
            writer.writeStartElement("enclosure");
            writer.writeAttribute("url", videoUrl);
            writer.writeAttribute("type", "video/youtube");
            writer.writeEndElement(); // enclosure

            foreach (const QVariant &tag, snippet.value("tags").toList()) {
                writer.writeTextElement("category", tag.toString());
            }
            
            writer.writeEndElement(); // item
        }
        
        writer.writeEndElement(); // channel
        writer.writeEndElement(); // rss
        writer.writeEndDocument();
        
        setResult(ba);
        setStatus(Ready);
        emit finished(this);
    }
    else if (m_request->status() == QYouTube::ResourcesRequest::Failed) {
        setErrorString(m_request->errorString());
        setStatus(Error);
        emit finished(this);
    }
}
