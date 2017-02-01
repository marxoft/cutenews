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

#include "plugindialog.h"
#include "dbconnection.h"
#include "json.h"
#include "pluginmanager.h"
#include "selectionmodel.h"
#include "subscription.h"
#include "subscriptions.h"
#include "updateintervaltypemodel.h"
#include "utils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>

PluginDialog::PluginDialog(const QString &pluginId, QWidget *parent) :
    QDialog(parent),
    m_updateIntervalModel(new UpdateIntervalTypeModel(this)),
    m_enclosuresCheckBox(new QCheckBox(tr("Download &enclosures"), this)),
    m_updateIntervalSelector(new QComboBox(this)),
    m_updateIntervalSpinBox(new QSpinBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_scrollArea(new QScrollArea(this)),
    m_widget(new QWidget(m_scrollArea)),
    m_form(new QFormLayout(m_widget)),
    m_layout(new QFormLayout(this)),
    m_pluginId(pluginId)
{
    setWindowTitle(tr("Subscription properties"));
    setMinimumWidth(500);

    m_updateIntervalSelector->setModel(m_updateIntervalModel);
    
    m_scrollArea->setWidget(m_widget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QScrollArea::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_form->setContentsMargins(0, 0, 0, 0);
    
    m_layout->addRow(m_scrollArea);
    m_layout->addWidget(m_enclosuresCheckBox);
    m_layout->addRow(tr("&Update interval (0 to disable):"), m_updateIntervalSpinBox);
    m_layout->addWidget(m_updateIntervalSelector);
    m_layout->addWidget(m_buttonBox);

    connect(m_updateIntervalSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setUpdateIntervalType(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setUpdateIntervalType(0);
    loadUi();
}

PluginDialog::PluginDialog(const QString &pluginId, const QString &subscriptionId, QWidget *parent) :
    QDialog(parent),
    m_updateIntervalModel(new UpdateIntervalTypeModel(this)),
    m_enclosuresCheckBox(new QCheckBox(tr("Download &enclosures"), this)),
    m_updateIntervalSelector(new QComboBox(this)),
    m_updateIntervalSpinBox(new QSpinBox(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_scrollArea(new QScrollArea(this)),
    m_widget(new QWidget(m_scrollArea)),
    m_form(new QFormLayout(m_widget)),
    m_layout(new QFormLayout(this)),
    m_pluginId(pluginId),
    m_subscriptionId(subscriptionId)
{
    setWindowTitle(tr("Subscription properties"));
    setMinimumWidth(500);

    m_updateIntervalSelector->setModel(m_updateIntervalModel);
    
    m_scrollArea->setWidget(m_widget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QScrollArea::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_form->setContentsMargins(0, 0, 0, 0);
    
    m_layout->addRow(m_scrollArea);
    m_layout->addWidget(m_enclosuresCheckBox);
    m_layout->addRow(tr("&Update interval (0 to disable):"), m_updateIntervalSpinBox);
    m_layout->addWidget(m_updateIntervalSelector);
    m_layout->addWidget(m_buttonBox);

    connect(m_updateIntervalSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setUpdateIntervalType(int)));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(subscriptionId);
}

void PluginDialog::accept() {
    QVariantMap sourceMap;
    sourceMap["pluginId"] = m_pluginId;
    sourceMap["settings"] = m_settings;
    const QString source = QString::fromUtf8(QtJson::Json::serialize(sourceMap));
    const bool downloadEnclosures = m_enclosuresCheckBox->isChecked();

    int interval = m_updateIntervalSpinBox->value();

    if (interval > 0) {
        interval *= m_updateIntervalSelector->itemData(m_updateIntervalSelector->currentIndex(),
                                                       UpdateIntervalTypeModel::ValueRole).toInt();
    }
    
    if (m_subscriptionId.isEmpty()) {
        Subscriptions::instance()->create(source, Subscription::Plugin, downloadEnclosures, interval);
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

void PluginDialog::loadUi() {
    if (const FeedPluginConfig *config = PluginManager::instance()->getConfig(m_pluginId)) {
        foreach (const QVariant &setting, config->feedSettings()) {
            addWidget(m_form, setting.toMap());
        }
    }
}

void PluginDialog::addCheckBox(QFormLayout *layout, const QString &label, const QString &key, bool value) {
    QCheckBox *checkbox = new QCheckBox("&" + label, this);
    checkbox->setProperty("key", key);
    checkbox->setChecked(value);
    layout->addRow(checkbox);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(setBooleanValue(bool)));
}

void PluginDialog::addComboBox(QFormLayout *layout, const QString &label, const QString &key,
                                    const QVariantList &options, const QVariant &value) {
    QComboBox *combobox = new QComboBox(this);
    SelectionModel *model = new SelectionModel(combobox);
    combobox->setProperty("key", key);
    combobox->setModel(model);

    foreach (const QVariant &var, options) {
        const QVariantMap option = var.toMap();
        model->append(option.value("label").toString(), option.value("value"));
    }

    combobox->setCurrentIndex(qMax(0, combobox->findData(value)));
    layout->addRow("&" + label + ":", combobox);
    connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(setListValue(int)));
}

void PluginDialog::addGroupBox(QFormLayout *layout, const QString &label, const QString &key,
                                    const QVariantList &settings) {
    QGroupBox *groupbox = new QGroupBox("&" + label, this);
    QFormLayout *form = new QFormLayout(groupbox);

    foreach (const QVariant &setting, settings) {
        addWidget(form, setting.toMap(), key);
    }

    layout->addRow(groupbox);
}

void PluginDialog::addLineEdit(QFormLayout *layout, const QString &label, const QString &key,
                                    const QString &value, bool isPassword) {
    QLineEdit *edit = new QLineEdit(value, this);
    edit->setProperty("key", key);

    if (isPassword) {
        edit->setEchoMode(QLineEdit::Password);
    }

    layout->addRow("&" + label + ":", edit);
    connect(edit, SIGNAL(textChanged(QString)), this, SLOT(setTextValue(QString)));
}

void PluginDialog::addSpinBox(QFormLayout *layout, const QString &label, const QString &key, int minimum,
                                   int maximum, int step, int value) {
    QSpinBox *spinbox = new QSpinBox(this);
    spinbox->setProperty("key", key);
    spinbox->setMinimum(minimum);
    spinbox->setMaximum(maximum);
    spinbox->setSingleStep(step);
    spinbox->setValue(value);
    layout->addRow("&" + label + ":", spinbox);
    connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(setIntegerValue(int)));
}

void PluginDialog::addWidget(QFormLayout *layout, const QVariantMap &setting, const QString &group) {
    QString key = setting.value("key").toString();

    if (key.isEmpty()) {
        return;
    }

    if (!group.isEmpty()) {
        key.prepend("/");
        key.prepend(group);
    }

    const QString type = setting.value("type").toString();
    const QVariant value = m_settings.value(key, setting.value("value"));

    if (type == "boolean") {
        addCheckBox(layout, setting.value("label").toString(), key, value.toBool());
    }
    else if (type == "group") {
        addGroupBox(layout, setting.value("label").toString(), key, setting.value("settings").toList());
    }
    else if (type == "integer") {
        addSpinBox(layout, setting.value("label").toString(), key, setting.value("minimum", 0).toInt(),
                   setting.value("maximum", 100).toInt(), setting.value("step", 1).toInt(), value.toInt());
    }
    else if (type == "list") {
        addComboBox(layout, setting.value("label").toString(), key, setting.value("options").toList(), value);
    }
    else if (type == "password") {
        addLineEdit(layout, setting.value("label").toString(), key, value.toString(), true);
    }
    else if (type == "text") {
        addLineEdit(layout, setting.value("label").toString(), key, value.toString());
    }
}

void PluginDialog::setBooleanValue(bool value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginDialog::setIntegerValue(int value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginDialog::setListValue(int value) {
    if (const QComboBox *combobox = qobject_cast<QComboBox*>(sender())) {
        m_settings[combobox->property("key").toString()] = combobox->itemData(value);
    }
}

void PluginDialog::setTextValue(const QString &value) {
    if (const QObject *obj = sender()) {
        m_settings[obj->property("key").toString()] = value;
    }
}

void PluginDialog::setUpdateIntervalType(int type) {
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

void PluginDialog::onSubscriptionFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        const QVariantMap result = connection->result().toMap();
        
        if (!result.isEmpty()) {
            m_enclosuresCheckBox->setChecked(result.value("downloadEnclosures").toBool());
            const QVariantMap source = result.value("source").toMap();
            
            if (!source.isEmpty()) {
                m_pluginId = source.value("pluginId").toString();
                m_settings = source.value("settings").toMap();
                loadUi();

                const int interval = result.value("updateInterval", 0).toInt();
                
                if (interval > 0) {
                    for (int i = m_updateIntervalSelector->count() - 1; i >= 0; i--) {
                        const int value = m_updateIntervalSelector->itemData(i, UpdateIntervalTypeModel::ValueRole)
                            .toInt();
                        
                        if ((value > 0) && (interval % value == 0)) {
                            m_updateIntervalSpinBox->setValue(interval / value);
                            m_updateIntervalSelector->setCurrentIndex(i);
                            break;
                        }
                    }
                }
            }
        }
    }
    
    connection->deleteLater();
}

void PluginDialog::onSubscriptionUpdated(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        QDialog::accept();
    }
    
    connection->deleteLater();
}
