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

#include "qdatetimedialog.h"
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

QDateTimeDialog::QDateTimeDialog(QWidget *parent) :
    QDialog(parent),
    m_label(new QLabel(tr("Enter date and time"), this)),
    m_edit(new QDateTimeEdit(this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_edit);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QDateTimeDialog::QDateTimeDialog(const QDateTime &dt, QWidget *parent) :
    QDialog(parent),
    m_label(new QLabel(tr("Enter date and time"), this)),
    m_edit(new QDateTimeEdit(dt, this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_edit);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QDateTime QDateTimeDialog::dateTime() const {
    return m_edit->dateTime();
}

void QDateTimeDialog::setDateTime(const QDateTime &dt) {
    m_edit->setDateTime(dt);
}

QDateTime QDateTimeDialog::minimumDateTime() const {
    return m_edit->minimumDateTime();
}

void QDateTimeDialog::setMinimumDateTime(const QDateTime &dt) {
    m_edit->setMinimumDateTime(dt);
}

QDateTime QDateTimeDialog::maximumDateTime() const {
    return m_edit->maximumDateTime();
}

void QDateTimeDialog::setMaximumDateTime(const QDateTime &dt) {
    m_edit->setMaximumDateTime(dt);
}

QString QDateTimeDialog::labelText() const {
    return m_label->text();
}

void QDateTimeDialog::setLabelText(const QString &text) {
    m_label->setText(text);
}
