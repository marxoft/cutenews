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

#include "categorymodel.h"
#include "settings.h"

CategoryModel::CategoryModel(QObject *parent) :
    SelectionModel(parent)
{
    load();
}

QVariant CategoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Path");
    default:
        return QVariant();
    }
}

void CategoryModel::append(const QString &name, const QVariant &path) {
    const int i = match(0, "name", name);

    if (i == -1) {
        SelectionModel::append(name, path);
    }
    else {
        setData(i, path, "value");
    }
}

void CategoryModel::insert(int row, const QString &name, const QVariant &path) {
    const int i = match(0, "name", name);

    if (i == -1) {
        SelectionModel::insert(row, name, path);
    }
    else {
        setData(i, path, "value");
    }
}

void CategoryModel::load() {
    clear();
    
    foreach (const Category &category, Settings::categories()) {
        append(category.name, category.path);
    }
}

void CategoryModel::save() {
    QList<Category> categories;

    for (int i = 0; i < rowCount(); i++) {
        Category category;
        category.name = data(i, "name").toString();
        category.path = data(i, "value").toString();
        categories << category;
    }

    Settings::setCategories(categories);
}
