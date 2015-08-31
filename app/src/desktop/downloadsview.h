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

#ifndef DOWNLOADSVIEW_H
#define DOWNLOADSVIEW_H

#include <QWidget>
#include <QStyledItemDelegate>

class TransferModel;
class QAction;
class QActionGroup;
class QMenu;
class QToolBar;
class QTreeView;
class QVBoxLayout;

class DownloadsView : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadsView(QWidget *parent = 0);
    
private Q_SLOTS:
    void startCurrentTransfer();
    void pauseCurrentTransfer();
    void cancelCurrentTransfer();
    
    void setCurrentTransferPriority(QAction *action);
    
    void showContextMenu(const QPoint &pos);
    
    void onCountChanged(int count);
    
private:
    TransferModel *m_model;
    
    QToolBar *m_toolBar;
    
    QTreeView *m_view;
    
    QVBoxLayout *m_layout;
    
    QMenu *m_contextMenu;
    QMenu *m_priorityMenu;
    
    QAction *m_startAction;
    QAction *m_pauseAction;
    
    QAction *m_startTransferAction;
    QAction *m_pauseTransferAction;
    QAction *m_cancelTransferAction;
    QAction *m_highPriorityAction;
    QAction *m_normalPriorityAction;
    QAction *m_lowPriorityAction;
    
    QActionGroup *m_priorityActionGroup;
};

class DownloadDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DownloadDelegate(QObject *parent = 0);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DOWNLOADSVIEW_H
