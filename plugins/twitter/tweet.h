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

#ifndef TWEET_H
#define TWEET_H

#include <QDateTime>
#include <QString>

static const QString TWEET_STYLESHEET(".cutenews-tweet { clear: both; padding-bottom: 16px; } .cutenews-tweet-text { clear: both; margin-top: 8px; }");

static const QString TWEET_STYLESHEET_INCLUDE_IMAGES(".cutenews-tweet { clear: both; padding-bottom: 16px; } .cutenews-tweet-avatar { width: 64px; height: 64px; float: left; } .cutenews-tweet-username { height: 64px; margin-left: 72px; } .cutenews-tweet-text { clear: both; margin-top: 8px; } .cutenews-tweet-media { clear: both; margin-top; 8px; } .Emoji { height: 16px }");

static const QString TWEET_HTML("<div class='cutenews-tweet'><div class='cutenews-tweet-username'>%1</div><div class='cutenews-tweet-text'>%2</div><div class='cutenews-tweet-text'>%3</div></div>");

static const QString TWEET_HTML_INCLUDE_IMAGES("<div class='cutenews-tweet'><img class='cutenews-tweet-avatar' src='%1'><div class='cutenews-tweet-username'>%2</div><div class='cutenews-tweet-text'>%3</div>%4<div class='cutenews-tweet-text'>%5</div></div>");

struct Tweet {
    QString author;
    QString body;
    QDateTime date;
    QString title;
    QString url;

    bool operator<(const Tweet &other) const {
        return date > other.date;
    }

    bool operator>(const Tweet &other) const {
        return date < other.date;
    }
};

#endif // TWEET_H
