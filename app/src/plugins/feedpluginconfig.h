/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FEEDPLUGINCONFIG_H
#define FEEDPLUGINCONFIG_H

#include <QObject>
#include <QRegExp>
#include <QVariantList>

class FeedPluginConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString displayName READ displayName NOTIFY changed)
    Q_PROPERTY(QString filePath READ filePath NOTIFY changed)
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString pluginFilePath READ pluginFilePath NOTIFY changed)
    Q_PROPERTY(QString pluginType READ pluginType NOTIFY changed)
    Q_PROPERTY(bool supportsArticles READ supportsArticles NOTIFY changed)
    Q_PROPERTY(QRegExp articleRegExp READ articleRegExp NOTIFY changed)
    Q_PROPERTY(QVariantList articleSettings READ articleSettings NOTIFY changed)
    Q_PROPERTY(bool supportsEnclosures READ supportsEnclosures NOTIFY changed)
    Q_PROPERTY(QRegExp enclosureRegExp READ enclosureRegExp NOTIFY changed)
    Q_PROPERTY(QVariantList enclosureSettings READ enclosureSettings NOTIFY changed)
    Q_PROPERTY(bool supportsFeeds READ supportsFeeds NOTIFY changed)
    Q_PROPERTY(QVariantList feedSettings READ feedSettings NOTIFY changed)
    Q_PROPERTY(int version READ version NOTIFY changed)

public:
    explicit FeedPluginConfig(QObject *parent = 0);

    QString displayName() const;
        
    QString filePath() const;
        
    QString id() const;

    QString pluginFilePath() const;
    
    QString pluginType() const;

    bool supportsArticles() const;
    QRegExp articleRegExp() const;
    QVariantList articleSettings() const;

    bool supportsEnclosures() const;
    QRegExp enclosureRegExp() const;
    QVariantList enclosureSettings() const;
    
    bool supportsFeeds() const;
    QVariantList feedSettings() const;
        
    int version() const;

public Q_SLOTS:    
    bool load(const QString &filePath);

    bool articleIsSupported(const QString &url) const;
    bool enclosureIsSupported(const QString &url) const;

Q_SIGNALS:
    void changed();

private:
    QString m_displayName;
    QString m_filePath;
    QString m_id;
    QString m_pluginFilePath;
    QString m_pluginType;
    
    QRegExp m_articleRegExp;
    QRegExp m_enclosureRegExp;
    
    QVariantList m_articleSettings;
    QVariantList m_enclosureSettings;
    QVariantList m_feedSettings;
    
    bool m_articles;
    bool m_enclosures;
    bool m_feeds;
    
    int m_version;    
};

#endif // FEEDPLUGINCONFIG_H
