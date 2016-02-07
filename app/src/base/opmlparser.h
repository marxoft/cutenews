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

#include <QObject>
#include <QXmlStreamReader>

class OpmlParser : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString description READ description NOTIFY ready)
    Q_PROPERTY(FeedType feedType READ feedType NOTIFY ready)
    Q_PROPERTY(QString errorString READ errorString NOTIFY error)
    Q_PROPERTY(QString htmlUrl READ htmlUrl NOTIFY ready)
    Q_PROPERTY(QString text READ text NOTIFY ready)
    Q_PROPERTY(QString title READ title NOTIFY ready)
    Q_PROPERTY(QString xmlUrl READ xmlUrl NOTIFY ready)
    
public:
    enum FeedType {
        RSS = 0,
        Atom
    };
    
    explicit OpmlParser(QObject *parent = 0);
    explicit OpmlParser(const QByteArray &content, QObject *parent = 0);
    explicit OpmlParser(const QString &content, QObject *parent = 0);
    explicit OpmlParser(QIODevice *device, QObject *parent = 0);
        
    QString description() const;
        
    QString errorString() const;
    
    FeedType feedType() const;
    
    QString htmlUrl() const;
    
    QString text() const;
            
    QString title() const;
    
    QString xmlUrl() const;
    
    Q_INVOKABLE bool setContent(const QByteArray &content);
    Q_INVOKABLE bool setContent(const QString &content);
    bool setContent(QIODevice *device);
    
public Q_SLOTS:
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

Q_SIGNALS:
    void error();
    void ready();

private:
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
