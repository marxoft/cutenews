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

#ifndef QDATETIMEDIALOG_H
#define QDATETIMEDIALOG_H

#include <QDialog>
#include <QDateTime>

class QDateTimeEdit;
class QDialogButtonBox;
class QLabel;
class QVBoxLayout;

class QDateTimeDialog : public QDialog
{
    Q_OBJECT
    
    Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime)
    Q_PROPERTY(QDateTime minimumDateTime READ minimumDateTime WRITE setMinimumDateTime)
    Q_PROPERTY(QDateTime maximumDateTime READ maximumDateTime WRITE setMaximumDateTime)
    Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)
    
public:
    explicit QDateTimeDialog(QWidget *parent = 0);
    explicit QDateTimeDialog(const QDateTime &dt, QWidget *parent = 0);
    
    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dt);
    
    QDateTime minimumDateTime() const;
    void setMinimumDateTime(const QDateTime &dt);
    
    QDateTime maximumDateTime() const;
    void setMaximumDateTime(const QDateTime &dt);
    
    QString labelText() const;
    void setLabelText(const QString &text);

private:
    QLabel *m_label;
    
    QDateTimeEdit *m_edit;
    
    QDialogButtonBox *m_buttonBox;
    
    QVBoxLayout *m_layout;
};

#endif // QDATETIMEDIALOG_H
