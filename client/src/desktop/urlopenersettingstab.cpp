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

#include "urlopenersettingstab.h"
#include "urlopenermodel.h"
#include <QFormLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTreeView>

UrlOpenerSettingsTab::UrlOpenerSettingsTab(QWidget *parent) :
    SettingsTab(parent),
    m_view(new QTreeView(this)),
    m_regexpEdit(new QLineEdit(this)),
    m_commandEdit(new QLineEdit(this)),
    m_saveButton(new QPushButton(QIcon::fromTheme("document-save"), tr("&Save"), this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("URL openers"));

    m_view->setModel(UrlOpenerModel::instance());
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setItemsExpandable(false);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setRootIsDecorated(false);
    m_view->header()->setStretchLastSection(true);

    m_saveButton->setEnabled(false);

    m_layout->addRow(m_view);
    m_layout->addRow(tr("&RegExp:"), m_regexpEdit);
    m_layout->addRow(tr("&Command:"), m_commandEdit);
    m_layout->addWidget(m_saveButton);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentOpener(QModelIndex)));
    connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(m_regexpEdit, SIGNAL(textChanged(QString)), this, SLOT(onRegExpChanged(QString)));
    connect(m_regexpEdit, SIGNAL(returnPressed()), m_saveButton, SLOT(animateClick()));
    connect(m_commandEdit, SIGNAL(textChanged(QString)), this, SLOT(onCommandChanged(QString)));
    connect(m_commandEdit, SIGNAL(returnPressed()), m_saveButton, SLOT(animateClick()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(addOpener()));

    restore();
}

void UrlOpenerSettingsTab::restore() {
    UrlOpenerModel::instance()->load();
}

void UrlOpenerSettingsTab::save() {
    UrlOpenerModel::instance()->save();
}

void UrlOpenerSettingsTab::addOpener() {
    UrlOpenerModel::instance()->append(m_regexpEdit->text(), m_commandEdit->text());
    m_regexpEdit->clear();
    m_commandEdit->clear();
}

void UrlOpenerSettingsTab::setCurrentOpener(const QModelIndex &index) {
    m_regexpEdit->setText(index.data(UrlOpenerModel::NameRole).toString());
    m_commandEdit->setText(index.data(UrlOpenerModel::ValueRole).toString());
}

void UrlOpenerSettingsTab::showContextMenu(const QPoint &pos) {
    if (!m_view->currentIndex().isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Remove"));

    if (menu.exec(m_view->mapToGlobal(pos))) {
        UrlOpenerModel::instance()->remove(m_view->currentIndex().row());
    }
}

void UrlOpenerSettingsTab::onRegExpChanged(const QString &pattern) {
    m_saveButton->setEnabled((!pattern.isEmpty()) && (!m_commandEdit->text().isEmpty()));
}

void UrlOpenerSettingsTab::onCommandChanged(const QString &command) {
    m_saveButton->setEnabled((!command.isEmpty()) && (!m_regexpEdit->text().isEmpty()));
}
