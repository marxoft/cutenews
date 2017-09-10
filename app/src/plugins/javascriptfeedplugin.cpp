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

#include "javascriptfeedplugin.h"
#include "javascriptarticlerequest.h"
#include "javascriptenclosurerequest.h"
#include "javascriptfeedrequest.h"

JavaScriptFeedPlugin::JavaScriptFeedPlugin(QObject *parent) :
    QObject(parent),
    FeedPlugin()
{
}

JavaScriptFeedPlugin::JavaScriptFeedPlugin(const QString &id, const QString &fileName, QObject *parent) :
    QObject(parent),
    FeedPlugin(),
    m_fileName(fileName),
    m_id(id)
{
}

QString JavaScriptFeedPlugin::fileName() const {
    return m_fileName;
}

void JavaScriptFeedPlugin::setFileName(const QString &fileName) {
    m_fileName = fileName;
}

QString JavaScriptFeedPlugin::id() const {
    return m_id;
}

void JavaScriptFeedPlugin::setId(const QString &id) {
    m_id = id;
}

ArticleRequest* JavaScriptFeedPlugin::articleRequest(QObject *parent) {
    return new JavaScriptArticleRequest(id(), fileName(), parent);
}

EnclosureRequest* JavaScriptFeedPlugin::enclosureRequest(QObject *parent) {
    return new JavaScriptEnclosureRequest(id(), fileName(), parent);
}

FeedRequest* JavaScriptFeedPlugin::feedRequest(QObject *parent) {
    return new JavaScriptFeedRequest(id(), fileName(), parent);
}
