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

#include "subscriptiondialog.h"
#include "dbconnection.h"
#include "subscription.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "updateintervaltypemodel.h"
#include "utils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

SubscriptionDialog::SubscriptionDialog(QWidget *parent) :
    QDialog(parent),
    m_subscriptionTypeModel(new SubscriptionSourceTypeModel(this)),
    m_updateIntervalModel(new UpdateIntervalTypeModel(this)),
    m_enclosuresCheckBox(new QCheckBox(tr("Download &enclosures"), this)),
    m_subscriptionTypeSelector(new QComboBox(this)),
    m_updateIntervalSelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this)),
    m_sourceEdit(new QLineEdit(this)),
    m_sourceButton(new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), this)),
    m_updateIntervalSpinBox(new QSpinBox(this))
{
    setWindowTitle(tr("Subscription properties"));
        
    m_subscriptionTypeSelector->setModel(m_subscriptionTypeModel);

    m_updateIntervalSelector->setModel(m_updateIntervalModel);
            
    m_layout->addRow(tr("Subscription &type:"), m_subscriptionTypeSelector);
    m_layout->addRow(tr("&Source:"), m_sourceEdit);
    m_layout->addWidget(m_sourceButton);
    m_layout->addWidget(m_enclosuresCheckBox);
    m_layout->addRow(tr("&Update interval:"), m_updateIntervalSpinBox);
    m_layout->addWidget(m_updateIntervalSelector);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_subscriptionTypeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setSubscriptionType(int)));
    connect(m_updateIntervalSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setUpdateIntervalType(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    setSubscriptionType(Subscription::Url);
    setUpdateIntervalType(0);
}

SubscriptionDialog::SubscriptionDialog(const QString &subscriptionId, QWidget *parent) :
    QDialog(parent),
    m_subscriptionTypeModel(new SubscriptionSourceTypeModel(this)),
    m_updateIntervalModel(new UpdateIntervalTypeModel(this)),
    m_enclosuresCheckBox(new QCheckBox(tr("Download &enclosures"), this)),
    m_subscriptionTypeSelector(new QComboBox(this)),
    m_updateIntervalSelector(new QComboBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this)),
    m_sourceEdit(new QLineEdit(this)),
    m_sourceButton(new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"), this)),
    m_updateIntervalSpinBox(new QSpinBox(this)),
    m_subscriptionId(subscriptionId)
{
    setWindowTitle(tr("Subscription properties"));
        
    m_subscriptionTypeSelector->setModel(m_subscriptionTypeModel);

    m_updateIntervalSelector->setModel(m_updateIntervalModel);
            
    m_layout->addRow(tr("Subscription &type:"), m_subscriptionTypeSelector);
    m_layout->addRow(tr("&Source:"), m_sourceEdit);
    m_layout->addWidget(m_sourceButton);
    m_layout->addWidget(m_enclosuresCheckBox);
    m_layout->addRow(tr("&Update interval:"), m_updateIntervalSpinBox);
    m_layout->addWidget(m_updateIntervalSelector);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_subscriptionTypeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setSubscriptionType(int)));
    connect(m_updateIntervalSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setUpdateIntervalType(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        
    DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->exec(QString("SELECT source, sourceType, downloadEnclosures, updateInterval FROM subscriptions WHERE id = '%1'").arg(subscriptionId));
}

void SubscriptionDialog::accept() {
    const int subscriptionType = m_subscriptionTypeSelector->itemData(m_subscriptionTypeSelector->currentIndex(),
                                                                      SubscriptionSourceTypeModel::ValueRole).toInt();
    
    const QString source = m_sourceEdit->text();
    const bool downloadEnclosures = m_enclosuresCheckBox->isChecked();

    int interval = m_updateIntervalSpinBox->value();

    if (interval > 0) {
        interval *= m_updateIntervalSelector->itemData(m_updateIntervalSelector->currentIndex(),
                                                       UpdateIntervalTypeModel::ValueRole).toInt();
    }
    
    if (m_subscriptionId.isEmpty()) {
        Subscriptions::instance()->create(source, subscriptionType, downloadEnclosures, interval);
        QDialog::accept();
    }
    else {
        QVariantMap properties;
        properties["source"] = source;
        properties["downloadEnclosures"] = downloadEnclosures;
        properties["updateInterval"] = interval;
        DBConnection::connection(this, SLOT(onSubscriptionUpdated(DBConnection*)))->updateSubscription(m_subscriptionId,
                                                                                                       properties);
    }    
}

void SubscriptionDialog::setSubscriptionType(int type) {
    m_sourceButton->setEnabled(type != Subscription::Url);
}

void SubscriptionDialog::setUpdateIntervalType(int type) {
    switch (type) {
    case 0:
        m_updateIntervalSpinBox->setRange(0, 1440);
        m_updateIntervalSpinBox->setSingleStep(5);
        break;
    case 1:
        m_updateIntervalSpinBox->setRange(0, 48);
        m_updateIntervalSpinBox->setSingleStep(1);
        break;
    case 2:
        m_updateIntervalSpinBox->setRange(0, 30);
        m_updateIntervalSpinBox->setSingleStep(1);
        break;
    default:
        break;
    }
}

void SubscriptionDialog::showFileDialog() {
    const QString source = QFileDialog::getOpenFileName(this, tr("Select file"));
    
    if (!source.isEmpty()) {
        m_sourceEdit->setText(source);
    }
}

void SubscriptionDialog::onSubscriptionFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        if (connection->nextRecord()) {
            m_sourceEdit->setText(connection->value(0).toString());
            m_sourceButton->setEnabled(connection->value(1) != Subscription::Url);
            m_enclosuresCheckBox->setChecked(connection->value(2).toBool());

            const int interval = connection->value(3).toInt();

            if (interval > 0) {
               for (int i = m_updateIntervalSelector->count() - 1; i >= 0; i--) {
                   const int value = m_updateIntervalSelector->itemData(i, UpdateIntervalTypeModel::ValueRole).toInt();
                   
                   if ((value > 0) && (interval % value == 0)) {
                       m_updateIntervalSpinBox->setValue(interval / value);
                       m_updateIntervalSelector->setCurrentIndex(i);
                       break;
                   }
               }
           }
        }
    }
    else {
        QMessageBox::critical(this, tr("Database error"), connection->errorString());
    }
    
    connection->deleteLater();
}

void SubscriptionDialog::onSubscriptionUpdated(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        connection->deleteLater();
        QDialog::accept();
    }
    else {
        QMessageBox::critical(this, tr("Database error"), connection->errorString());
        connection->deleteLater();
    }
}
