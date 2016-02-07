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

#include "feedparser.h"
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

FeedParser::FeedParser(QObject *parent) :
    QObject(parent),
    m_feedType(RSS)
{
}

FeedParser::FeedParser(const QByteArray &content, QObject *parent) :
    QObject(parent),
    m_feedType(RSS)
{
    setContent(content);
}

FeedParser::FeedParser(const QString &content, QObject *parent) :
    QObject(parent),
    m_feedType(RSS)
{
    setContent(content);
}

FeedParser::FeedParser(QIODevice *device, QObject *parent) :
    QObject(parent),
    m_feedType(RSS)
{
    setContent(device);
}

QString FeedParser::author() const {
    return m_author;
}

void FeedParser::setAuthor(const QString &a) {
    m_author = a;
}

QStringList FeedParser::categories() const {
    return m_categories;
}

void FeedParser::setCategories(const QStringList &c) {
    m_categories = c;
}

QDateTime FeedParser::date() const {
    return m_date;
}

void FeedParser::setDate(const QDateTime &d) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setDate" << d;
#endif
    m_date = d;
}

QString FeedParser::description() const {
    return m_description;
}

void FeedParser::setDescription(const QString &d) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setDescription" << d;
#endif
    m_description = d;
}

QVariantList FeedParser::enclosures() const {
    return m_enclosures;
}

void FeedParser::setEnclosures(const QVariantList &e) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setEnclosures" << e;
#endif
    m_enclosures = e;
}

QString FeedParser::errorString() const {
    return m_errorString;
}

void FeedParser::setErrorString(const QString &e) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setErrorString" << e;
#endif
    m_errorString = e;
}

FeedParser::FeedType FeedParser::feedType() const {
    return m_feedType;
}

void FeedParser::setFeedType(FeedParser::FeedType t) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setFeedType" << t;
#endif
    m_feedType = t;
}

QString FeedParser::iconUrl() const {
    return m_iconUrl;
}

void FeedParser::setIconUrl(const QString &i) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setIconUrl" << i;
#endif
    m_iconUrl = i;
}

QString FeedParser::title() const {
    return m_title;
}

void FeedParser::setTitle(const QString &t) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setTitle" << t;
#endif
    m_title = t;
}

QString FeedParser::url() const {
    return m_url;
}

void FeedParser::setUrl(const QString &u) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "FeedParser::setUrl" << u;
#endif
    m_url = u;
}

bool FeedParser::setContent(const QByteArray &content) {
    m_reader.clear();
    m_reader.addData(content);
    
    if (m_reader.hasError()) {
        setErrorString(tr("Unable to parse XML"));
        emit error();
        return false;
    }
    
    return true;
}

bool FeedParser::setContent(const QString &content) {
    m_reader.clear();
    m_reader.addData(content);
    
    if (m_reader.hasError()) {
        setErrorString(tr("Unable to parse XML"));
        emit error();
        return false;
    }
    
    return true;
}

bool FeedParser::setContent(QIODevice *device) {    
    if ((!device) || ((!device->isOpen()) && (!device->open(QIODevice::ReadOnly)))) {
        setErrorString(tr("Unable to open IO device"));
        emit error();
        return false;
    }
    
    m_reader.clear();
    m_reader.setDevice(device);
    
    if (m_reader.hasError()) {
        setErrorString(tr("Unable to parse XML"));
        emit error();
        return false;
    }
    
    return true;
}

void FeedParser::clear() {    
    setAuthor(QString());
    setCategories(QStringList());
    setDate(QDateTime());
    setDescription(QString());
    setEnclosures(QVariantList());
    setIconUrl(QString());
    setTitle(QString());
    setUrl(QString());
}

bool FeedParser::readChannel() {
    clear();
    m_reader.readNextStartElement();
    
    if (m_reader.qualifiedName() == "feed") {
        setFeedType(Atom);
    }
    else {
        setFeedType(RSS);
    }
    
    if (feedType() == Atom) {
        while ((!m_reader.atEnd()) && (!m_reader.hasError())) {        
            const QStringRef name = m_reader.qualifiedName();
#ifdef CUTENEWS_DEBUG
            qDebug() << "FeedParser::readChannel:" << name;
#endif
            if ((name == "published") || ((name == "updated") && (date().isNull()))) {
                readDate();
            }
            else if (name == "subtitle") {
                readDescription();
            }
            else if (name == "logo") {
                readIconUrl();
            }
            else if (name == "title") {
                readTitle();
            }
            else if (name == "link") {
                readUrl();
            }
            else if (name == "entry") {
                emit ready();
                return true;
            }
            else {
                m_reader.readNextStartElement();
            }
        }
    }
    else {
        while ((!m_reader.atEnd()) && (!m_reader.hasError())) {        
            const QStringRef name = m_reader.qualifiedName();
#ifdef CUTENEWS_DEBUG
            qDebug() << "FeedParser::readChannel:" << name;
#endif
            if (name == "lastBuildDate") {
                readDate();
            }
            else if (name == "description") {
                readDescription();
            }
            else if ((name == "image") || (name == "itunes:image")) {
                readIconUrl();
            }
            else if (name == "title") {
                readTitle();
            }
            else if (name == "link") {
                readUrl();
            }
            else if (name == "item") {
                emit ready();
                return true;
            }
            else {
                m_reader.readNextStartElement();
            }
        }
    }
    
    if ((m_reader.hasError()) && (!m_reader.atEnd())) {
        setErrorString(tr("Error parsing tag %1").arg(m_reader.qualifiedName().toString()));
        emit error();
    }
    
    return false;
}

