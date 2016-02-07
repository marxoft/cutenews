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

class QCheckBox;
class QDialogButtonBox;
class QScrollArea;
class QSqlQuery;
class QVBoxLayout;

class PluginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginDialog(const QString &pluginName, QWidget *parent = 0);
    explicit PluginDialog(int subscriptionId, QWidget *parent = 0);

public Q_SLOTS:
    void accept();

private:
    QVariant initParam(const QString &key, const QVariant &value);
    
    bool loadUi();
    
    QWidget* loadBoolean(const QString &title, const QString &key, bool checked);
    QWidget* loadInteger(const QString &title, const QString &key, int minimum, int maximum, int singleStep, int value);
    QWidget* loadList(const QString &title, const QString &key, const QList< QPair<QString, QString> > &elements,
                      const QString &value);
    QWidget* loadText(const QString &title, const QString &key, const QString &text);

private Q_SLOTS:            
    void onBooleanChanged(bool isChecked);
    void onIntegerChanged(int currentValue);
    void onListChanged(int currentIndex);
    void onTextChanged(const QString &currentText);
    
    void onSubscriptionFetched(const QSqlQuery &query, int requestId);

private:    
    QCheckBox *m_enclosuresCheckBox;
        
    QDialogButtonBox *m_buttonBox;
            
    QScrollArea *m_scrollArea;
        
    QVBoxLayout *m_layout;
        
    int m_requestId;
    int m_subscriptionId;
    
    QString m_pluginName;
    
    QVariantMap m_params;
};
    
#endif // PLUGINDIALOG_H
