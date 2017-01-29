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

#ifndef SUBSCRIPTIONDIALOG_H
#define SUBSCRIPTIONDIALOG_H

#include <QDialog>
#include <QVariantMap>

class DBConnection;
class SubscriptionSourceTypeModel;
class UpdateIntervalTypeModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QLineEdit;
class QSpinBox;

class SubscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubscriptionDialog(QWidget *parent = 0);
    explicit SubscriptionDialog(const QString &subscriptionId, QWidget *parent = 0);

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void setSubscriptionType(int type);
    void setUpdateIntervalType(int type);
    
    void showFileDialog();
    
    void onSubscriptionFetched(DBConnection *connection);
    void onSubscriptionUpdated(DBConnection *connection);

private:
    SubscriptionSourceTypeModel *m_subscriptionTypeModel;
    UpdateIntervalTypeModel *m_updateIntervalModel;
    
    QCheckBox *m_enclosuresCheckBox;
    
    QComboBox *m_subscriptionTypeSelector;
    QComboBox *m_updateIntervalSelector;
    
    QDialogButtonBox *m_buttonBox;
    
    QFormLayout *m_layout;
    
    QLineEdit *m_sourceEdit;
    
    QPushButton *m_sourceButton;

    QSpinBox *m_updateIntervalSpinBox;
    
    QString m_subscriptionId;
};
    
#endif // SUBSCRIPTIONDIALOG_H