bool FeedParser::readNextArticle() {
    clear();
    m_reader.readNextStartElement();
    
    if (feedType() == Atom) {
        while ((!m_reader.atEnd()) && (!m_reader.hasError())) {        
            const QStringRef name = m_reader.qualifiedName();
#ifdef CUTENEWS_DEBUG
            qDebug() << "FeedParser::readNextArticle:" << name;
#endif
            if (name == "author") {
                readAuthor();
            }
            else if (name == "category") {
                readCategories();
            }
            else if ((name == "published") || ((name == "updated") && (date().isNull()))) {
                readDate();
            }
            else if ((name == "content:encoded") || ((name == "summary") && (description().isEmpty()))) {
                readDescription();
            }
            else if (name == "media:content") {
                readEnclosures();
            }
            else if (name == "title") {
                readTitle();
            }
            else if (name == "link") {
                if (m_reader.attributes().value("rel") == "enclosure") {
                    readEnclosures();
                }
                else {
                    readUrl();
                }
            }
            else if (name == "entry") {
                m_reader.readNextStartElement();
                emit ready();
                return true;
            }
            else {
                m_reader.readNextStartElement();
            }      
        }
    }
    else {
        while ((!m_reader.atEnd()) && (!m_reader.hasError())) {        
            const QStringRef name = m_reader.qualifiedName();
#ifdef CUTENEWS_DEBUG
            qDebug() << "FeedParser::readNextArticle:" << name;
#endif
            if ((name == "dc:creator") || (name == "itunes:author")) {
                readAuthor();
            }
            else if ((name == "category") || (name == "itunes:keywords")) {
                readCategories();
            }
            else if ((name == "pubDate") || (name == "dc:date")) {
                readDate();
            }
            else if ((name == "content:encoded") || ((name == "description") && (description().isEmpty()))) {
                readDescription();
            }
            else if ((name == "enclosure") || (name == "media:content")) {
                readEnclosures();
            }
            else if (name == "title") {
                readTitle();
            }
            else if (name == "link") {
                readUrl();
            }
            else if (name == "item") {
                m_reader.readNextStartElement();
                emit ready();
                return true;
            }
            else {
                m_reader.readNextStartElement();
            }      
        }
    }
    
    if ((m_reader.hasError()) && (!m_reader.atEnd())) {
        setErrorString(tr("Error parsing tag %1").arg(m_reader.qualifiedName().toString()));
        emit error();
    }
    
    return false;
}

void FeedParser::readAuthor() {
    if (feedType() == Atom) {
        m_reader.readNextStartElement();
        
        while (m_reader.qualifiedName() != "author") {
            if (m_reader.qualifiedName() == "name") {
                setAuthor(m_reader.readElementText());
            }
            
            m_reader.readNextStartElement();
        }
    }
    else {
        setAuthor(m_reader.readElementText());
    }
    
    m_reader.readNextStartElement();
}

void FeedParser::readCategories() {    
    if (m_reader.qualifiedName() == "itunes:keywords") {
        setCategories(m_reader.readElementText().replace(", ", ",").split(","));
        m_reader.readNextStartElement();
    }
    else {
        QStringList c;
        
        while (m_reader.qualifiedName() == "category") {
            c << m_reader.readElementText();
            m_reader.readNextStartElement();
        }
        
        setCategories(c);
    }    
}

void FeedParser::readDate() {
    if (m_reader.qualifiedName() == "pubDate") {
        setDate(QDateTime::fromString(m_reader.readElementText().section(' ', 0, -2),
                                              "ddd, dd MMM yyyy HH:mm:ss").toLocalTime());
    }
    else {
        setDate(QDateTime::fromString(m_reader.readElementText(), Qt::ISODate).toLocalTime());
    }
    
    m_reader.readNextStartElement();
}

void FeedParser::readDescription() {
    setDescription(m_reader.readElementText());
    m_reader.readNextStartElement();
}

void FeedParser::readEnclosures() {
    QVariantList el;
    const QStringRef enclosureName = m_reader.qualifiedName();
    
    while (m_reader.qualifiedName() == enclosureName) {
        const QXmlStreamAttributes attributes = m_reader.attributes();
        QVariantMap e;
        e["length"] = attributes.hasAttribute("length") ? attributes.value("length").toString().toInt()
                                                        : attributes.value("fileSize").toString().toInt();
        e["url"] = attributes.value("url").toString();
        e["type"] = attributes.value("type").toString();
        el << e;
        m_reader.readNextStartElement();
        m_reader.readNextStartElement();
    }
    
    setEnclosures(el);
}

void FeedParser::readIconUrl() {
    const QXmlStreamAttributes attributes = m_reader.attributes();
    
    if (attributes.hasAttribute("href")) {
        setIconUrl(attributes.value("href").toString());
    }
    else if (attributes.hasAttribute("src")) {
        setIconUrl(attributes.value("src").toString());
    }
    else if (m_reader.qualifiedName() == "image") {
        m_reader.readNextStartElement();
        
        while (m_reader.name() != "image") {
            if (m_reader.qualifiedName() == "url") {
                setIconUrl(m_reader.readElementText());
            }
            
            m_reader.readNextStartElement();
        }
    }
    
    m_reader.readNextStartElement();
}   

void FeedParser::readTitle() {
    setTitle(m_reader.readElementText());
    m_reader.readNextStartElement();
}

void FeedParser::readUrl() {
    const QXmlStreamAttributes attributes = m_reader.attributes();
    
    if (attributes.hasAttribute("href")) {
        setUrl(attributes.value("href").toString());
        m_reader.readNextStartElement();
    }
    else {
        setUrl(m_reader.readElementText());
    }
    
    m_reader.readNextStartElement();
}
