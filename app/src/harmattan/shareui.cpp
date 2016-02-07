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

#include "shareui.h"
#include "article.h"
#include "utils.h"
#include <MDataUri>
#include <maemo-meegotouch-interfaces/shareuiinterface.h>
#ifdef CUTENEWS_DEBUG
#include <QDebug>
#endif

ShareUi::ShareUi(QObject *parent) :
    QObject(parent)
{
}

bool ShareUi::shareArticle(Article *article) {    
    QVariantMap attributes;
    attributes["title"] = article->title();
    attributes["source"] = article->url();
    attributes["description"] = article->body();
    return share("text/url", article->url().toString(), attributes);
}

bool ShareUi::share(const QString &uri) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "ShareUI:share" << uri;
#endif
    ShareUiInterface shareIf("com.nokia.ShareUi");

    if (shareIf.isValid()) {
        shareIf.share(QStringList(uri.startsWith("file") ? uri.mid(7) : uri));
        return true;
    }

    return false;
}

bool ShareUi::share(const QString &mimeType, const QString &textData, const QVariantMap &attributes) {
#ifdef CUTENEWS_DEBUG
    qDebug() << "ShareUI:share" << mimeType << textData << attributes;
#endif
    MDataUri duri;
    duri.setMimeType(mimeType);
    duri.setTextData(textData);

    QMapIterator<QString, QVariant> iterator(attributes);

    while (iterator.hasNext()) {
        iterator.next();
        duri.setAttribute(iterator.key(), iterator.value().toString());
    }

    if (duri.isValid()) {
        ShareUiInterface shareIf("com.nokia.ShareUi");

        if (shareIf.isValid()) {
            shareIf.share(QStringList(duri.toString()));
            return true;
        }
    }

    return false;
}
