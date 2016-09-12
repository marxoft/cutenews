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
#include "networkproxytypemodel.h"
#include "settings.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_proxyTypeModel(0),
    m_downloadsCheckBox(0),
    m_webInterfaceAuthenticationCheckBox(0),
    m_proxyTypeSelector(0),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this)),
    m_proxyGroupBox(0),
    m_webInterfaceGroupBox(0),
    m_downloadPathEdit(0),
    m_proxyHostEdit(0),
    m_proxyUsernameEdit(0),
    m_proxyPasswordEdit(0),
    m_webInterfaceUsernameEdit(0),
    m_webInterfacePasswordEdit(0),
    m_downloadPathButton(0),
    m_expirySpinBox(0),
    m_downloadsSpinBox(0),
    m_proxyPortSpinBox(0),
    m_webInterfacePortSpinBox(0),
    m_stack(new QStackedWidget(this)),
    m_tabs(new QTabBar(this)),
    m_generalTab(0),
    m_networkTab(0),
    m_interfacesTab(0),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Preferences"));
    setMinimumSize(QSize(520, 400));
    
    m_tabs->addTab(tr("General"));
    m_tabs->addTab(tr("Network"));
    m_tabs->addTab(tr("Interfaces"));
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
    if (m_networkTab) {
        Settings::setNetworkProxy();
    }
    
    QDialog::accept();
}

void SettingsDialog::showGeneralTab() {
    if (!m_generalTab) {
        m_generalTab = new QWidget(m_stack);
        m_downloadsCheckBox = new QCheckBox(tr("Start downloads &automatically"), m_generalTab);
        m_downloadsCheckBox->setChecked(Settings::startTransfersAutomatically());
        m_downloadPathEdit = new QLineEdit(Settings::downloadPath(), m_generalTab);
        m_downloadPathButton = new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), m_generalTab);
        m_expirySpinBox = new QSpinBox(m_generalTab);
        m_expirySpinBox->setSuffix(tr(" day(s)"));
        m_expirySpinBox->setRange(-1, 90);
        m_expirySpinBox->setValue(Settings::readArticleExpiry());
        m_downloadsSpinBox = new QSpinBox(m_generalTab);
        m_downloadsSpinBox->setRange(1, MAX_CONCURRENT_TRANSFERS);
        m_downloadsSpinBox->setValue(Settings::maximumConcurrentTransfers());
        
        QFormLayout *form = new QFormLayout(m_generalTab);
        form->addRow(tr("&Delete read articles older than (-1 to disable):"), m_expirySpinBox);
        form->addRow(tr("Download &path:"), m_downloadPathEdit);
        form->addWidget(m_downloadPathButton);
        form->addRow(tr("&Maximum concurrent downloads:"), m_downloadsSpinBox);
        form->addRow(m_downloadsCheckBox);

        connect(m_expirySpinBox, SIGNAL(valueChanged(int)), Settings::instance(), SLOT(setReadArticleExpiry(int)));
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
        m_proxyGroupBox = new QGroupBox(tr("Use network proxy"), m_networkTab);
        m_proxyGroupBox->setCheckable(true);
        m_proxyGroupBox->setChecked(Settings::networkProxyEnabled());
        m_proxyTypeModel = new NetworkProxyTypeModel(m_proxyGroupBox);
        m_proxyTypeSelector = new QComboBox(m_proxyGroupBox);
        m_proxyTypeSelector->setModel(m_proxyTypeModel);
        m_proxyTypeSelector->setCurrentIndex(m_proxyTypeModel->match(0, "value", Settings::networkProxyType()));
        m_proxyHostEdit = new QLineEdit(Settings::networkProxyHost(), m_proxyGroupBox);
        m_proxyPortSpinBox = new QSpinBox(m_proxyGroupBox);
        m_proxyPortSpinBox->setRange(1, 9999);
        m_proxyPortSpinBox->setValue(Settings::networkProxyPort());
        m_proxyUsernameEdit = new QLineEdit(Settings::networkProxyUsername(), m_proxyGroupBox);
        m_proxyPasswordEdit = new QLineEdit(Settings::networkProxyPassword(), m_proxyGroupBox);
        m_proxyPasswordEdit->setEchoMode(QLineEdit::Password);
        
        QFormLayout *form = new QFormLayout(m_proxyGroupBox);
        form->addRow(tr("&Type:"), m_proxyTypeSelector);
        form->addRow(tr("&Host:"), m_proxyHostEdit);
        form->addRow(tr("&Port:"), m_proxyPortSpinBox);
        form->addRow(tr("&Username:"), m_proxyUsernameEdit);
        form->addRow(tr("&Password:"), m_proxyPasswordEdit);
        
        QVBoxLayout *vbox = new QVBoxLayout(m_networkTab);
        vbox->addWidget(m_proxyGroupBox);
        
        connect(m_proxyGroupBox, SIGNAL(toggled(bool)), Settings::instance(), SLOT(setNetworkProxyEnabled(bool)));
        connect(m_proxyTypeSelector, SIGNAL(activated(int)), this, SLOT(setNetworkProxyType(int)));
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

