/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>

class CategoryNameModel;
class TransferPriorityModel;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QFormLayout;
class QLineEdit;

class DownloadDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString category READ category WRITE setCategory)
    Q_PROPERTY(QString customCommand READ customCommand WRITE setCustomCommand)
    Q_PROPERTY(bool customCommandOverrideEnabled READ customCommandOverrideEnabled
               WRITE setCustomCommandOverrideEnabled)
    Q_PROPERTY(int priority READ priority WRITE setPriority)
    Q_PROPERTY(bool usePlugin READ usePlugin WRITE setUsePlugin)

public:
    explicit DownloadDialog(QWidget *parent = 0);
    explicit DownloadDialog(const QString &url, QWidget *parent = 0);

    QString category() const;
    void setCategory(const QString &category);

    QString customCommand() const;
    void setCustomCommand(const QString &command);
    bool customCommandOverrideEnabled() const;
    void setCustomCommandOverrideEnabled(bool enabled);

    int priority() const;
    void setPriority(int priority);

    QString url() const;
    void setUrl(const QString &url);

    bool usePlugin() const;
    void setUsePlugin(bool enabled);

private:
    CategoryNameModel *m_categoryModel;
    TransferPriorityModel *m_priorityModel;

    QLineEdit *m_commandEdit;

    QComboBox *m_categorySelector;
    QComboBox *m_prioritySelector;

    QCheckBox *m_commandCheckBox;
    QCheckBox *m_pluginCheckBox;

    QDialogButtonBox *m_buttonBox;

    QFormLayout *m_layout;

    QString m_url;
};

#endif // DOWNLOADDIALOG_H
