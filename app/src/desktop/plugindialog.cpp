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
#include "database.h"
#include "json.h"
#include "selectionmodel.h"
#include "subscription.h"
#include "subscriptionplugins.h"
#include "subscriptions.h"
#include "utils.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSqlQuery>
#include <QXmlStreamReader>

PluginDialog::PluginDialog(const QString &pluginName, QWidget *parent) :
    QDialog(parent),
    m_enclosuresCheckBox(new QCheckBox(tr("Download enclosures"), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_scrollArea(new QScrollArea(this)),
    m_layout(new QVBoxLayout(this)),
    m_requestId(-1),
    m_subscriptionId(-1),
    m_pluginName(pluginName)
{
    setWindowTitle(tr("Subscription properties"));
    setMinimumWidth(500);
    setAttribute(Qt::WA_DeleteOnClose, true);
    
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QScrollArea::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_layout->addWidget(m_scrollArea);
        
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    loadUi();
}

PluginDialog::PluginDialog(int subscriptionId, QWidget *parent) :
    QDialog(parent),
    m_enclosuresCheckBox(new QCheckBox(tr("Download enclosures"), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_scrollArea(new QScrollArea(this)),
    m_layout(new QVBoxLayout(this)),
    m_requestId(Utils::createId()),
    m_subscriptionId(subscriptionId)
{
    setWindowTitle(tr("Subscription properties"));
    setMinimumWidth(500);
    setAttribute(Qt::WA_DeleteOnClose, true);
    
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QScrollArea::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_layout->addWidget(m_scrollArea);
        
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    connect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
            this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
    Database::fetchSubscription(m_subscriptionId, m_requestId);
}

void PluginDialog::accept() {
    QVariantMap sourceMap;
    sourceMap["pluginName"] = m_pluginName;
    sourceMap["params"] = m_params;
    const QString source = QString::fromUtf8(QtJson::Json::serialize(sourceMap));
    const bool downloadEnclosures = m_enclosuresCheckBox->isChecked();
    
    if (m_subscriptionId == -1) {
        Subscriptions::instance()->create(source, Subscription::Plugin, downloadEnclosures);
    }
    else {
        QVariantMap properties;
        properties["source"] = source;
        properties["downloadEnclosures"] = downloadEnclosures;
        Database::updateSubscription(m_subscriptionId, properties);
    }
    
    QDialog::accept();
}

QVariant PluginDialog::initParam(const QString &key, const QVariant &defaultValue) {
    if (m_params.contains(key)) {
        return m_params.value(key);
    }
    
    m_params[key] = defaultValue;
    return defaultValue;
}

bool PluginDialog::loadUi() {
    const QString fileName = SubscriptionPlugins::filePath(m_pluginName);
    
    if (fileName.isEmpty()) {
        return false;
    }
    
    QFile file(fileName);
    
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }
    
    QXmlStreamReader reader(&file);
    QWidget *widget = new QWidget(m_scrollArea);
    QVBoxLayout *vbox = new QVBoxLayout(widget);
    
    while ((!reader.atEnd()) && (!reader.hasError())) {
        reader.readNextStartElement();
        const QStringRef name = reader.name();
        const QXmlStreamAttributes attributes = reader.attributes();
        
        if (attributes.isEmpty()) {
            continue;
        }
        
        if (name == "boolean") {
            vbox->addWidget(loadBoolean(attributes.value("title").toString(), attributes.value("name").toString(),
                                        initParam(attributes.value("name").toString(),
                                                  attributes.value("default").toString()) == "true"));
        }
        else if (name == "integer") {
            vbox->addWidget(loadInteger(attributes.value("title").toString(), attributes.value("name").toString(),
                                        qMax(0, attributes.value("min").toString().toInt()),
                                        qMax(1, attributes.value("max").toString().toInt()),
                                        qMax(1, attributes.value("step").toString().toInt()),
                                        initParam(attributes.value("name").toString(),
                                                  attributes.value("default").toString()).toInt()));
        }
        else if (name == "list") {
            reader.readNextStartElement();
            QList< QPair<QString, QString> > elements;
            
            while (reader.name() == "element") {
                const QXmlStreamAttributes elementAttributes = reader.attributes();
                
                if (!elementAttributes.isEmpty()) {
                    elements << QPair<QString, QString>(elementAttributes.value("name").toString(),
                                                        elementAttributes.value("value").toString());
                }
                
                reader.readNextStartElement();
            }
            
            vbox->addWidget(loadList(attributes.value("title").toString(), attributes.value("name").toString(),
                                     elements, initParam(attributes.value("name").toString(),
                                                         attributes.value("default").toString()).toString()));
        }
        else if (name == "text") {
            vbox->addWidget(loadText(attributes.value("title").toString(), attributes.value("name").toString(),
                                     initParam(attributes.value("name").toString(),
                                               attributes.value("default").toString()).toString()));
        }
    }
    
    vbox->addWidget(m_enclosuresCheckBox);
    vbox->addWidget(m_buttonBox);
    m_scrollArea->setWidget(widget);
    setMinimumHeight(qMin(500, widget->height() + 20));
    return true;
}

QWidget* PluginDialog::loadBoolean(const QString &title, const QString &key, bool checked) {
    QCheckBox *checkbox = new QCheckBox(this);
    checkbox->setText(title);
    checkbox->setProperty("key", key);
    checkbox->setChecked(checked);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onBooleanChanged(bool)));
    return checkbox;
}

QWidget* PluginDialog::loadInteger(const QString &title, const QString &key, int minimum, int maximum, int singleStep,
                                   int value) {
    QWidget *widget = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(widget);
    QSpinBox *spinbox = new QSpinBox(widget);
    spinbox->setProperty("key", key);
    spinbox->setMinimum(minimum);
    spinbox->setMaximum(maximum);
    spinbox->setSingleStep(singleStep);
    spinbox->setValue(value);
    hbox->addWidget(new QLabel(title + ":", widget));
    hbox->addWidget(spinbox);
    connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(onIntegerChanged(int)));
    return widget;
}