void SettingsDialog::showInterfacesTab() {
    if (!m_interfacesTab) {
        m_interfacesTab = new QWidget(m_stack);
        m_webInterfaceGroupBox = new QGroupBox(tr("Enable web interface"), m_interfacesTab);
        m_webInterfaceGroupBox->setCheckable(true);
        m_webInterfaceGroupBox->setChecked(Settings::webInterfaceEnabled());
        m_webInterfacePortSpinBox = new QSpinBox(m_webInterfaceGroupBox);
        m_webInterfacePortSpinBox->setMaximum(100000);
        m_webInterfacePortSpinBox->setValue(Settings::webInterfacePort());
        m_webInterfaceAuthenticationCheckBox = new QCheckBox(tr("Enable authentication"), m_webInterfaceGroupBox);
        m_webInterfaceAuthenticationCheckBox->setChecked(Settings::webInterfaceAuthenticationEnabled());
        m_webInterfaceUsernameEdit = new QLineEdit(Settings::webInterfaceUsername(), m_webInterfaceGroupBox);
        m_webInterfacePasswordEdit = new QLineEdit(Settings::webInterfacePassword(), m_webInterfaceGroupBox);
        m_webInterfacePasswordEdit->setEchoMode(QLineEdit::Password);
        
        QFormLayout *form = new QFormLayout(m_webInterfaceGroupBox);
        form->addRow(tr("&Port:"), m_webInterfacePortSpinBox);
        form->addRow(m_webInterfaceAuthenticationCheckBox);
        form->addRow(tr("&Username:"), m_webInterfaceUsernameEdit);
        form->addRow(tr("&Password:"), m_webInterfacePasswordEdit);
        
        QVBoxLayout *vbox = new QVBoxLayout(m_interfacesTab);
        vbox->addWidget(m_webInterfaceGroupBox);
        
        connect(m_webInterfaceGroupBox, SIGNAL(toggled(bool)),
                Settings::instance(), SLOT(setWebInterfaceEnabled(bool)));
        connect(m_webInterfacePortSpinBox, SIGNAL(valueChanged(int)),
                Settings::instance(), SLOT(setWebInterfacePort(int)));
        connect(m_webInterfaceAuthenticationCheckBox, SIGNAL(toggled(bool)),
                Settings::instance(), SLOT(setWebInterfaceAuthenticationEnabled(bool)));
        connect(m_webInterfaceUsernameEdit, SIGNAL(textChanged(QString)),
                Settings::instance(), SLOT(setWebInterfaceUsername(QString)));
        connect(m_webInterfacePasswordEdit, SIGNAL(textChanged(QString)),
                Settings::instance(), SLOT(setWebInterfacePassword(QString)));
        
        m_stack->addWidget(m_interfacesTab);
    }
    
    m_stack->setCurrentWidget(m_interfacesTab);
}

void SettingsDialog::setCurrentTab(int index) {
    switch (index) {
    case 0:
        showGeneralTab();
        break;
    case 1:
        showNetworkTab();
        break;
    case 2:
        showInterfacesTab();
        break;
    default:
        break;
    }
}

void SettingsDialog::setNetworkProxyType(int index) {
    Settings::setNetworkProxyType(m_proxyTypeSelector->itemData(index).toInt());
}

void SettingsDialog::showFileDialog() {
    const QString path = QFileDialog::getExistingDirectory(this, tr("Select folder"),
                                                           Settings::instance()->downloadPath());
    
    if (!path.isEmpty()) {
        m_downloadPathEdit->setText(path);
    }
}
