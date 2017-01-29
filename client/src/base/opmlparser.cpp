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

OpmlParser::OpmlParser() :
    m_feedType(RSS)
{
}

OpmlParser::OpmlParser(const QByteArray &content) :
    m_feedType(RSS)
{
    setContent(content);
}

OpmlParser::OpmlParser(const QString &content) :
    m_feedType(RSS)
{
    setContent(content);
}

OpmlParser::OpmlParser(QIODevice *device) :
    m_feedType(RSS)
{
    setContent(device);
}

OpmlParser::~OpmlParser() {}

QString OpmlParser::description() const {
    return m_description;
}

void OpmlParser::setDescription(const QString &d) {
    m_description = d;
}

QString OpmlParser::errorString() const {
    return m_errorString;
}

void OpmlParser::setErrorString(const QString &e) {
    m_errorString = e;
}

OpmlParser::FeedType OpmlParser::feedType() const {
    return m_feedType;
}

void OpmlParser::setFeedType(OpmlParser::FeedType t) {
    m_feedType = t;
}

QString OpmlParser::htmlUrl() const {
    return m_htmlUrl;
}

void OpmlParser::setHtmlUrl(const QString &u) {
    m_htmlUrl = u;
}

QString OpmlParser::text() const {
    return m_text;
}

void OpmlParser::setText(const QString &t) {
    m_text = t;
}

QString OpmlParser::title() const {
    return m_title;
}

void OpmlParser::setTitle(const QString &t) {
    m_title = t;
}

QString OpmlParser::xmlUrl() const {
    return m_xmlUrl;
}

void OpmlParser::setXmlUrl(const QString &u) {
    m_xmlUrl = u;
}

bool OpmlParser::setContent(const QByteArray &content) {
    m_reader.clear();
    m_reader.addData(content);
    
    if (m_reader.hasError()) {
        setErrorString(QString("Unable to parse XML"));
        return false;
    }
    
    return true;
}

bool OpmlParser::setContent(const QString &content) {
    m_reader.clear();
    m_reader.addData(content);
    
    if (m_reader.hasError()) {
        setErrorString(QString("Unable to parse XML"));
        return false;
    }
    
    return true;
}

bool OpmlParser::setContent(QIODevice *device) {    
    if ((!device) || ((!device->isOpen()) && (!device->open(QIODevice::ReadOnly)))) {
        setErrorString(QString("Unable to open IO device"));
        return false;
    }
    
    m_reader.clear();
    m_reader.setDevice(device);
    
    if (m_reader.hasError()) {
        setErrorString(QString("Unable to parse XML"));
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
        
        if (name == "title") {
            setTitle(m_reader.readElementText());
        }
        else if (name == "body") {
            return true;
        }
        
        m_reader.readNextStartElement();
    }
        
    if ((m_reader.hasError()) && (!m_reader.atEnd())) {
        setErrorString(QString("Error parsing tag %1").arg(m_reader.name().toString()));
    }

    return false;
}

bool OpmlParser::readNextSubscription() {
    clear();
    m_reader.readNextStartElement();
    
    const QStringRef name = m_reader.name();
    
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
        return true;
    }
    
    if ((m_reader.hasError()) && (!m_reader.atEnd())) {
        setErrorString(QString("Error parsing tag %1").arg(name.toString()));
    }
    
    return false;
}
