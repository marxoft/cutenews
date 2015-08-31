/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "youtube.h"
#include <qyoutube/resourcesrequest.h>
#include <qyoutube/streamsrequest.h>
#include <QXmlStreamWriter>
#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <iostream>

static const QString API_KEY("AIzaSyDhIlkLzHJKDCNr6thsjlQpZrkY3lO_Uu4");
static const QStringList VIDEO_FORMATS = QStringList() << "37" << "22" << "18" << "5" << "17";

YouTube::YouTube(QObject *parent) :
    QObject(parent),
    m_resourcesRequest(0),
    m_streamsRequest(0),
    m_format("18"),
    m_iconUrl("http://www.google.com/s2/favicons?domain=youtube.com"),
    m_safeSearch("none"),
    m_maxResults(20)
{
}

void YouTube::getVideoStreamUrl(const QString &url, const QString &format) {
    if (!m_streamsRequest) {
        m_streamsRequest = new QYouTube::StreamsRequest(this);
    }
    
    m_format = format;
    m_videoId = url.section(QRegExp("v=|list=|/"), -1).section(QRegExp("&|\\?"), 0, 0);
    
    if (m_videoId.isEmpty()) {
        std::cout << QString("{\"error\": \"%1\"}").arg(tr("Cannot extract video id from url")).toUtf8().constData();
        QCoreApplication::exit(1);
    }
    else {
        disconnect(m_resourcesRequest, 0, this, 0);
        connect(m_streamsRequest, SIGNAL(finished()), this, SLOT(onStreamsRequestFinished()));
        m_streamsRequest->list(m_videoId);
    }
}

void YouTube::getChannelVideos(const QString &channel, const QString &safeSearch, int maxResults) {
    if (!m_resourcesRequest) {
        m_resourcesRequest = new QYouTube::ResourcesRequest(this);
        m_resourcesRequest->setApiKey(API_KEY);
    }
    
    m_query = channel;
    m_safeSearch = safeSearch;
    m_maxResults = maxResults;
        
    QVariantMap filter;
    filter["forUsername"] = channel;
    
    disconnect(m_resourcesRequest, 0, this, 0);
    connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onChannelIdRequestFinished()));
    m_resourcesRequest->list("/channels", QStringList() << "id" << "snippet", filter);
}

void YouTube::searchVideos(const QString &query, const QString &safeSearch, int maxResults) {
    if (!m_resourcesRequest) {
        m_resourcesRequest = new QYouTube::ResourcesRequest(this);
        m_resourcesRequest->setApiKey(API_KEY);
    }
    
    m_query = query;
    m_iconUrl = QString("http://www.google.com/s2/favicons?domain=youtube.com");
    m_safeSearch = safeSearch;
    m_maxResults = maxResults;
    
    QVariantMap params;
    params["q"] = query;
    params["safeSearch"] = safeSearch;
    params["type"] = "video";
    params["maxResults"] = maxResults;
    params["order"] = "date";
    
    disconnect(m_resourcesRequest, 0, this, 0);
    connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onVideosRequestFinished()));
    m_resourcesRequest->list("/search", QStringList() << "snippet", QVariantMap(), params);
}

void YouTube::onChannelIdRequestFinished() {
    if (m_resourcesRequest->status() == QYouTube::ResourcesRequest::Ready) {
        const QVariantList items = m_resourcesRequest->result().toMap().value("items").toList();
        
        if (!items.isEmpty()) {
            const QVariantMap channel = items.first().toMap();
            const QString channelId = channel.value("id").toString();
        
            if (!channelId.isEmpty()) {
                const QVariantMap snippet = channel.value("snippet").toMap();
                m_query = snippet.value("title").toString();
                m_iconUrl = snippet.value("thumbnails").toMap().value("default").toMap().value("url").toString();
                
                QVariantMap params;
                params["channelId"] = channelId;
                params["safeSearch"] = m_safeSearch;
                params["type"] = "video";
                params["maxResults"] = m_maxResults;
                params["order"] = "date";
    
                disconnect(m_resourcesRequest, 0, this, 0);
                connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onVideosRequestFinished()));
                m_resourcesRequest->list("/search", QStringList() << "snippet", QVariantMap(), params);
                return;
            }
        }
        
        std::cout << QString("{\"error\": \"%1\"}").arg(tr("Channel id not found")).toUtf8().constData();
        QCoreApplication::exit(1);
    }
    else {
        std::cout << QString("{\"error\": \"%1\"}").arg(m_resourcesRequest->errorString()).toUtf8().constData();
        QCoreApplication::exit(1);
    }
}

