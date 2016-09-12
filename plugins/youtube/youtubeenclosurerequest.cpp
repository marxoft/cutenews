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

#include "youtubeenclosurerequest.h"
#include <qyoutube/resourcesrequest.h>
#include <qyoutube/streamsrequest.h>
#include <QRegExp>

const QString YouTubeEnclosureRequest::API_KEY("AIzaSyDhIlkLzHJKDCNr6thsjlQpZrkY3lO_Uu4");
const QStringList YouTubeEnclosureRequest::VIDEO_FORMATS = QStringList() << "37" << "46" << "22" << "45" << "44"
                                                                         << "35" << "18" << "43" << "34" << "36"
                                                                         << "17";

YouTubeEnclosureRequest::YouTubeEnclosureRequest(QObject *parent) :
    EnclosureRequest(parent),
    m_videoRequest(0),
    m_streamsRequest(0),
    m_status(Idle)
{
}

QString YouTubeEnclosureRequest::errorString() const {
    return m_errorString;
}

void YouTubeEnclosureRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

Enclosure YouTubeEnclosureRequest::result() const {
    return m_result;
}

void YouTubeEnclosureRequest::setResult(const Enclosure &r) {
    m_result = r;
}

YouTubeEnclosureRequest::Status YouTubeEnclosureRequest::status() const {
    return m_status;
}

void YouTubeEnclosureRequest::setStatus(YouTubeEnclosureRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool YouTubeEnclosureRequest::cancel() {
    if (status() == Active) {
        if (m_videoRequest) {
            m_videoRequest->cancel();
        }
        
        if (m_streamsRequest) {
            m_streamsRequest->cancel();
        }

        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool YouTubeEnclosureRequest::getEnclosure(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    const QString videoId = url.section(QRegExp("v=|list=|/"), -1).section(QRegExp("&|\\?"), 0, 0);
    
    
    if (videoId.isEmpty()) {
        setErrorString(tr("Cannot extract video ID from URL"));
        setResult(Enclosure());
        setStatus(Error);
        emit finished(this);
        return false;
    }

    m_settings = settings;
    m_settings["videoId"] = videoId;
    setStatus(Active);
    getStreams();
    return true;
}

QYouTube::ResourcesRequest* YouTubeEnclosureRequest::videoRequest() {
    if (!m_videoRequest) {
        m_videoRequest = new QYouTube::ResourcesRequest(this);
        m_videoRequest->setApiKey(API_KEY);
        connect(m_videoRequest, SIGNAL(finished()), this, SLOT(checkVideo()));
    }

    return m_videoRequest;
}

QYouTube::StreamsRequest* YouTubeEnclosureRequest::streamsRequest() {
    if (!m_streamsRequest) {
        m_streamsRequest = new QYouTube::StreamsRequest(this);
        connect(m_streamsRequest, SIGNAL(finished()), this, SLOT(checkStreams()));
    }

    return m_streamsRequest;
}

void YouTubeEnclosureRequest::getStreams() {
    streamsRequest()->list(m_settings.value("videoId").toString());
}
           
void YouTubeEnclosureRequest::checkStreams() {
    if (m_streamsRequest->status() == QYouTube::StreamsRequest::Ready) {
        const QVariantList items = m_streamsRequest->result().toList();
        const int start = qMax(0, VIDEO_FORMATS.indexOf(m_settings.value("videoFormat", "22").toString()));
        
        for (int i = start; i < VIDEO_FORMATS.size(); i++) {
            foreach (const QVariant &item, items) {
                const QVariantMap stream = item.toMap();

                if (stream.value("id") == VIDEO_FORMATS.at(i)) {
                    m_result.request = QNetworkRequest(stream.value("url").toString());
                    getVideo();
                    return;
                }
            }
        }

        setErrorString(tr("No video streams found"));
        setResult(Enclosure());
        setStatus(Error);
        emit finished(this);
    }
    else if (m_streamsRequest->status() == QYouTube::StreamsRequest::Failed) {
        setErrorString(m_streamsRequest->errorString());
        setResult(Enclosure());
        setStatus(Error);
        emit finished(this);
    }
}

void YouTubeEnclosureRequest::getVideo() {
    QVariantMap filter;
    filter["id"] = m_settings.value("videoId");
    videoRequest()->list("/videos", QStringList() << "snippet", filter);
}

void YouTubeEnclosureRequest::checkVideo() {
    if (m_videoRequest->status() == QYouTube::ResourcesRequest::Ready) {
        const QVariantList items = m_videoRequest->result().toMap().value("items").toList();

        if (!items.isEmpty()) {
            m_result.fileName = items.first().toMap().value("snippet").toMap().value("title").toString() + ".mp4";
            setErrorString(QString());
            setStatus(Ready);
        }
        else {
            setErrorString(tr("No video found"));
            setResult(Enclosure());
            setStatus(Error);
        }

        emit finished(this);
    }
    else if (m_videoRequest->status() == QYouTube::ResourcesRequest::Failed) {
        setErrorString(m_videoRequest->errorString());
        setResult(Enclosure());
        setStatus(Error);
        emit finished(this);
    }
}
