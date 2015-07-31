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

#ifndef VIEWMODEMODEL_H
#define VIEWMODEMODEL_H

#include "selectionmodel.h"

class ViewModeModel : public SelectionModel
{
    Q_OBJECT
    
public:
    explicit ViewModeModel(QObject *parent = 0) :
        SelectionModel(parent)
    {
        append(tr("Dark on light"), QString("light"));
        append(tr("Light on dark"), QString("dark"));
    }
};

#endif // VIEWMODEMODEL_H
