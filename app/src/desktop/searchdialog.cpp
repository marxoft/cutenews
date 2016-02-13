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

#include "searchdialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QFormLayout(this)),
    m_searchEdit(new QLineEdit(this))
{
    setWindowTitle(tr("Search all articles"));
    setMinimumWidth(400);
    
    m_layout->addRow(tr("&Query:"), m_searchEdit);
    m_layout->addWidget(m_buttonBox);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QString SearchDialog::query() const {
    const QString text = m_searchEdit->text();
    return text.startsWith("WHERE ") ? text : QString("WHERE title LIKE '%%1%'").arg(text);
}
