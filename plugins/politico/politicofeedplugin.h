/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef POLITICOFEEDPLUGIN_H
#define POLITICOFEEDPLUGIN_H

#include "feedplugin.h"
#include "politicoarticlerequest.h"
#include "politicofeedrequest.h"
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

class PoliticoFeedPlugin : public QObject, public FeedPlugin
{
    Q_OBJECT
    Q_INTERFACES(FeedPlugin)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.cutenews.PoliticoFeedPlugin")
#endif

public:
    virtual ArticleRequest* articleRequest(QObject *parent = 0) { return new PoliticoArticleRequest(parent); }
    virtual FeedRequest* feedRequest(QObject *parent = 0) { return new PoliticoFeedRequest(parent); }
};

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(cutenews-politico, PoliticoFeedPlugin)
#endif

#endif // POLITICOFEEDPLUGIN_H
