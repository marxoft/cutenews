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

#ifndef YOUTUBEFEEDREQUEST_H
#define YOUTUBEFEEDREQUEST_H

#include "feedrequest.h"
#include <QVariantMap>

namespace QYouTube {
    class ResourcesRequest;
}

class YouTubeFeedRequest : public FeedRequest
{
    Q_OBJECT

public:
    explicit YouTubeFeedRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual QByteArray result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getFeed(const QVariantMap &settings);

private Q_SLOTS:
    void checkChannels();
    void checkVideos();

private:
    void setErrorString(const QString &e);

    void setResult(const QByteArray &r);

    void setStatus(Status s);
    
    QYouTube::ResourcesRequest* request(const char *slot);

    void getChannels();
    void getChannelVideos(const QString &id);
    void searchVideos();

    static const QString API_KEY;
    static const QString DEFAULT_ICON;
    static const QString CONFIG_FILE;

    QYouTube::ResourcesRequest *m_request;

    QString m_errorString;

    QByteArray m_result;

    Status m_status;

    QVariantMap m_settings;
};

#endif // YOUTUBEFEEDREQUEST_H
