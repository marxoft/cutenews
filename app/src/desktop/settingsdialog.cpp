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
#include "categorysettingspage.h"
#include "generalsettingspage.h"
#include "interfacesettingspage.h"
#include "networksettingspage.h"
#include "pluginssettingspage.h"
#include "urlopenersettingspage.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_generalPage(0),
    m_networkPage(0),
    m_interfacePage(0),
    m_categoryPage(0),
    m_openerPage(0),
    m_pluginPage(0),
    m_tabBar(new QTabBar(this)),
    m_stack(new QStackedWidget(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Preferences"));

    m_tabBar->addTab(tr("General"));
    m_tabBar->addTab(tr("Network"));
    m_tabBar->addTab(tr("Interfaces"));
    m_tabBar->addTab(tr("Categories"));
    m_tabBar->addTab(tr("URL openers"));
    m_tabBar->addTab(tr("Plugins"));
    
    m_buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
    
    m_layout->addWidget(m_tabBar);
    m_layout->addWidget(m_stack);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(setCurrentPage(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    showGeneralPage();
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
        showGeneralPage();
        break;
    case 1:
        showNetworkPage();
        break;
    case 2:
        showInterfacePage();
        break;
    case 3:
        showCategoryPage();
        break;
    case 4:
        showUrlOpenerPage();
        break;
    case 5:
        showPluginPage();
        break;
    default:
        break;
    }
}

void SettingsDialog::showGeneralPage() {
    if (!m_generalPage) {
        m_generalPage = new GeneralSettingsPage(m_stack);
        m_stack->addWidget(m_generalPage);
    }

    m_stack->setCurrentWidget(m_generalPage);
}

void SettingsDialog::showNetworkPage() {
    if (!m_networkPage) {
        m_networkPage = new NetworkSettingsPage(m_stack);
        m_stack->addWidget(m_networkPage);
    }

    m_stack->setCurrentWidget(m_networkPage);
}

void SettingsDialog::showInterfacePage() {
    if (!m_interfacePage) {
        m_interfacePage = new InterfaceSettingsPage(m_stack);
        m_stack->addWidget(m_interfacePage);
    }

    m_stack->setCurrentWidget(m_interfacePage);
}

void SettingsDialog::showCategoryPage() {
    if (!m_categoryPage) {
        m_categoryPage = new CategorySettingsPage(m_stack);
        m_stack->addWidget(m_categoryPage);
    }

    m_stack->setCurrentWidget(m_categoryPage);
}

void SettingsDialog::showUrlOpenerPage() {
    if (!m_openerPage) {
        m_openerPage = new UrlOpenerSettingsPage(m_stack);
        m_stack->addWidget(m_openerPage);
    }

    m_stack->setCurrentWidget(m_openerPage);
}

void SettingsDialog::showPluginPage() {
    if (!m_pluginPage) {
        m_pluginPage = new PluginsSettingsPage(m_stack);
        m_stack->addWidget(m_pluginPage);
    }

    m_stack->setCurrentWidget(m_pluginPage);
}
