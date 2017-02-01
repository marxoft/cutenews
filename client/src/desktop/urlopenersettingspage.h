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

#ifndef URLOPENERSETTINGSPAGE_H
#define URLOPENERSETTINGSPAGE_H

#include "settingspage.h"

class QFormLayout;
class QLineEdit;
class QPushButton;
class QTreeView;

class UrlOpenerSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit UrlOpenerSettingsPage(QWidget *parent = 0);

public Q_SLOTS:
    virtual void restore();
    virtual void save();

private Q_SLOTS:
    void addOpener();
    void setCurrentOpener(const QModelIndex &index);
    
    void showContextMenu(const QPoint &pos);
    
    void onRegExpChanged(const QString &pattern);
    void onCommandChanged(const QString &command);

private:    
    QTreeView *m_view;

    QLineEdit *m_regexpEdit;
    QLineEdit *m_commandEdit;
    
    QPushButton *m_saveButton;
    
    QFormLayout *m_layout;
};

#endif // URLOPENERSETTINGSPAGE_H