void YouTube::onStreamsRequestFinished() {
    if (m_streamsRequest->status() == QYouTube::StreamsRequest::Ready) {
        const QVariantList items = m_streamsRequest->result().toList();
        const int min = qMax(0, VIDEO_FORMATS.indexOf(m_format));
        const int max = VIDEO_FORMATS.size();
        
        foreach (QVariant item, items) {
            const QVariantMap stream = item.toMap();
            
            for (int i = min; i < max; i++) {
                if (stream.value("id") == VIDEO_FORMATS.at(i)) {
                    m_videoUrl = stream.value("url").toString();
                    
                    if (!m_videoUrl.isEmpty()) {
                        if (!m_resourcesRequest) {
                            m_resourcesRequest = new QYouTube::ResourcesRequest(this);
                            m_resourcesRequest->setApiKey(API_KEY);
                        }
                        
                        QVariantMap filter;
                        filter["id"] = m_videoId;
    
                        disconnect(m_resourcesRequest, 0, this, 0);
                        connect(m_resourcesRequest, SIGNAL(finished()), this, SLOT(onVideoTitleRequestFinished()));
                        m_resourcesRequest->list("/videos", QStringList() << "snippet", filter);
                        return;
                    }
                }
            }
        }
        
        std::cout << QString("{\"error\": \"%1\"}").arg(tr("No video url found")).toUtf8().constData();
        QCoreApplication::exit(1);
    }
    else {
        std::cout << QString("{\"error\": \"%1\"}").arg(m_streamsRequest->errorString()).toUtf8().constData();
        QCoreApplication::exit(1);
    }    
}

void YouTube::onVideoTitleRequestFinished() {
    if (m_resourcesRequest->status() == QYouTube::ResourcesRequest::Ready) {
        const QVariantList items = m_resourcesRequest->result().toMap().value("items").toList();
        
        if (!items.isEmpty()) {
            m_videoTitle = items.first().toMap().value("snippet").toMap().value("title").toString();
        }
    }
    
    std::cout << QString("{\"url\": \"%1\", \"fileName\": \"%2.mp4\"}").arg(m_videoUrl)
                        .arg(m_videoTitle.isEmpty() ? tr("Unknown title") : m_videoTitle).toUtf8().constData();
    QCoreApplication::exit(0);
}       

void YouTube::onVideosRequestFinished() {
    if (m_resourcesRequest->status() == QYouTube::ResourcesRequest::Ready) {
        const QVariantMap result = m_resourcesRequest->result().toMap();
        
        if (result.isEmpty()) {
            std::cout << QString("{\"error\": \"%1\"}").arg(tr("No server response")).toUtf8().constData();
            QCoreApplication::exit(1);
            return;
        }
        
        QByteArray ba;
        QXmlStreamWriter writer(&ba);        
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("rss");
        writer.writeAttribute("version", "2.0");
        writer.writeAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
        writer.writeAttribute("xmlns:content", "http://purl.org/rss/1.0/modules/content/");
        writer.writeStartElement("channel");
        writer.writeTextElement("title", m_query);
        writer.writeTextElement("description", QString("YouTube videos - %1").arg(m_query));
        writer.writeTextElement("link", "https://www.youtube.com");
        writer.writeStartElement("image");
        writer.writeTextElement("url", m_iconUrl);
        writer.writeEndElement(); // image
        
        const QVariantList items = result.value("items").toList();
        
        foreach (QVariant item, items) {
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
            writer.writeEndElement(); // item
        }
        
        writer.writeEndElement(); // channel
        writer.writeEndElement(); // rss
        writer.writeEndDocument();
        
        std::cout << ba.constData();
        QCoreApplication::exit(0);
    }
    else {
        std::cout << QString("{\"error\": \"%1\"}").arg(m_resourcesRequest->errorString()).toUtf8().constData();
        QCoreApplication::exit(1);
    }    
}
