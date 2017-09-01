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

#ifndef FEEDPARSER_H
#define FEEDPARSER_H

#include <QVariantList>
#include <QStringList>
#include <QDateTime>
#include <QXmlStreamReader>

class FeedParser
{

public:
    enum FeedType {
        RSS = 0,
        Atom
    };

    explicit FeedParser();
    explicit FeedParser(const QByteArray &content);
    explicit FeedParser(const QString &content);
    explicit FeedParser(QIODevice *device);
    virtual ~FeedParser();
    
    QString author() const;
    
    QStringList categories() const;
    
    QDateTime date() const;
    
    QString description() const;
    
    QVariantList enclosures() const;
    
    QString errorString() const;
    
    FeedType feedType() const;
    
    QString iconUrl() const;
            
    QString title() const;
    
    QString url() const;
    
    bool setContent(const QByteArray &content);
    bool setContent(const QString &content);
    bool setContent(QIODevice *device);
    
    void clear();
    
    bool readChannel();
    bool readNextArticle();

private:
    void setAuthor(const QString &a);
    
    void setCategories(const QStringList &c);
    
    void setDate(const QDateTime &d);
    
    void setDescription(const QString &d);
    
    void setEnclosures(const QVariantList &e);
    
    void setErrorString(const QString &e);
    
    void setFeedType(FeedType t);
    
    void setIconUrl(const QString &i);
    
    void setTitle(const QString &t);
    
    void setUrl(const QString &u);
    
    void readAuthor();
    void readCategories();
    void readDate();
    void readDescription();
    void readEnclosures();
    void readIconUrl();
    void readTitle();
    void readUrl();

    QXmlStreamReader m_reader;
    
    QString m_author;
    
    QStringList m_categories;
    
    QDateTime m_date;
    
    QString m_description;
    
    QVariantList m_enclosures;
    
    QString m_errorString;
    
    FeedType m_feedType;
    
    QString m_iconUrl;
        
    QString m_title;
    
    QString m_url;
};

#endif // FEEDPARSER_H
