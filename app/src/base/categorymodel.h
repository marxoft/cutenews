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

#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include "selectionmodel.h"

class CategoryModel : public SelectionModel
{
    Q_OBJECT

public:
    CategoryModel(QObject *parent = 0);
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    
    Q_INVOKABLE void append(const QString &name, const QVariant &path);
    Q_INVOKABLE void insert(int row, const QString &name, const QVariant &path);

public Q_SLOTS:
    void load();
    void save();
};

#endif // CATEGORYMODEL_H
