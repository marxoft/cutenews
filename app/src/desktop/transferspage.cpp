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

#include "transferspage.h"
#include "customcommanddialog.h"
#include "definitions.h"
#include "settings.h"
#include "transfermodel.h"
#include "transfers.h"
#include <QActionGroup>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOptionProgressBar>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

TransfersPage::TransfersPage(QWidget *parent) :
    Page(parent),
    m_model(new TransferModel(this)),
    m_transferMenu(new QMenu(tr("&Download"), this)),
    m_categoryMenu(new QMenu(tr("&Category"), this)),
    m_priorityMenu(new QMenu(tr("&Priority"), this)),
    m_propertiesMenu(new QMenu(tr("&Properties"), this)),
    m_concurrentMenu(new QMenu(tr("Maximum &concurrent downloads"), this)),
    m_categoryGroup(new QActionGroup(this)),
    m_priorityGroup(new QActionGroup(this)),
    m_concurrentGroup(new QActionGroup(this)),
    m_startAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("&Start all downloads"), this)),
    m_pauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause all downloads"), this)),
    m_propertiesAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Properties"), this)),
    m_transferCommandAction(new QAction(QIcon::fromTheme("system-run"), tr("Set &custom command"), this)),
    m_transferStartAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("&Start"), this)),
    m_transferPauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause"), this)),
    m_transferRemoveAction(new QAction(QIcon::fromTheme("edit-delete"), tr("&Remove"), this)),
    m_toolBar(new QToolBar(this)),
    m_view(new QTreeView(this)),
    m_layout(new QVBoxLayout(this))
{
    m_transferMenu->addAction(m_transferCommandAction);
    m_transferMenu->addAction(m_transferStartAction);
    m_transferMenu->addAction(m_transferPauseAction);
    m_transferMenu->addMenu(m_categoryMenu);
    m_transferMenu->addMenu(m_priorityMenu);
    m_transferMenu->addAction(m_transferRemoveAction);
    m_transferMenu->setEnabled(false);
    
    setCategoryMenuActions();
    
    const QStringList priorities = QStringList() << tr("Highest") << tr("High") << tr("Normal") << tr("Low")
                                                 << tr("Lowest");
    
    for (int i = 0; i < priorities.size(); i++) {
        QAction *action = m_priorityMenu->addAction(priorities.at(i), this, SLOT(setTransferPriority()));
        action->setCheckable(true);
        action->setData(i);
        m_priorityGroup->addAction(action);
    }
    
    const int max = Settings::maximumConcurrentTransfers();
    
    for (int i = 1; i <= MAX_CONCURRENT_TRANSFERS; i++) {
        QAction *action = m_concurrentMenu->addAction(QString::number(i), this, SLOT(setMaximumConcurrentTransfers()));
        action->setCheckable(true);
        action->setChecked(i == max);
        action->setData(i);
        m_concurrentGroup->addAction(action);
    }
    
    m_propertiesMenu->addMenu(m_concurrentMenu);
    
    m_toolBar->setMovable(false);
    m_toolBar->addAction(m_startAction);
    m_toolBar->addAction(m_pauseAction);
    m_toolBar->addAction(m_propertiesAction);
    
    m_view->setModel(m_model);
    m_view->setItemDelegate(new TransferDelegate(m_view));
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setItemsExpandable(false);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setRootIsDecorated(false);
    m_view->header()->restoreState(Settings::transfersHeaderViewState());

    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_view);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    connect(m_categoryMenu, SIGNAL(aboutToShow()), this, SLOT(setActiveCategoryMenuAction()));
    connect(m_priorityMenu, SIGNAL(aboutToShow()), this, SLOT(setActivePriorityMenuAction()));
    connect(m_startAction, SIGNAL(triggered()), Transfers::instance(), SLOT(start()));
    connect(m_pauseAction, SIGNAL(triggered()), Transfers::instance(), SLOT(pause()));
    connect(m_propertiesAction, SIGNAL(triggered()), this, SLOT(showPropertiesMenu()));
    connect(m_transferCommandAction, SIGNAL(triggered()), this, SLOT(setTransferCustomCommand()));
    connect(m_transferStartAction, SIGNAL(triggered()), this, SLOT(queueTransfer()));
    connect(m_transferPauseAction, SIGNAL(triggered()), this, SLOT(pauseTransfer()));
    connect(m_transferRemoveAction, SIGNAL(triggered()), this, SLOT(removeTransfer()));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_view->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentTransferChanged(QModelIndex)));
    connect(Settings::instance(), SIGNAL(categoriesChanged()), this, SLOT(setCategoryMenuActions()));
    connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int)),
            this, SLOT(onMaximumConcurrentTransfersChanged(int)));
}

void TransfersPage::closeEvent(QCloseEvent *e) {
    Settings::setTransfersHeaderViewState(m_view->header()->saveState());
    QWidget::closeEvent(e);
}

void TransfersPage::queueTransfer() {
    if (m_view->currentIndex().isValid()) {
        queueTransfer(m_view->currentIndex());
    }
}