QWidget* PluginDialog::loadList(const QString &title, const QString &key,
                                const QList< QPair<QString, QString> > &elements, const QString &value) {
    QWidget *widget = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(widget);
    QComboBox *combobox = new QComboBox(widget);
    SelectionModel *model = new SelectionModel(combobox);
    
    for (int i = 0; i < elements.size(); i++) {
        model->append(elements.at(i).first, elements.at(i).second);
    }

    combobox->setProperty("key", key);    
    combobox->setModel(model);
    combobox->setCurrentIndex(model->match("value", value));
    hbox->addWidget(new QLabel(title + ":", widget));
    hbox->addWidget(combobox);
    connect(combobox, SIGNAL(activated(int)), this, SLOT(onListChanged(int)));
    return widget;
}

QWidget* PluginDialog::loadText(const QString &title, const QString &key, const QString &text) {
    QWidget *widget = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(widget);
    QLineEdit *lineedit = new QLineEdit(text, widget);
    lineedit->setProperty("key", key);
    lineedit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    hbox->addWidget(new QLabel(title + ":", widget));
    hbox->addWidget(lineedit);
    connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(onTextChanged(QString)));
    return widget;
}

void PluginDialog::onBooleanChanged(bool isChecked) {
    if (QObject *obj = sender()) {
        const QString key = obj->property("key").toString();
        
        if (!key.isEmpty()) {
            m_params[key] = isChecked;
        }
    }
}

void PluginDialog::onIntegerChanged(int currentValue) {
    if (QObject *obj = sender()) {
        const QString key = obj->property("key").toString();
        
        if (!key.isEmpty()) {
            m_params[key] = currentValue;
        }
    }
}

void PluginDialog::onListChanged(int currentIndex) {
    if (QComboBox *cb = qobject_cast<QComboBox*>(sender())) {
        const QString key = cb->property("key").toString();
        
        if (!key.isEmpty()) {
            m_params[key] = cb->itemData(currentIndex, SelectionModel::ValueRole);
        }
    }
}

void PluginDialog::onTextChanged(const QString &currentText) {
    if (QObject *obj = sender()) {
        const QString key = obj->property("key").toString();
        
        if (!key.isEmpty()) {
            m_params[key] = currentText;
        }
    }
}

void PluginDialog::onSubscriptionFetched(const QSqlQuery &query, int requestId) {
    if (requestId == m_requestId) {
        disconnect(Database::instance(), SIGNAL(subscriptionFetched(QSqlQuery, int)),
                   this, SLOT(onSubscriptionFetched(QSqlQuery, int)));
        QVariantMap source = Database::subscriptionSource(query).toMap();
        
        if (!source.isEmpty()) {
            m_pluginName = source.value("pluginName").toString();
            m_params = source.value("params").toMap();
            loadUi();
        }
    }
}
