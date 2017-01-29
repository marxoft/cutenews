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
#include "clientsettingstab.h"
#include "serversettingstab.h"
#include "urlopenersettingstab.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_clientTab(0),
    m_serverTab(0),
    m_openerTab(0),
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
    
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTab(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    showClientTab();
}

void SettingsDialog::accept() {
    for (int i = 0; i < m_stack->count(); i++) {
        if (SettingsTab *tab = qobject_cast<SettingsTab*>(m_stack->widget(i))) {
            tab->save();
        }
    }

    QDialog::accept();
}

void SettingsDialog::setCurrentTab(int index) {
    switch (index) {
    case 0:
        showClientTab();
        break;
    case 1:
        showServerTab();
        break;
    case 2:
        showUrlOpenerTab();
        break;
    default:
        break;
    }
}

void SettingsDialog::showClientTab() {
    if (!m_clientTab) {
        m_clientTab = new ClientSettingsTab(m_stack);
        m_stack->addWidget(m_clientTab);
    }

    m_stack->setCurrentWidget(m_clientTab);
}

void SettingsDialog::showServerTab() {
    if (!m_serverTab) {
        m_serverTab = new ServerSettingsTab(m_stack);
        m_stack->addWidget(m_serverTab);
    }

    m_stack->setCurrentWidget(m_serverTab);
}

void SettingsDialog::showUrlOpenerTab() {
    if (!m_openerTab) {
        m_openerTab = new UrlOpenerSettingsTab(m_stack);
        m_stack->addWidget(m_openerTab);
    }

    m_stack->setCurrentWidget(m_openerTab);
}
