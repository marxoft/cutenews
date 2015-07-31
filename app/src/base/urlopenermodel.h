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

#ifndef URLOPENERMODEL_H
#define URLOPENERMODEL_H

#include "selectionmodel.h"
#include "urlopener.h"

class UrlOpenerModel : public SelectionModel
{
    Q_OBJECT
    
public:
    explicit UrlOpenerModel(QObject *parent = 0) :
        SelectionModel(parent)
    {
        reload();
    }
    
public Q_SLOTS:
    inline void reload() {
        clear();
        
        QList<Opener> openers = UrlOpener::instance()->openers();
        
        for (int i = 0; i < openers.size(); i++) {
            QVariantMap m;
            m["regExp"] = openers.at(i).regExp.pattern();
            m["command"] = openers.at(i).command;
            append(openers.at(i).name, m);
        }
    }
};

#endif // URLOPENERMODEL_H
