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

#include "settingsdialog.h"
#include "clientsettingspage.h"
#include "serversettingspage.h"
#include "urlopenersettingspage.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_clientPage(0),
    m_serverPage(0),
    m_openerPage(0),
    m_tabBar(new QTabBar(this)),
    m_stack(new QStackedWidget(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Preferences"));

    m_tabBar->addTab(tr("Client"));
    m_tabBar->addTab(tr("Server"));
    m_tabBar->addTab(tr("URL openers"));
    
    m_buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
    
    m_layout->addWidget(m_tabBar);
    m_layout->addWidget(m_stack);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(setCurrentPage(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    showClientPage();
}

void SettingsDialog::accept() {
    for (int i = 0; i < m_stack->count(); i++) {
        if (SettingsPage *page = qobject_cast<SettingsPage*>(m_stack->widget(i))) {
            page->save();
        }
    }

    QDialog::accept();
}

void SettingsDialog::setCurrentPage(int index) {
    switch (index) {
    case 0:
        showClientPage();
        break;
    case 1:
        showServerPage();
        break;
    case 2:
        showUrlOpenerPage();
        break;
    default:
        break;
    }
}

void SettingsDialog::showClientPage() {
    if (!m_clientPage) {
        m_clientPage = new ClientSettingsPage(m_stack);
        m_stack->addWidget(m_clientPage);
    }

    m_stack->setCurrentWidget(m_clientPage);
}

void SettingsDialog::showServerPage() {
    if (!m_serverPage) {
        m_serverPage = new ServerSettingsPage(m_stack);
        m_stack->addWidget(m_serverPage);
    }

    m_stack->setCurrentWidget(m_serverPage);
}

void SettingsDialog::showUrlOpenerPage() {
    if (!m_openerPage) {
        m_openerPage = new UrlOpenerSettingsPage(m_stack);
        m_stack->addWidget(m_openerPage);
    }

    m_stack->setCurrentWidget(m_openerPage);
}
