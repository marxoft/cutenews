/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QDateTime>
#include <QXmlStreamReader>

class FeedParser : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString author READ author NOTIFY ready)
    Q_PROPERTY(QStringList categories READ categories NOTIFY ready)
    Q_PROPERTY(QDateTime date READ date NOTIFY ready)
    Q_PROPERTY(QString description READ description NOTIFY ready)
    Q_PROPERTY(QVariantList enclosures READ enclosures NOTIFY ready)
    Q_PROPERTY(FeedType feedType READ feedType)
    Q_PROPERTY(QString errorString READ errorString NOTIFY error)
    Q_PROPERTY(QString iconUrl READ iconUrl NOTIFY ready)
    Q_PROPERTY(QString title READ title NOTIFY ready)
    Q_PROPERTY(QString url READ url NOTIFY ready)
    
    Q_ENUMS(FeedType)

public:
    enum FeedType {
        RSS = 0,
        Atom
    };
    
    explicit FeedParser(QObject *parent = 0);
    explicit FeedParser(const QByteArray &content, QObject *parent = 0);
    explicit FeedParser(const QString &content, QObject *parent = 0);
    explicit FeedParser(QIODevice *device, QObject *parent = 0);
    
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
    
    Q_INVOKABLE bool setContent(const QByteArray &content);
    Q_INVOKABLE bool setContent(const QString &content);
    bool setContent(QIODevice *device);
    
public Q_SLOTS:
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

Q_SIGNALS:
    void error();
    void ready();

private:
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
