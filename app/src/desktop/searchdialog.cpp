/*
 * Copyright (C) 2015 Stuart Howarth <showarth@marxoft.co.uk>
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
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QGridLayout(this)),
    m_searchEdit(new QLineEdit(this))
{
    setWindowTitle(tr("Search all articles"));
    setMinimumWidth(400);
    setAttribute(Qt::WA_DeleteOnClose, true);
    
    m_layout->addWidget(new QLabel(tr("Query") + ":", 0, 0));
    m_layout->addWidget(m_searchEdit, 0, 1);
    m_layout->addWidget(m_buttonBox, 1, 0, 1, 2);
    m_layout->setColumnStretch(1, 1);
    
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void SearchDialog::accept() {
    const QString text = m_searchEdit->text();
    emit search(text.startsWith("WHERE ") ? text : QString("WHERE title LIKE '%%1%'").arg(text));    
    QDialog::accept();
}