void TransfersPage::queueTransfer(const QModelIndex &index) {
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
        transfer->queue();
    }
}

void TransfersPage::pauseTransfer() {
    if (m_view->currentIndex().isValid()) {
        pauseTransfer(m_view->currentIndex());
    }
}

void TransfersPage::pauseTransfer(const QModelIndex &index) {
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
        transfer->pause();
    }
}

void TransfersPage::removeTransfer() {
    if (m_view->currentIndex().isValid()) {
        removeTransfer(m_view->currentIndex());
    }
}

void TransfersPage::removeTransfer(const QModelIndex &index) {
    if (Transfer *transfer = Transfers::instance()->get(index.row())) {
        if (QMessageBox::question(this, tr("Remove?"),
            tr("Do you want to remove download '%1'?").arg(transfer->name())) == QMessageBox::Yes) {
            transfer->cancel();
        }
    }
}

void TransfersPage::setTransferCategory() {
    if (m_view->currentIndex().isValid()) {
        if (const QAction *action = m_categoryGroup->checkedAction()) {
            setTransferCategory(m_view->currentIndex(), action->text());
        }
    }
}

void TransfersPage::setTransferCategory(const QModelIndex &index, const QString &category) {
    m_model->setData(index, category, Transfer::CategoryRole);
}

void TransfersPage::setTransferCustomCommand() {
    const QModelIndex index = m_view->currentIndex();
    
    if (index.isValid()) {
        CustomCommandDialog dialog(this);
        dialog.setCommand(index.data(Transfer::CustomCommandRole).toString());
        dialog.setOverrideEnabled(index.data(Transfer::CustomCommandOverrideEnabledRole).toBool());
        
        if (dialog.exec() == QDialog::Accepted) {
            setTransferCustomCommand(index, dialog.command(), dialog.overrideEnabled());
        }
    }
}

void TransfersPage::setTransferCustomCommand(const QModelIndex &index, const QString &command, bool overrideEnabled) {
    m_model->setData(index, command, Transfer::CustomCommandRole);
    m_model->setData(index, overrideEnabled, Transfer::CustomCommandOverrideEnabledRole);
}

void TransfersPage::setTransferPriority() {
    if (m_view->currentIndex().isValid()) {
        if (const QAction *action = m_priorityGroup->checkedAction()) {
            setTransferPriority(m_view->currentIndex(), action->data().toInt());
        }
    }
}

void TransfersPage::setTransferPriority(const QModelIndex &index, int priority) {
    m_model->setData(index, priority, Transfer::PriorityRole);
}

void TransfersPage::setCategoryMenuActions() {
    const QStringList categories = Settings::categoryNames();
    m_categoryMenu->clear();
    
    foreach (const QString &category, categories) {
        QAction *action = m_categoryMenu->addAction(category, this, SLOT(setTransferCategory()));
        action->setCheckable(true);
        action->setData(category);
        m_categoryGroup->addAction(action);
    }
}

void TransfersPage::setActiveCategoryMenuAction() {
    if (!m_view->currentIndex().isValid()) {
        return;
    }
    
    const QVariant category = m_view->currentIndex().data(Transfer::CategoryRole).toString();
    
    foreach (QAction *action, m_categoryGroup->actions()) {
        if (action->data() == category) {
            action->setChecked(true);
            break;
        }
    }
}

void TransfersPage::setActivePriorityMenuAction() {
    if (!m_view->currentIndex().isValid()) {
        return;
    }
    
    const QVariant priority = m_view->currentIndex().data(Transfer::PriorityRole);
    
    foreach (QAction *action, m_priorityGroup->actions()) {
        if (action->data() == priority) {
            action->setChecked(true);
            break;
        }
    }
}

void TransfersPage::setMaximumConcurrentTransfers() {
    if (const QAction *action = m_concurrentGroup->checkedAction()) {
        Settings::setMaximumConcurrentTransfers(action->data().toInt());
    }
}

void TransfersPage::showContextMenu(const QPoint &pos) {
    if (m_view->currentIndex().isValid()) {
        m_transferMenu->popup(m_view->mapToGlobal(pos));
    }
}

void TransfersPage::showPropertiesMenu() {
    m_propertiesMenu->popup(QCursor::pos());
}

void TransfersPage::onCurrentTransferChanged(const QModelIndex &index) {
    m_transferMenu->setEnabled(index.isValid());
}

void TransfersPage::onMaximumConcurrentTransfersChanged(int maximum) {
    foreach (QAction *action, m_concurrentGroup->actions()) {
        if (action->data() == maximum) {
            action->setChecked(true);
            break;
        }
    }
}

TransferDelegate::TransferDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void TransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.column() == 2) {
        QStyleOptionProgressBar progressBar;
        progressBar.rect = option.rect;
        progressBar.minimum = 0;
        progressBar.maximum = 100;
        progressBar.progress = index.data(Transfer::ProgressRole).toInt();
        progressBar.textVisible = true;
        progressBar.text = index.data(Transfer::ProgressStringRole).toString();
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBar, painter);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
