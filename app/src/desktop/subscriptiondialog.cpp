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

#include "subscriptiondialog.h"
#include "database.h"
#include "selectionmodel.h"
#include "subscription.h"
#include "subscriptions.h"
#include "utils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlQuery>

SubscriptionDialog::SubscriptionDialog(QWidget *parent) :
    QDialog(parent),
    m_subscriptionTypeModel(new SelectionModel(this)),
    m_enclosuresCheckBox(new QCheckBox(tr("Download enclosures"), this)),
    m_subscriptionTypeSelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QGridLayout(this)),
    m_sourceEdit(new QLineEdit(this)),
    m_sourceButton(new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), this)),
    m_requestId(-1),
    m_subscriptionId(-1)
{
    setWindowTitle(tr("Subscription properties"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    
    m_subscriptionTypeModel->append(tr("URL"), Subscription::Url);
    m_subscriptionTypeModel->append(tr("Local file"), Subscription::LocalFile);
    m_subscriptionTypeModel->append(tr("Command"), Subscription::Command);
    
    m_subscriptionTypeSelector->setModel(m_subscriptionTypeModel);
        
    m_layout->addWidget(new QLabel(tr("Subscription type") + ":", this), 0, 0);
    m_layout->addWidget(m_subscriptionTypeSelector, 0, 1, 1, 2);
    m_layout->addWidget(new QLabel(tr("Source") + ":", this), 1, 0);
    m_layout->addWidget(m_sourceEdit, 1, 1);
    m_layout->addWidget(m_sourceButton, 1, 2);
    m_layout->addWidget(m_enclosuresCheckBox, 2, 1);
    m_layout->addWidget(m_buttonBox, 3, 0, 1, 3);
    m_layout->setColumnStretch(1, 1);
    
    connect(m_subscriptionTypeSelector, SIGNAL(activated(int)), this, SLOT(setSubscriptionType(int)));
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    setSubscriptionType(Subscription::Url);
}

SubscriptionDialog::SubscriptionDialog(int subscriptionId, QWidget *parent) :
    QDialog(parent),
    m_subscriptionTypeModel(new SelectionModel(this)),
    m_enclosuresCheckBox(new QCheckBox(tr("Download enclosures"), this)),
    m_subscriptionTypeSelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QGridLayout(this)),
    m_sourceEdit(new QLineEdit(this)),
    m_sourceButton(new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), this)),
    m_requestId(Utils::createId()),
    m_subscriptionId(subscriptionId)
{
    setWindowTitle(tr("Subscription properties"));
    setAttribute(Qt::WA_DeleteOnClose, true);
    
    m_subscriptionTypeModel->append(tr("URL"), Subscription::Url);
    m_subscriptionTypeModel->append(tr("Local file"), Subscription::LocalFile);
    m_subscriptionTypeModel->append(tr("Command"), Subscription::Command);
    
    m_subscriptionTypeSelector->setModel(m_subscriptionTypeModel);
        
    m_layout->addWidget(new QLabel(tr("Subscription type") + ":", this), 0, 0);
    m_layout->addWidget(m_subscriptionTypeSelector, 0, 1, 1, 2);
    m_layout->addWidget(new QLabel(tr("Source") + ":", this), 1, 0);
    m_layout->addWidget(m_sourceEdit, 1, 1);
    m_layout->addWidget(m_sourceButton, 1, 2);
    m_layout->addWidget(m_enclosuresCheckBox, 2, 1);
    m_layout->addWidget(m_buttonBox, 3, 0, 1, 3);
    m_layout->setColumnStretch(1, 1);
        
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
    Database::fetchSubscription(m_subscriptionId, m_requestId);
}

void SubscriptionDialog::accept() {
    const int subscriptionType = m_subscriptionTypeSelector->itemData(m_subscriptionTypeSelector->currentIndex(),
                                                                      SelectionModel::ValueRole).toInt();
    
    const QString source = m_sourceEdit->text();
    const bool downloadEnclosures = m_enclosuresCheckBox->isChecked();
    
    if (m_subscriptionId == -1) {
        Subscriptions::instance()->create(source, subscriptionType, downloadEnclosures);
    }
    else {
        QVariantMap properties;
        properties["source"] = source;
        properties["downloadEnclosures"] = downloadEnclosures;
        Database::updateSubscription(m_subscriptionId, properties);
    }
    
    QDialog::accept();
}

void SubscriptionDialog::setSubscriptionType(int type) {
    m_sourceButton->setEnabled(type != Subscription::Url);
}

void SubscriptionDialog::showFileDialog() {
    const QString source = QFileDialog::getOpenFileName(this, tr("Select file"));
    
    if (!source.isEmpty()) {
        m_sourceEdit->setText(source);
    }
}

void SubscriptionDialog::onSubscriptionFetched(const QSqlQuery &query, int requestId) {
    if (requestId == m_requestId) {
        disconnect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
        
        m_sourceEdit->setText(Database::subscriptionSource(query).toString());
        m_sourceButton->setEnabled(Database::subscriptionSourceType(query) != Subscription::Url);
        m_enclosuresCheckBox->setChecked(Database::subscriptionDownloadEnclosures(query));
    }
}
