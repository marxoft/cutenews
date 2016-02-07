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

#include "opmlparser.h"
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

OpmlParser::OpmlParser(QObject *parent) :
    QObject(parent)
{
}

OpmlParser::OpmlParser(const QByteArray &content, QObject *parent) :
    QObject(parent)
{
    setContent(content);
}

OpmlParser::OpmlParser(const QString &content, QObject *parent) :
    QObject(parent)
{
    setContent(content);
}

OpmlParser::OpmlParser(QIODevice *device, QObject *parent) :
    QObject(parent)
{
    setContent(device);
}

QString OpmlParser::description() const {
    return m_description;
}

void OpmlParser::setDescription(const QString &d) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setDescription" << d;
#endif
    m_description = d;
}

QString OpmlParser::errorString() const {
    return m_errorString;
}

void OpmlParser::setErrorString(const QString &e) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setErrorString" << e;
#endif
    m_errorString = e;
}

OpmlParser::FeedType OpmlParser::feedType() const {
    return m_feedType;
}

void OpmlParser::setFeedType(OpmlParser::FeedType t) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setFeedType" << t;
#endif
    m_feedType = t;
}

QString OpmlParser::htmlUrl() const {
    return m_htmlUrl;
}

void OpmlParser::setHtmlUrl(const QString &u) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setHtmlUrl" << u;
#endif
    m_htmlUrl = u;
}

QString OpmlParser::text() const {
    return m_text;
}

void OpmlParser::setText(const QString &t) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setText" << t;
#endif
    m_text = t;
}

QString OpmlParser::title() const {
    return m_title;
}

void OpmlParser::setTitle(const QString &t) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setTitle" << t;
#endif
    m_title = t;
}

QString OpmlParser::xmlUrl() const {
    return m_xmlUrl;
}

void OpmlParser::setXmlUrl(const QString &u) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::setXmlUrl" << u;
#endif
    m_xmlUrl = u;
}

bool OpmlParser::setContent(const QByteArray &content) {
    m_reader.clear();
    m_reader.addData(content);
    
    if (m_reader.hasError()) {
        setErrorString(tr("Unable to parse XML"));
        emit error();
        return false;
    }
    
    return true;
}

bool OpmlParser::setContent(const QString &content) {
    m_reader.clear();
    m_reader.addData(content);
    
    if (m_reader.hasError()) {
        setErrorString(tr("Unable to parse XML"));
        emit error();
        return false;
    }
    
    return true;
}

bool OpmlParser::setContent(QIODevice *device) {    
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

void OpmlParser::clear() {    
    setDescription(QString());
    setFeedType(RSS);
    setHtmlUrl(QString());
    setText(QString());
    setTitle(QString());
    setXmlUrl(QString());
}

bool OpmlParser::readHead() {
    clear();
    m_reader.readNextStartElement();
    
    while ((!m_reader.atEnd()) && (!m_reader.hasError())) {
        const QStringRef name = m_reader.name();
#ifdef CUTENEWS_DEBUG
        qDebug() << "OpmlParser::readHead:" << name;
#endif
        if (name == "title") {
            setTitle(m_reader.readElementText());
        }
        else if (name == "body") {
            emit ready();
            return true;
        }
        
        m_reader.readNextStartElement();
    }
    
    return false;
    
    if ((m_reader.hasError()) && (!m_reader.atEnd())) {
        setErrorString(tr("Error parsing tag %1").arg(m_reader.name().toString()));
        emit error();
    }
}

bool OpmlParser::readNextSubscription() {
    clear();
    m_reader.readNextStartElement();
    
    const QStringRef name = m_reader.name();
#ifdef CUTENEWS_DEBUG
    qDebug() << "OpmlParser::readNextSubscription:" << name;
#endif
    if (name == "outline") {
        const QXmlStreamAttributes attributes = m_reader.attributes();
        setDescription(attributes.value("description").toString());
        setFeedType(attributes.value("type") == "atom" ? Atom : RSS);
        setHtmlUrl(attributes.value("htmlUrl").toString());
        setText(attributes.value("text").toString());
        setTitle(attributes.value("title").toString());
        setXmlUrl(attributes.hasAttribute("xmlUrl") ? attributes.value("xmlUrl").toString()
                                                    : attributes.value("url").toString());
        m_reader.readNextStartElement();
        emit ready();
        return true;
    }
    
    if ((m_reader.hasError()) && (!m_reader.atEnd())) {
        setErrorString(tr("Error parsing tag %1").arg(name.toString()));
        emit error();
    }
    
    return false;
}
