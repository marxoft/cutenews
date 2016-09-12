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

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>
#include <QVariantMap>

class DBConnection;
class UpdateIntervalTypeModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QScrollArea;
class QSpinBox;

class PluginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginDialog(const QString &pluginId, QWidget *parent = 0);
    explicit PluginDialog(const QString &pluginId, const QString &subscriptionId, QWidget *parent = 0);

public Q_SLOTS:
    virtual void accept();

private Q_SLOTS:    
    void setBooleanValue(bool value);
    void setIntegerValue(int value);
    void setListValue(int value);
    void setTextValue(const QString &value);

    void setUpdateIntervalType(int type);
    
    void onSubscriptionFetched(DBConnection *connection);
    void onSubscriptionUpdated(DBConnection *connection);

private:
    void loadUi();
    
    void addCheckBox(QFormLayout *layout, const QString &label, const QString &key, bool value);
    void addComboBox(QFormLayout *layout, const QString &label, const QString &key, const QVariantList &options,
                     const QVariant &value);
    void addGroupBox(QFormLayout *layout, const QString &label, const QString &key, const QVariantList &settings);
    void addLineEdit(QFormLayout *layout, const QString &label, const QString &key, const QString &value,
                     bool isPassword = false);
    void addSpinBox(QFormLayout *layout, const QString &label, const QString &key, int minimum, int maximum,
                    int step, int value);
    void addWidget(QFormLayout *layout, const QVariantMap &setting, const QString &group = QString());

    UpdateIntervalTypeModel *m_updateIntervalModel;
    
    QCheckBox *m_enclosuresCheckBox;

    QComboBox *m_updateIntervalSelector;

    QSpinBox *m_updateIntervalSpinBox;
        
    QDialogButtonBox *m_buttonBox;
            
    QScrollArea *m_scrollArea;
    QWidget *m_widget;
        
    QFormLayout *m_form;
    QFormLayout *m_layout;
        
    QString m_pluginId;
    QString m_subscriptionId;
    
    QVariantMap m_settings;
};
    
#endif // PLUGINDIALOG_H
