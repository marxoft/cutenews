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

#include "downloadsview.h"
#include "transfermodel.h"
#include "transfers.h"
#include <QActionGroup>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionProgressBar>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

DownloadsView::DownloadsView(QWidget *parent) :
    QWidget(parent),
    m_model(new TransferModel(this)),
    m_toolBar(new QToolBar(this)),
    m_view(new QTreeView(this)),
    m_layout(new QVBoxLayout(this)),
    m_contextMenu(new QMenu(this)),
    m_priorityMenu(new QMenu(tr("Priority"), this)),
    m_startAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("Start all downloads"), this)),
    m_pauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("Pause all downloads"), this)),
    m_startTransferAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("Start"), this)),
    m_pauseTransferAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("Pause"), this)),
    m_cancelTransferAction(new QAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this)),
    m_highPriorityAction(new QAction(tr("High"), this)),
    m_normalPriorityAction(new QAction(tr("Normal"), this)),
    m_lowPriorityAction(new QAction(tr("Low"), this)),
    m_priorityActionGroup(new QActionGroup(this))
{
    m_toolBar->addAction(m_startAction);
    m_toolBar->addAction(m_pauseAction);
    m_toolBar->setMovable(false);
    
    m_view->setModel(m_model);
    m_view->setItemDelegate(new DownloadDelegate(m_view));
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setItemsExpandable(false);
    m_view->setIndentation(0);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    
    QHeaderView *header = m_view->header();
    QFontMetrics fm = header->fontMetrics();
    header->resizeSection(0, fm.width("A long download name") + 10);
    header->resizeSection(1, fm.width(m_model->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString() + 10));
    header->resizeSection(2, fm.width("999.99MB of 999.99MB (99.99%)") + 10);
    
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_view);
    m_layout->setStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    m_contextMenu->addAction(m_startTransferAction);
    m_contextMenu->addAction(m_pauseTransferAction);
    m_contextMenu->addMenu(m_priorityMenu);
    m_contextMenu->addAction(m_cancelTransferAction);
    
    m_priorityMenu->addAction(m_highPriorityAction);
    m_priorityMenu->addAction(m_normalPriorityAction);
    m_priorityMenu->addAction(m_lowPriorityAction);
    
    m_highPriorityAction->setCheckable(true);
    m_highPriorityAction->setData(Transfer::HighPriority);
    m_highPriorityAction->setActionGroup(m_priorityActionGroup);
    m_normalPriorityAction->setCheckable(true);
    m_normalPriorityAction->setData(Transfer::NormalPriority);
    m_normalPriorityAction->setActionGroup(m_priorityActionGroup);
    m_lowPriorityAction->setCheckable(true);
    m_lowPriorityAction->setData(Transfer::LowPriority);
    m_lowPriorityAction->setActionGroup(m_priorityActionGroup);
    
    m_priorityActionGroup->setExclusive(true);
    
    connect(Transfers::instance(), SIGNAL(countChanged(int)), this, SLOT(onCountChanged(int)));
    
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    
    connect(m_priorityMenu, SIGNAL(triggered(QAction*)), this, SLOT(setCurrentTransferPriority(QAction*)));
    
    connect(m_startAction, SIGNAL(triggered()), Transfers::instance(), SLOT(start()));
    connect(m_pauseAction, SIGNAL(triggered()), Transfers::instance(), SLOT(pause()));
    
    connect(m_startTransferAction, SIGNAL(triggered()), this, SLOT(startCurrentTransfer()));
    connect(m_pauseTransferAction, SIGNAL(triggered()), this, SLOT(pauseCurrentTransfer()));
    connect(m_cancelTransferAction, SIGNAL(triggered()), this, SLOT(cancelCurrentTransfer()));
    
    onCountChanged(Transfers::instance()->count());
}

void DownloadsView::startCurrentTransfer() {
    if (Transfer *transfer = Transfers::instance()->get(m_view->currentIndex().row())) {
        transfer->queue();
    }
}

void DownloadsView::pauseCurrentTransfer() {
    if (Transfer *transfer = Transfers::instance()->get(m_view->currentIndex().row())) {
        transfer->pause();
    }
}

void DownloadsView::cancelCurrentTransfer() {
    if (Transfer *transfer = Transfers::instance()->get(m_view->currentIndex().row())) {
        transfer->cancel();
    }
}

void DownloadsView::setCurrentTransferPriority(QAction *action) {
    if (action) {
        m_model->setData(m_view->currentIndex(), action->data().toInt(), TransferModel::PriorityRole);
    }
}

void DownloadsView::showContextMenu(const QPoint &pos) {
    const QModelIndex index = m_view->currentIndex();
    
    if (!index.isValid()) {
        return;
    }
        
    switch (index.data(TransferModel::StatusRole).toInt()) {
    case Transfer::Paused:
    case Transfer::Failed:
        m_startTransferAction->setEnabled(true);
        m_pauseTransferAction->setEnabled(false);
        break;
    default:
        m_startTransferAction->setEnabled(false);
        m_pauseTransferAction->setEnabled(true);
        break;
    }

    switch (index.data(TransferModel::PriorityRole).toInt()) {
    case Transfer::HighPriority:
        m_highPriorityAction->setChecked(true);
        break;
    case Transfer::LowPriority:
        m_lowPriorityAction->setChecked(true);
        break;
    default:
        m_normalPriorityAction->setChecked(true);
        break;
    }
    
    m_contextMenu->popup(m_view->mapToGlobal(pos), m_startTransferAction);
}

void DownloadsView::onCountChanged(int count) {
    const bool enable = (count > 0);
    m_startAction->setEnabled(enable);
    m_pauseAction->setEnabled(enable);
}

DownloadDelegate::DownloadDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == 2) {
        QStyleOptionProgressBar progressBar;
        progressBar.rect = option.rect;
        progressBar.minimum = 0;
        progressBar.maximum = 100;
        progressBar.progress = index.data(TransferModel::ProgressRole).toInt();
        progressBar.textVisible = true;
        progressBar.text = index.data(Qt::DisplayRole).toString();
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
