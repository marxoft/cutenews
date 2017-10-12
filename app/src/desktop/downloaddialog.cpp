/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "downloaddialog.h"
#include "categorynamemodel.h"
#include "pluginmanager.h"
#include "transferprioritymodel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>

DownloadDialog::DownloadDialog(QWidget *parent) :
    QDialog(parent),
    m_categoryModel(new CategoryNameModel(this)),
    m_priorityModel(new TransferPriorityModel(this)),
    m_commandEdit(new QLineEdit(this)),
    m_categorySelector(new QComboBox(this)),
    m_prioritySelector(new QComboBox(this)),
    m_pluginCheckBox(new QCheckBox(tr("Use &plugin"), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Download"));

    m_categorySelector->setModel(m_categoryModel);
    m_prioritySelector->setModel(m_priorityModel);

    m_pluginCheckBox->setEnabled(false);

    m_layout->addRow(tr("Custom &command:"), m_commandEdit);
    m_layout->addRow(tr("&Category:"), m_categorySelector);
    m_layout->addRow(tr("&Priority:"), m_prioritySelector);
    m_layout->addRow(m_pluginCheckBox);
    m_layout->addRow(m_buttonBox);

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setPriority(Transfer::NormalPriority);
}

DownloadDialog::DownloadDialog(const QString &url, QWidget *parent) :
    QDialog(parent),
    m_categoryModel(new CategoryNameModel(this)),
    m_priorityModel(new TransferPriorityModel(this)),
    m_commandEdit(new QLineEdit(this)),
    m_categorySelector(new QComboBox(this)),
    m_prioritySelector(new QComboBox(this)),
    m_pluginCheckBox(new QCheckBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Download"));

    m_categorySelector->setModel(m_categoryModel);
    m_prioritySelector->setModel(m_priorityModel);

    m_layout->addRow(tr("Custom &command:"), m_commandEdit);
    m_layout->addRow(tr("&Category:"), m_categorySelector);
    m_layout->addRow(tr("&Priority:"), m_prioritySelector);
    m_layout->addRow(m_pluginCheckBox);
    m_layout->addRow(m_buttonBox);

    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setPriority(Transfer::NormalPriority);
    setUrl(url);
}

QString DownloadDialog::category() const {
    return m_categorySelector->currentText();
}

void DownloadDialog::setCategory(const QString &category) {
    m_categorySelector->setCurrentIndex(m_categorySelector->findData(category));
}

QString DownloadDialog::customCommand() const {
    return m_commandEdit->text();
}

void DownloadDialog::setCustomCommand(const QString &command) {
    m_commandEdit->setText(command);
}

int DownloadDialog::priority() const {
    return m_prioritySelector->itemData(m_prioritySelector->currentIndex()).toInt();
}

void DownloadDialog::setPriority(int priority) {
    m_prioritySelector->setCurrentIndex(m_prioritySelector->findData(priority));
}

QString DownloadDialog::url() const {
    return m_url;
}

void DownloadDialog::setUrl(const QString &url) {
    m_url = url;
    const FeedPluginConfig *config = PluginManager::instance()->getConfigForEnclosure(url);

    if (config) {
        m_pluginCheckBox->setEnabled(true);
        m_pluginCheckBox->setChecked(true);
        m_pluginCheckBox->setText(tr("Use %1 &plugin").arg(config->displayName()));
    }
    else {
        m_pluginCheckBox->setEnabled(false);
        m_pluginCheckBox->setChecked(false);
        m_pluginCheckBox->setText(tr("Use &plugin"));
    }
}

bool DownloadDialog::usePlugin() const {
    return (m_pluginCheckBox->isEnabled()) && (m_pluginCheckBox->isChecked());
}

void DownloadDialog::setUsePlugin(bool enabled) {
    if (m_pluginCheckBox->isEnabled()) {
        m_pluginCheckBox->setChecked(enabled);
    }
}
