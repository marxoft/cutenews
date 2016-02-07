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

#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <QObject>

namespace QYouTube {
    class ResourcesRequest;
    class StreamsRequest;
};

class YouTube : public QObject
{
    Q_OBJECT
    
public:
    explicit YouTube(QObject *parent = 0);
    
    void getVideoStreamUrl(const QString &url, const QString &format);
    
    void getChannelVideos(const QString &channel, const QString &safeSearch, int maxResults);
    
    void searchVideos(const QString &query, const QString &safeSearch, int maxResults);
    
private Q_SLOTS:
    void onChannelIdRequestFinished();
    void onStreamsRequestFinished();
    void onVideoTitleRequestFinished();
    void onVideosRequestFinished();

private:
    QYouTube::ResourcesRequest *m_resourcesRequest;
    QYouTube::StreamsRequest *m_streamsRequest;
    
    QString m_format;
    QString m_iconUrl;
    QString m_query;
    QString m_safeSearch;
    QString m_videoId;
    QString m_videoTitle;
    QString m_videoUrl;
    
    int m_maxResults;
};

#endif // YOUTUBE_H
