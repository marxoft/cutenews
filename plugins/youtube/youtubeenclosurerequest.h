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

#ifndef YOUTUBEENCLOSUREREQUEST_H
#define YOUTUBEENCLOSUREREQUEST_H

#include "enclosurerequest.h"
#include <QStringList>

namespace QYouTube {
    class ResourcesRequest;
    class StreamsRequest;
}

class YouTubeEnclosureRequest : public EnclosureRequest
{
    Q_OBJECT

public:
    explicit YouTubeEnclosureRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual EnclosureResult result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void checkVideo();
    void checkStreams();

private:
    void setErrorString(const QString &e);

    void setResult(const EnclosureResult &r);

    void setStatus(Status s);
    
    QYouTube::ResourcesRequest* videoRequest();
    QYouTube::StreamsRequest* streamsRequest();

    void getStreams();
    void getVideo();

    static const QString API_KEY;
    static const QStringList VIDEO_FORMATS;

    QYouTube::ResourcesRequest *m_videoRequest;
    QYouTube::StreamsRequest *m_streamsRequest;

    QString m_errorString;

    EnclosureResult m_result;

    Status m_status;
    
    QVariantMap m_settings;
};

#endif // YOUTUBEENCLOSUREREQUEST_H
