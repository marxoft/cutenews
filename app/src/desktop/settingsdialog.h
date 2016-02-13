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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class NetworkProxyTypeModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTabBar;
class QVBoxLayout;

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = 0);

public Q_SLOTS:
    void accept();
    
private:
    void showGeneralTab();
    void showNetworkTab();
    
private Q_SLOTS:
    void setCurrentTab(int index);
    
    void setNetworkProxyType(int index);
    
    void showFileDialog();
        
private:
    NetworkProxyTypeModel *m_proxyTypeModel;
    
    QCheckBox *m_downloadsCheckBox;
    
    QComboBox *m_proxyTypeSelector;
    
    QDialogButtonBox *m_buttonBox;
    
    QGroupBox *m_proxyGroupBox;
    
    QLineEdit *m_downloadPathEdit;
    QLineEdit *m_proxyHostEdit;
    QLineEdit *m_proxyUsernameEdit;
    QLineEdit *m_proxyPasswordEdit;
    
    QPushButton *m_downloadPathButton;
    
    QSpinBox *m_downloadsSpinBox;
    QSpinBox *m_proxyPortSpinBox;
    
    QStackedWidget *m_stack;
    
    QTabBar *m_tabs;
    
    QWidget *m_generalTab;
    QWidget *m_networkTab;
    
    QVBoxLayout *m_layout;
};
    
#endif // SETTINGSDIALOG_H
