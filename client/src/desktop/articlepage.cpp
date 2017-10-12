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

#include "articlepage.h"
#include "browserpage.h"
#include "pluginmanager.h"
#include "utils.h"
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

ArticlePage::ArticlePage(QWidget *parent) :
    Page(parent),
    m_enclosuresModel(0),
    m_headerLabel(new QLabel(this)),
    m_enclosuresLabel(0),
    m_browser(new BrowserPage(this)),
    m_enclosuresView(0),
    m_layout(new QVBoxLayout(this))
{
    m_headerLabel->setWordWrap(true);

    m_layout->addWidget(m_headerLabel);
    m_layout->addWidget(m_browser);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setStretch(3, 1);

    connect(m_headerLabel, SIGNAL(linkActivated(QString)), this, SLOT(setUrl(QString)));
    connect(m_browser, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(m_browser, SIGNAL(information(QString)), this, SIGNAL(information(QString)));
    connect(m_browser, SIGNAL(openArticleInTab(QString, QString)), this, SIGNAL(openArticleInTab(QString, QString)));
    connect(m_browser, SIGNAL(openUrlInTab(QString, QString)), this, SIGNAL(openUrlInTab(QString, QString)));
    connect(m_browser, SIGNAL(openUrlExternally(QString)), this, SIGNAL(openUrlExternally(QString)));
    connect(m_browser, SIGNAL(openUrlWithPlugin(QString)), this, SIGNAL(openUrlWithPlugin(QString)));
    connect(m_browser, SIGNAL(downloadUrl(QString)), this, SIGNAL(downloadUrl(QString)));
    connect(m_browser, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SIGNAL(showHtmlInTab(QString, QString, QString)));
    connect(m_browser, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SIGNAL(showTextInTab(QString, QString, QString)));
    connect(m_browser, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
    connect(m_browser, SIGNAL(urlChanged(QString)), this, SIGNAL(urlChanged(QString)));
}

void ArticlePage::setEnclosures(const QVariantList &enclosures) {
    if (enclosures.isEmpty()) {
        if (m_enclosuresLabel) {
            m_enclosuresLabel->hide();
        }

        if (m_enclosuresView) {
            m_enclosuresView->hide();
        }

        return;
    }

    if (!m_enclosuresLabel) {
        m_enclosuresLabel = new QLabel(tr("%1 enclosure(s)").arg(enclosures.size()), this);
        m_layout->addWidget(m_enclosuresLabel);
    }

    if (!m_enclosuresModel) {
        m_enclosuresModel = new QStandardItemModel(this);
    }

    if (!m_enclosuresView) {
        m_enclosuresView = new QTreeView(this);
        m_enclosuresView->setModel(m_enclosuresModel);
        m_enclosuresView->setSelectionBehavior(QTreeView::SelectRows);
        m_enclosuresView->setContextMenuPolicy(Qt::CustomContextMenu);
        m_enclosuresView->setEditTriggers(QTreeView::NoEditTriggers);
        m_enclosuresView->setItemsExpandable(false);
        m_enclosuresView->setIndentation(0);
        m_enclosuresView->setUniformRowHeights(true);
        m_enclosuresView->setAllColumnsShowFocus(true);
        m_enclosuresView->setHeaderHidden(true);
        m_enclosuresView->header()->setMinimumSectionSize(64);
#if QT_VERSION >= 0x050000
        m_enclosuresView->header()->setSectionResizeMode(QHeaderView::Stretch);
#else
        m_enclosuresView->header()->setResizeMode(QHeaderView::Stretch);
#endif
        m_layout->addWidget(m_enclosuresView);
        connect(m_enclosuresView, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showEnclosureContextMenu(QPoint)));
        connect(m_enclosuresView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(openEnclosureUrlInBrowser(QModelIndex)));
    }

    m_enclosuresModel->clear();

    foreach (const QVariant &enclosure, enclosures) {
        const QVariantMap map = enclosure.toMap();
        const int size = map.value("length").toLongLong();
        
        QList<QStandardItem*> items;
        items << new QStandardItem(map.value("url").toString());
        QStandardItem *typeItem = new QStandardItem(size > 0 ? QString("%1  %2").arg(Utils::formatBytes(size))
                .arg(map.value("type").toString()) : map.value("type").toString());
        typeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        items << typeItem;
        m_enclosuresModel->appendRow(items);
    }

    m_enclosuresView->setFixedHeight(m_enclosuresView->sizeHintForRow(0) * qMin(4, enclosures.size() + 1));
    m_enclosuresLabel->show();
    m_enclosuresView->show();
}

QString ArticlePage::header() const {
    return m_headerLabel->text();
}

void ArticlePage::setHeader(const QString &header) {
    m_headerLabel->setText(header);
}

QString ArticlePage::toHtml() const {
    return m_browser->toHtml();
}

void ArticlePage::setHtml(const QString &html, const QString &baseUrl) {
    m_browser->setHtml(html, baseUrl);
}

QString ArticlePage::toPlainText() const {
    return m_browser->toPlainText();
}

void ArticlePage::setText(const QString &text, const QString &baseUrl) {
    m_browser->setText(text, baseUrl);
}

QString ArticlePage::title() const {
    return m_browser->title();
}

QString ArticlePage::url() const {
    return m_browser->url();
}

void ArticlePage::setUrl(const QString &url) {
    m_browser->setUrl(url);
}

void ArticlePage::openEnclosureUrlInBrowser(const QModelIndex &index) {
    if (const QStandardItem *item = m_enclosuresModel->item(index.row(), 0)) {
        setUrl(item->text());
    }
}

void ArticlePage::showEnclosureContextMenu(const QPoint &pos) {
    const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0);

    if (!item) {
        return;
    }

    const QString url = item->text();
    const FeedPluginConfig *config = PluginManager::instance()->getConfigForEnclosure(url);
    QMenu menu(this);
    QAction *copyAction = menu.addAction(tr("&Copy URL"));
    QAction *tabAction = menu.addAction(tr("Open in &tab"));
    QAction *browserAction = menu.addAction(tr("Open in &browser"));
    QAction *externalAction = menu.addAction(tr("Open &externally"));
    QAction *pluginAction = config ? menu.addAction(tr("Open externally via &%1").arg(config->displayName())) : 0;
    QAction *downloadAction = menu.addAction(tr("Download"));

    QAction *action = menu.exec(m_enclosuresView->mapToGlobal(pos));

    if (!action) {
        return;
    }

    if (action == copyAction) {
        QApplication::clipboard()->setText(url);
    }
    else if (action == tabAction) {
        emit openUrlInTab(url, url);
    }
    else if (action == browserAction) {
        setUrl(url);
    }
    else if (action == externalAction) {
        emit openUrlExternally(url);
    }
    else if (action == pluginAction) {
        emit openUrlWithPlugin(url);
    }
    else if (action == downloadAction) {
        emit downloadUrl(url);
    }
}
