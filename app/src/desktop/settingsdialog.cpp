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
#include "definitions.h"
#include "settings.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabBar>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_downloadsCheckBox(0),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this)),
    m_proxyGroupBox(0),
    m_downloadPathEdit(0),
    m_proxyHostEdit(0),
    m_proxyUsernameEdit(0),
    m_proxyPasswordEdit(0),
    m_downloadPathButton(0),
    m_downloadsSpinBox(0),
    m_proxyPortSpinBox(0),
    m_stack(new QStackedWidget(this)),
    m_tabs(new QTabBar(this)),
    m_generalTab(0),
    m_networkTab(0),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Preferences"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMinimumSize(QSize(400, 400));
    
    m_tabs->addTab(tr("General"));
    m_tabs->addTab(tr("Network"));
    m_tabs->setExpanding(false);
    
    m_layout->addWidget(m_tabs);
    m_layout->addWidget(m_stack);
    m_layout->addWidget(m_buttonBox);
    m_layout->setStretch(1, 1);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTab(int)));
    
    showGeneralTab();
}

void SettingsDialog::accept() {
    if (m_generalTab) {
        Settings::instance()->setNetworkProxy();
    }
    
    QDialog::accept();
}

void SettingsDialog::showGeneralTab() {
    if (!m_generalTab) {
        m_generalTab = new QWidget(m_stack);
        m_downloadsCheckBox = new QCheckBox(tr("Start downloads automatically"), m_stack);
        m_downloadsCheckBox->setChecked(Settings::instance()->startTransfersAutomatically());
        m_downloadPathEdit = new QLineEdit(Settings::instance()->downloadPath(), m_stack);
        m_downloadPathButton = new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), m_stack);
        m_downloadsSpinBox = new QSpinBox(m_stack);
        m_downloadsSpinBox->setRange(1, MAX_CONCURRENT_TRANSFERS);
        m_downloadsSpinBox->setValue(Settings::instance()->maximumConcurrentTransfers());
                
        QGridLayout *grid = new QGridLayout(m_generalTab);
        grid->addWidget(new QLabel(tr("Download path:"), m_generalTab), 0, 0);
        grid->addWidget(m_downloadPathEdit, 0, 1);
        grid->addWidget(m_downloadPathButton, 0, 2);
        grid->addWidget(m_downloadsCheckBox, 1, 0, 1, 3);
        grid->addWidget(new QLabel(tr("Maximum concurrent downloads:"), m_generalTab), 2, 0);
        grid->addWidget(m_downloadsSpinBox, 2, 1, 1, 2);
        
        connect(m_downloadsCheckBox, SIGNAL(toggled(bool)),
                Settings::instance(), SLOT(setStartTransfersAutomatically(bool)));
        connect(m_downloadPathEdit, SIGNAL(textChanged(QString)), Settings::instance(), SLOT(setDownloadPath(QString)));
        connect(m_downloadPathButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
        connect(m_downloadsSpinBox, SIGNAL(valueChanged(int)),
                Settings::instance(), SLOT(setMaximumConcurrentTransfers(int)));
        
        m_stack->addWidget(m_generalTab);
    }
    
    m_stack->setCurrentWidget(m_generalTab);
}

void SettingsDialog::showNetworkTab() {
    if (!m_networkTab) {
        m_networkTab = new QWidget(m_stack);
        m_proxyGroupBox = new QGroupBox(tr("Use network proxy"), m_stack);
        m_proxyGroupBox->setCheckable(true);
        m_proxyGroupBox->setChecked(Settings::instance()->networkProxyEnabled());
        m_proxyHostEdit = new QLineEdit(Settings::instance()->networkProxyHost(), m_proxyGroupBox);
        m_proxyPortSpinBox = new QSpinBox(m_proxyGroupBox);
        m_proxyPortSpinBox->setRange(1, 9999);
        m_proxyPortSpinBox->setValue(Settings::instance()->networkProxyPort());
        m_proxyUsernameEdit = new QLineEdit(Settings::instance()->networkProxyUsername(), m_proxyGroupBox);
        m_proxyPasswordEdit = new QLineEdit(Settings::instance()->networkProxyPassword(), m_proxyGroupBox);
        m_proxyPasswordEdit->setEchoMode(QLineEdit::Password);
        
        QFormLayout *form = new QFormLayout(m_proxyGroupBox);
        form->addRow(tr("&Host:"), m_proxyHostEdit);
        form->addRow(tr("&Port:"), m_proxyPortSpinBox);
        form->addRow(tr("&Username:"), m_proxyUsernameEdit);
        form->addRow(tr("&Password:"), m_proxyPasswordEdit);
        
        QVBoxLayout *vbox = new QVBoxLayout(m_networkTab);
        vbox->addWidget(m_proxyGroupBox);
        
        connect(m_proxyGroupBox, SIGNAL(toggled(bool)), Settings::instance(), SLOT(setNetworkProxyEnabled(bool)));
        connect(m_proxyHostEdit, SIGNAL(textChanged(QString)),
                Settings::instance(), SLOT(setNetworkProxyHost(QString)));
        connect(m_proxyPortSpinBox, SIGNAL(valueChanged(int)), Settings::instance(), SLOT(setNetworkProxyPort(int)));
        connect(m_proxyUsernameEdit, SIGNAL(textChanged(QString)),
                Settings::instance(), SLOT(setNetworkProxyUsername(QString)));
        connect(m_proxyPasswordEdit, SIGNAL(textChanged(QString)),
                Settings::instance(), SLOT(setNetworkProxyPassword(QString)));
        
        m_stack->addWidget(m_networkTab);
    }
    
    m_stack->setCurrentWidget(m_networkTab);
}

void SettingsDialog::setCurrentTab(int index) {
    switch (index) {
    case 0:
        showGeneralTab();
        break;
    case 1:
        showNetworkTab();
        break;
    default:
        break;
    }
}

void SettingsDialog::showFileDialog() {
    const QString path = QFileDialog::getExistingDirectory(this, tr("Select folder"),
                                                           Settings::instance()->downloadPath());
    
    if (!path.isEmpty()) {
        m_downloadPathEdit->setText(path);
    }
}
