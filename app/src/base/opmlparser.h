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

#ifndef OPMLPARSER_H
#define OPMLPARSER_H

#include <QXmlStreamReader>

class OpmlParser
{
    
public:
    enum FeedType {
        RSS = 0,
        Atom
    };
    
    explicit OpmlParser();
    explicit OpmlParser(const QByteArray &content);
    explicit OpmlParser(const QString &content);
    explicit OpmlParser(QIODevice *device);
    virtual ~OpmlParser();
        
    QString description() const;
        
    QString errorString() const;
    
    FeedType feedType() const;
    
    QString htmlUrl() const;
    
    QString text() const;
            
    QString title() const;
    
    QString xmlUrl() const;
    
    bool setContent(const QByteArray &content);
    bool setContent(const QString &content);
    bool setContent(QIODevice *device);
    
    void clear();
    
    bool readHead();
    bool readNextSubscription();

private:    
    void setDescription(const QString &d);
        
    void setErrorString(const QString &e);
    
    void setFeedType(FeedType t);
    
    void setHtmlUrl(const QString &u);
    
    void setText(const QString &t);
    
    void setTitle(const QString &t);
    
    void setXmlUrl(const QString &u);

    QXmlStreamReader m_reader;
        
    QString m_description;
        
    QString m_errorString;
    
    FeedType m_feedType;
    
    QString m_htmlUrl;
    
    QString m_text;
        
    QString m_title;
    
    QString m_xmlUrl;
};

#endif // OPMLPARSER_H
