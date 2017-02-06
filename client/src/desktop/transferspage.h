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

#ifndef TRANSFERSPAGE_H
#define TRANSFERSPAGE_H

#include "page.h"
#include <QStyledItemDelegate>

class QAction;
class QActionGroup;
class QMenu;
class QToolBar;
class QTreeView;
class QVBoxLayout;

class TransfersPage : public Page
{
    Q_OBJECT

public:
    explicit TransfersPage(QWidget *parent = 0);
    
private Q_SLOTS:
    void queueTransfer();
    void queueTransfer(const QModelIndex &index);
    
    void pauseTransfer();
    void pauseTransfer(const QModelIndex &index);
    
    void removeTransfer();
    void removeTransfer(const QModelIndex &index);
    
    void setTransferCategory();
    void setTransferCategory(const QModelIndex &index, const QString &category);
    
    void setTransferCustomCommand();
    void setTransferCustomCommand(const QModelIndex &index, const QString &command, bool overrideEnabled);
    
    void setTransferPriority();
    void setTransferPriority(const QModelIndex &index, int priority);
    
    void setCategoryMenuActions(const QStringList &categories);
    
    void setActiveCategoryMenuAction();
    void setActivePriorityMenuAction();
    
    void setMaximumConcurrentTransfers();
    
    void showContextMenu(const QPoint &pos);
    void showPropertiesMenu();
    
    void onCurrentTransferChanged(const QModelIndex &index);
    void onMaximumConcurrentTransfersChanged(int maximum);

private:
    virtual void closeEvent(QCloseEvent *e);
        
    QMenu *m_transferMenu;
    QMenu *m_categoryMenu;
    QMenu *m_priorityMenu;
    QMenu *m_propertiesMenu;
    QMenu *m_concurrentMenu;
    
    QActionGroup *m_categoryGroup;
    QActionGroup *m_priorityGroup;
    QActionGroup *m_concurrentGroup;
    
    QAction *m_startAction;
    QAction *m_pauseAction;
    QAction *m_propertiesAction;
    
    QAction *m_transferCommandAction;
    QAction *m_transferStartAction;
    QAction *m_transferPauseAction;
    QAction *m_transferRemoveAction;
    
    QToolBar *m_toolBar;
        
    QTreeView *m_view;

    QVBoxLayout *m_layout;
};

class TransferDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TransferDelegate(QObject *parent = 0);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // TRANSFERSPAGE_H
