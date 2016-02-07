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

class SelectionModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QGridLayout;
class QLineEdit;
class QSqlQuery;

class SubscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubscriptionDialog(QWidget *parent = 0);
    explicit SubscriptionDialog(int subscriptionId, QWidget *parent = 0);

public Q_SLOTS:
    void accept();

private Q_SLOTS:
    void setSubscriptionType(int type);
    
    void showFileDialog();
    
    void onSubscriptionFetched(const QSqlQuery &query, int requestId);

private:
    SelectionModel *m_subscriptionTypeModel;
    
    QCheckBox *m_enclosuresCheckBox;
    
    QComboBox *m_subscriptionTypeSelector;
    
    QDialogButtonBox *m_buttonBox;
    
    QGridLayout *m_layout;
    
    QLineEdit *m_sourceEdit;
    
    QPushButton *m_sourceButton;
        
    int m_requestId;
    int m_subscriptionId;
};
    
#endif // SUBSCRIPTIONDIALOG_H
