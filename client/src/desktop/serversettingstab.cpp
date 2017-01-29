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

#include "serversettingstab.h"
#include "definitions.h"
#include "serversettings.h"
#include "settings.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

ServerSettingsTab::ServerSettingsTab(QWidget *parent) :
    SettingsTab(parent),
    m_commandEdit(new QLineEdit(this)),
    m_concurrentSpinBox(new QSpinBox(this)),
    m_commandCheckBox(new QCheckBox(tr("Enable &custom download command"), this)),
    m_automaticCheckBox(new QCheckBox(tr("Start downloads &automatically"), this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Server"));
    
    m_concurrentSpinBox->setRange(1, MAX_CONCURRENT_TRANSFERS);

    m_layout->addRow(tr("&Maximum concurrent downloads:"), m_concurrentSpinBox);
    m_layout->addRow(tr("&Custom download command (%f for filename):"), m_commandEdit);
    m_layout->addRow(m_commandCheckBox);
    m_layout->addRow(m_automaticCheckBox);
    
    connect(ServerSettings::instance(), SIGNAL(loaded()), this, SLOT(onSettingsLoaded()));
    restore();
}

void ServerSettingsTab::restore() {
    if (!Settings::serverAddress().isEmpty()) {
        ServerSettings::instance()->load();
    }
}

void ServerSettingsTab::save() {
    if (!Settings::serverAddress().isEmpty()) {
        ServerSettings::instance()->setMaximumConcurrentTransfers(m_concurrentSpinBox->value());
        ServerSettings::instance()->setCustomTransferCommand(m_commandEdit->text());
        ServerSettings::instance()->setCustomTransferCommandEnabled(m_commandCheckBox->isChecked());
        ServerSettings::instance()->setStartTransfersAutomatically(m_automaticCheckBox->isChecked());
    }
}

void ServerSettingsTab::onSettingsLoaded() {
    m_concurrentSpinBox->setValue(ServerSettings::instance()->maximumConcurrentTransfers());
    m_commandEdit->setText(ServerSettings::instance()->customTransferCommand());
    m_commandCheckBox->setChecked(ServerSettings::instance()->customTransferCommandEnabled());
    m_automaticCheckBox->setChecked(ServerSettings::instance()->startTransfersAutomatically());
}
