/*!
 * \file feedplugin.h
 *
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

#ifndef FEEDPLUGIN_H
#define FEEDPLUGIN_H

#include "articlerequest.h"
#include "enclosurerequest.h"
#include "feedrequest.h"

/*!
 * Base class for cuteNews feed plugins.
 * 
 * The FeedPlugin class provides an interface which all cuteNews plugins must implement. 
 * The interface consists of three methods, articleRequest(), enclosureRequest() and feedRequest().
 *
 * \sa ArticleRequest, EnclosureRequest, FeedRequest
 */
class FeedPlugin
{

public:
    virtual ~FeedPlugin() {}

    /*!
     * This method can be re-implemented to return a valid ArticleRequest with the parent set to \a parent.
     *
     * By default, this method returns \c 0.
     *
     * \sa ArticleRequest
     */
    virtual ArticleRequest* articleRequest(QObject *parent = 0) { Q_UNUSED(parent); return 0; }

    /*!
     * This method can be re-implemented to return a valid EnclosureRequest with the parent set to \a parent.
     *
     * By default, this method returns \c 0.
     *
     * \sa EnclosureRequest
     */
    virtual EnclosureRequest* enclosureRequest(QObject *parent = 0) { Q_UNUSED(parent); return 0; }

    /*!
     * This method can be re-implemented to return a valid FeedRequest with the parent set to \a parent.
     *
     * By default, this method returns \c 0.
     *
     * \sa FeedRequest
     */
    virtual FeedRequest* feedRequest(QObject *parent = 0) { Q_UNUSED(parent); return 0; }
};

Q_DECLARE_INTERFACE(FeedPlugin, "org.cutenews.FeedPlugin")

#endif // FEEDPLUGIN_H
