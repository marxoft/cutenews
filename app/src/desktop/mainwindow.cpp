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

#include "mainwindow.h"
#include "articlemodel.h"
#include "browser.h"
#include "database.h"
#include "downloadsview.h"
#include "plugindialog.h"
#include "searchdialog.h"
#include "settingsdialog.h"
#include "subscription.h"
#include "subscriptiondialog.h"
#include "subscriptionmodel.h"
#include "subscriptionplugins.h"
#include "transfers.h"
#include "urlopenermodel.h"
#include "utils.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTabBar>
#include <QTreeView>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_subscriptionsModel(new SubscriptionModel(this)),
    m_articlesModel(new ArticleModel(this)),
    m_articlesProxyModel(new QSortFilterProxyModel(this)),
    m_enclosuresModel(new QStandardItemModel(this)),
    m_urlOpenerModel(new UrlOpenerModel(this)),
    m_subscriptionsMenu(new QMenu(tr("&Subscriptions"), this)),
    m_newSubscriptionMenu(new QMenu(tr("New source"), this)),
    m_subscriptionMenu(new QMenu(tr("&Subscription"), this)),
    m_articleMenu(new QMenu(tr("&Article"), this)),
    m_articleContextMenu(new QMenu(this)),
    m_enclosureContextMenu(new QMenu(this)),
    m_viewMenu(new QMenu(tr("&View"), this)),
    m_toolsMenu(new QMenu(tr("&Tools"), this)),
    m_toolBar(new QToolBar(this)),
    m_updateAllSubscriptionsAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Update all"), this)),
    m_cancelSubscriptionUpdatesAction(new QAction(QIcon::fromTheme("dialog-cancel"), tr("&Cancel updates"), this)),
    m_markAllSubscriptionsReadAction(new QAction(QIcon::fromTheme("mail-mark-read"), tr("Mark all &read"), this)),
    m_newSubscriptionAction(new QAction(QIcon::fromTheme("list-add"), tr("&New subscription"), this)),
    m_importSubscriptionsAction(new QAction(QIcon::fromTheme("document-open"), tr("&Import from OPML"), this)),
    m_quitAction(new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this)),
    m_updateSubscriptionAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Update"), this)),
    m_markSubscriptionReadAction(new QAction(QIcon::fromTheme("mail-mark-read"), tr("Mark all &read"), this)),
    m_deleteSubscriptionAction(new QAction(QIcon::fromTheme("edit-delete"), tr("Un&subscribe"), this)),
    m_subscriptionPropertiesAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Properties"), this)),
    m_nextUnreadArticleAction(new QAction(QIcon::fromTheme("go-jump"), tr("Next &unread article"), this)),
    m_nextArticleAction(new QAction(QIcon::fromTheme("go-next"), tr("&Next article"), this)),
    m_previousArticleAction(new QAction(QIcon::fromTheme("go-previous"), tr("&Previous article"), this)),
    m_toggleArticleReadAction(new QAction(QIcon::fromTheme("mail-mark-read"), tr("Toggle &read status"), this)),
    m_toggleArticleFavouriteAction(new QAction(QIcon::fromTheme("mail-mark-important"), tr("Toggle &favourite status"), this)),
    m_deleteArticleAction(new QAction(QIcon::fromTheme("edit-delete"), tr("&Delete"), this)),
    m_copyArticleUrlAction(new QAction(tr("&Copy URL"), this)),
    m_openArticleInTabAction(new QAction(tr("Open in &tab"), this)),
    m_openArticleInBrowserAction(new QAction(tr("Open in &browser"), this)),
    m_openArticleExternallyAction(new QAction(tr("Open &externally"), this)),
    m_copyEnclosureUrlAction(new QAction(tr("&Copy URL"), this)),
    m_openEnclosureInTabAction(new QAction(tr("Open in &tab"), this)),
    m_openEnclosureInBrowserAction(new QAction(tr("Open in &browser"), this)),
    m_openEnclosureExternallyAction(new QAction(tr("Open &externally"), this)),
    m_downloadEnclosureAction(new QAction(tr("&Download"), this)),
    m_downloadsAction(new QAction(QIcon::fromTheme("folder-download"), tr("Show &downloads"), this)),
    m_closeTabAction(new QAction(QIcon::fromTheme("view-close"), tr("Close &tab"), this)),
    m_searchAction(new QAction(QIcon::fromTheme("edit-find"), tr("&Search all articles"), this)),
    m_settingsAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Preferences"), this)),
    m_horizontalSplitter(new QSplitter(Qt::Horizontal, this)),
    m_verticalSplitter(new QSplitter(Qt::Vertical, this)),
    m_stack(new QStackedWidget(this)),
    m_tabs(new QTabBar(this)),
    m_subscriptionsView(new QTreeView(this)),
    m_articlesView(new QTreeView(this)),
    m_enclosuresView(new QTreeView(this)),
    m_infoLabel(new QLabel(this)),
    m_enclosuresLabel(new QLabel(this)),
    m_browser(new Browser(this)),
    m_tabsContainer(new QWidget(this)),
    m_articleContainer(new QWidget(this)),
    m_tabsLayout(new QVBoxLayout(m_tabsContainer)),
    m_articleLayout(new QVBoxLayout(m_articleContainer)),
    m_downloadsTab(0)
{
    setWindowTitle("cuteNews");
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setCentralWidget(m_horizontalSplitter);
    addToolBar(Qt::TopToolBarArea, m_toolBar);
    
    menuBar()->addMenu(m_subscriptionsMenu);
    menuBar()->addMenu(m_subscriptionMenu);
    menuBar()->addMenu(m_articleMenu);
    menuBar()->addMenu(m_viewMenu);
    menuBar()->addMenu(m_toolsMenu);
    
    m_articlesProxyModel->setSourceModel(m_articlesModel);
    m_articlesProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_articlesProxyModel->setSortRole(ArticleModel::SortRole);
        
    m_subscriptionsMenu->addAction(m_updateAllSubscriptionsAction);
    m_subscriptionsMenu->addAction(m_cancelSubscriptionUpdatesAction);
    m_subscriptionsMenu->addAction(m_markAllSubscriptionsReadAction);
    m_subscriptionsMenu->addAction(m_newSubscriptionAction);
    m_subscriptionsMenu->addMenu(m_newSubscriptionMenu);
    m_subscriptionsMenu->addAction(m_importSubscriptionsAction);
    m_subscriptionsMenu->addAction(m_quitAction);
    
    foreach (QString name, SubscriptionPlugins::pluginNames()) {
        m_newSubscriptionMenu->addAction(name)->setData(Subscription::Plugin);
    }
    
    m_subscriptionMenu->addAction(m_updateSubscriptionAction);
    m_subscriptionMenu->addAction(m_markSubscriptionReadAction);
    m_subscriptionMenu->addAction(m_deleteSubscriptionAction);
    m_subscriptionMenu->addSeparator();
    m_subscriptionMenu->addAction(m_subscriptionPropertiesAction);
    
    m_articleMenu->addAction(m_nextUnreadArticleAction);
    m_articleMenu->addAction(m_nextArticleAction);
    m_articleMenu->addAction(m_previousArticleAction);
    m_articleMenu->addAction(m_toggleArticleReadAction);
    m_articleMenu->addAction(m_toggleArticleFavouriteAction);
    m_articleMenu->addAction(m_deleteArticleAction);
    m_articleMenu->addSeparator();
    m_articleMenu->addAction(m_copyArticleUrlAction);
    m_articleMenu->addAction(m_openArticleInTabAction);
    m_articleMenu->addAction(m_openArticleInBrowserAction);
    m_articleMenu->addAction(m_openArticleExternallyAction);
    
    m_articleContextMenu->addAction(m_toggleArticleReadAction);
    m_articleContextMenu->addAction(m_toggleArticleFavouriteAction);
    m_articleContextMenu->addAction(m_deleteArticleAction);
    m_articleContextMenu->addSeparator();
    m_articleContextMenu->addAction(m_copyArticleUrlAction);
    m_articleContextMenu->addAction(m_openArticleInTabAction);
    m_articleContextMenu->addAction(m_openArticleInBrowserAction);
    m_articleContextMenu->addAction(m_openArticleExternallyAction);
    
    m_enclosureContextMenu->addAction(m_copyEnclosureUrlAction);
    m_enclosureContextMenu->addAction(m_openEnclosureInTabAction);
    m_enclosureContextMenu->addAction(m_openEnclosureInBrowserAction);
    m_enclosureContextMenu->addAction(m_openEnclosureExternallyAction);
    m_enclosureContextMenu->addAction(m_downloadEnclosureAction);
    
    m_viewMenu->addAction(m_downloadsAction);
    m_viewMenu->addAction(m_closeTabAction);
    
    m_toolsMenu->addAction(m_searchAction);
    m_toolsMenu->addAction(m_settingsAction);

    m_toolBar->setWindowTitle(tr("Main toolbar"));
    m_toolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_toolBar->setMovable(false);
    m_toolBar->addAction(m_newSubscriptionAction);
    m_toolBar->addAction(m_markSubscriptionReadAction);
    m_toolBar->addAction(m_previousArticleAction);
    m_toolBar->addAction(m_nextArticleAction);
    m_toolBar->addAction(m_nextUnreadArticleAction);
    m_toolBar->addAction(m_updateAllSubscriptionsAction);
    m_toolBar->addAction(m_cancelSubscriptionUpdatesAction);
    m_toolBar->addAction(m_searchAction);
    
    m_updateAllSubscriptionsAction->setShortcut(QKeySequence(tr("Ctrl+U")));
    m_cancelSubscriptionUpdatesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+U")));
    m_importSubscriptionsAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    m_quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));
    
    m_markSubscriptionReadAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    
    m_nextUnreadArticleAction->setShortcut(QKeySequence(tr("Ctrl+N")));
    m_nextArticleAction->setPriority(QAction::LowPriority);
    m_previousArticleAction->setShortcut(QKeySequence(tr("Ctrl+Shift+N")));
    m_previousArticleAction->setPriority(QAction::LowPriority);
    m_toggleArticleReadAction->setShortcut(QKeySequence(tr("Ctrl+M")));
    m_toggleArticleFavouriteAction->setShortcut(QKeySequence(tr("Ctrl+T")));
    
    m_downloadsAction->setShortcut(QKeySequence(tr("Ctrl+D")));
    m_closeTabAction->setShortcut(QKeySequence(tr("Ctrl+W")));
    m_closeTabAction->setEnabled(false);
    
    m_searchAction->setShortcut(QKeySequence(tr("Ctrl+F")));
    m_settingsAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    
    m_horizontalSplitter->addWidget(m_subscriptionsView);
    m_horizontalSplitter->addWidget(m_tabsContainer);
    m_horizontalSplitter->setStretchFactor(1, 1);
            
    m_verticalSplitter->addWidget(m_articlesView);
    m_verticalSplitter->addWidget(m_articleContainer);
    m_verticalSplitter->setStretchFactor(1, 1);
    
    m_stack->addWidget(m_verticalSplitter);
    
    m_tabs->setTabsClosable(true);
    m_tabs->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    m_tabs->setExpanding(false);
    m_tabs->addTab(tr("Articles"));
    m_tabs->hide();
    
    if (QWidget *button = m_tabs->tabButton(0, QTabBar::RightSide)) {
        button->hide();
    }
        
    m_subscriptionsView->setModel(m_subscriptionsModel);
    m_subscriptionsView->setSelectionBehavior(QTreeView::SelectRows);
    m_subscriptionsView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_subscriptionsView->setEditTriggers(QTreeView::NoEditTriggers);
    m_subscriptionsView->setItemsExpandable(false);
    m_subscriptionsView->setIndentation(0);
    m_subscriptionsView->setUniformRowHeights(true);
    m_subscriptionsView->setAllColumnsShowFocus(true);
    m_subscriptionsView->setHeaderHidden(true);
    m_subscriptionsView->header()->setMinimumSectionSize(24);
    m_subscriptionsView->header()->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    m_subscriptionsView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_subscriptionsView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
    m_subscriptionsView->header()->setResizeMode(0, QHeaderView::Stretch);
    m_subscriptionsView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif
    
    m_articlesView->setModel(m_articlesProxyModel);
    m_articlesView->setSelectionBehavior(QTreeView::SelectRows);
    m_articlesView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_articlesView->setEditTriggers(QTreeView::NoEditTriggers);
    m_articlesView->setItemsExpandable(false);
    m_articlesView->setIndentation(0);
    m_articlesView->setUniformRowHeights(true);
    m_articlesView->setAllColumnsShowFocus(true);
    m_articlesView->setSortingEnabled(true);
    m_articlesView->header()->setSortIndicator(1, Qt::DescendingOrder);
    m_articlesView->header()->setMinimumSectionSize(24);
    m_articlesView->header()->resizeSection(0, 24);
#if QT_VERSION >= 0x050000
    m_articlesView->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_articlesView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
#else
    m_articlesView->header()->setResizeMode(0, QHeaderView::Fixed);
    m_articlesView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
#endif
    
    m_enclosuresView->setModel(m_enclosuresModel);
    m_enclosuresView->setSelectionBehavior(QTreeView::SelectRows);
    m_enclosuresView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_enclosuresView->setEditTriggers(QTreeView::NoEditTriggers);
    m_enclosuresView->setItemsExpandable(false);
    m_enclosuresView->setIndentation(0);
    m_enclosuresView->setUniformRowHeights(true);
    m_enclosuresView->setAllColumnsShowFocus(true);
    m_enclosuresView->setHeaderHidden(true);
    m_enclosuresView->hide();
    m_enclosuresView->header()->setMinimumSectionSize(64);
#if QT_VERSION >= 0x050000
    m_enclosuresView->header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    m_enclosuresView->header()->setResizeMode(QHeaderView::Stretch);
#endif

    m_infoLabel->setWordWrap(true);
    
    m_enclosuresLabel->hide();
    
    m_tabsLayout->addWidget(m_tabs);
    m_tabsLayout->addWidget(m_stack);
    m_tabsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabsLayout->setStretch(1, 1);
        
    m_articleLayout->addWidget(m_infoLabel);
    m_articleLayout->addWidget(m_browser);
    m_articleLayout->addWidget(m_enclosuresLabel);
    m_articleLayout->addWidget(m_enclosuresView);
    m_articleLayout->setContentsMargins(0, 0, 0, 0);
    m_articleLayout->setStretch(1, 1);
    
    connect(Subscriptions::instance(), SIGNAL(statusChanged(Subscriptions::Status)),
            this, SLOT(onSubscriptionsStatusChanged(Subscriptions::Status)));
    connect(Subscriptions::instance(), SIGNAL(statusTextChanged(QString)), statusBar(), SLOT(showMessage(QString)));
    
    connect(m_subscriptionsModel, SIGNAL(countChanged(int)), this, SLOT(onSubscriptionsCountChanged(int)));
    connect(m_articlesModel, SIGNAL(countChanged(int)), this, SLOT(onArticlesCountChanged(int)));
    
    connect(m_newSubscriptionMenu, SIGNAL(triggered(QAction*)), this, SLOT(newSubscriptionRequested(QAction*)));
    
    connect(m_updateAllSubscriptionsAction, SIGNAL(triggered()), Subscriptions::instance(), SLOT(updateAll()));
    connect(m_cancelSubscriptionUpdatesAction, SIGNAL(triggered()), Subscriptions::instance(), SLOT(cancel()));
    connect(m_markAllSubscriptionsReadAction, SIGNAL(triggered()), this, SLOT(markAllSubscriptionsRead()));
    connect(m_newSubscriptionAction, SIGNAL(triggered()), this, SLOT(newSubscriptionRequested()));
    connect(m_importSubscriptionsAction, SIGNAL(triggered()), this, SLOT(importSubscriptions()));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    connect(m_updateSubscriptionAction, SIGNAL(triggered()), this, SLOT(updateCurrentSubscription()));
    connect(m_markSubscriptionReadAction, SIGNAL(triggered()), this, SLOT(markCurrentSubscriptionRead()));
    connect(m_deleteSubscriptionAction, SIGNAL(triggered()), this, SLOT(deleteCurrentSubscription()));
    connect(m_subscriptionPropertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentSubscriptionProperties()));
    
    connect(m_nextUnreadArticleAction, SIGNAL(triggered()), this, SLOT(nextUnreadArticle()));
    connect(m_nextArticleAction, SIGNAL(triggered()), this, SLOT(nextArticle()));
    connect(m_previousArticleAction, SIGNAL(triggered()), this, SLOT(previousArticle()));
    connect(m_toggleArticleReadAction, SIGNAL(triggered()), this, SLOT(toggleCurrentArticleRead()));
    connect(m_toggleArticleFavouriteAction, SIGNAL(triggered()), this, SLOT(toggleCurrentArticleFavourite()));
    connect(m_deleteArticleAction, SIGNAL(triggered()), this, SLOT(deleteCurrentArticle()));
    connect(m_copyArticleUrlAction, SIGNAL(triggered()), this, SLOT(copyCurrentArticleUrl()));
    connect(m_openArticleInTabAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleInTab()));
    connect(m_openArticleInBrowserAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleInBrowser()));
    connect(m_openArticleExternallyAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleExternally()));
    
    connect(m_copyEnclosureUrlAction, SIGNAL(triggered()), this, SLOT(copyCurrentEnclosureUrl()));
    connect(m_openEnclosureInTabAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureInTab()));
    connect(m_openEnclosureInBrowserAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureInBrowser()));
    connect(m_openEnclosureExternallyAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureExternally()));
    connect(m_downloadEnclosureAction, SIGNAL(triggered()), this, SLOT(downloadCurrentEnclosure()));
    
    connect(m_downloadsAction, SIGNAL(triggered()), this, SLOT(showDownloadsTab()));
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));
    
    connect(m_searchAction, SIGNAL(triggered()), this, SLOT(showSearchDialog()));
    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    
    connect(m_subscriptionsView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showSubscriptionContextMenu(QPoint)));
    connect(m_subscriptionsView, SIGNAL(activated(QModelIndex)), this, SLOT(setCurrentSubscription(QModelIndex)));
    
    connect(m_articlesView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showArticleContextMenu(QPoint)));
    connect(m_articlesView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openCurrentArticleInBrowser()));
    connect(m_articlesView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(setCurrentArticle(QModelIndex)));
    
    connect(m_enclosuresView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showEnclosureContextMenu(QPoint)));
    connect(m_enclosuresView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openCurrentEnclosureInBrowser()));

    connect(m_browser, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(m_browser, SIGNAL(openUrlInTab(QString)), this, SLOT(openUrlInTab(QString)));
    
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));
    connect(m_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    
    connect(m_infoLabel, SIGNAL(linkActivated(QString)), this, SLOT(openUrlExternally(QString)));
    
    onSubscriptionsStatusChanged(Subscriptions::instance()->status());
    
    m_subscriptionsModel->load();

    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("windowGeometry").toByteArray());
    m_horizontalSplitter->restoreState(settings.value("horizontalSplitterState").toByteArray());
    m_verticalSplitter->restoreState(settings.value("verticalSplitterState").toByteArray());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *e) {
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("horizontalSplitterState", m_horizontalSplitter->saveState());
    settings.setValue("verticalSplitterState", m_verticalSplitter->saveState());
    settings.endGroup();
    QMainWindow::closeEvent(e);
}

void MainWindow::markAllSubscriptionsRead() {
    for (int i = 2; i < m_subscriptionsModel->rowCount(); i++) {
        const int id = m_subscriptionsModel->data(i, "id").toInt();
        
        if (id > 0) {
            Database::markSubscriptionRead(id, true);
        }
    }
}

void MainWindow::newSubscriptionRequested(QAction *action) {
    if ((action) && (action->data().toInt() == Subscription::Plugin)) {
        PluginDialog(action->text(), this).exec();
    }
    else{
        SubscriptionDialog(this).exec();
    }
}

void MainWindow::importSubscriptions() {
    const QString opmlFile = QFileDialog::getOpenFileName(this, tr("Select OPML file"), QString(),
                                                          tr("OPML (*.opml);;All files (*.*)"));
    
    if (!opmlFile.isEmpty()) {
        Subscriptions::instance()->importFromOpml(opmlFile);
    }
}

void MainWindow::updateCurrentSubscription() {
    const int id = m_subscriptionsView->currentIndex().data(SubscriptionModel::IdRole).toInt();
    
    if (id > 0) {
        Subscriptions::instance()->update(id);
    }
}

void MainWindow::markCurrentSubscriptionRead() {
    const int id = m_subscriptionsView->currentIndex().data(SubscriptionModel::IdRole).toInt();
    
    if (id > 0) {
        Database::markSubscriptionRead(id, true);
    }
}

void MainWindow::deleteCurrentSubscription() {
    const int id = m_subscriptionsView->currentIndex().data(SubscriptionModel::IdRole).toInt();
    
    if (id > 0) {
        Database::deleteSubscription(id);
    }
}

void MainWindow::showCurrentSubscriptionProperties() {
    const int id = m_subscriptionsView->currentIndex().data(SubscriptionModel::IdRole).toInt();
    
    if (id > 0) {
        const int sourceType = m_subscriptionsView->currentIndex().data(SubscriptionModel::SourceTypeRole).toInt();
        
        if (sourceType == Subscription::Plugin) {
            PluginDialog(id, this).exec();
        }
        else {
            SubscriptionDialog(id, this).exec();
        }
    }
}

void MainWindow::nextUnreadArticle() {
    if (m_articlesModel->status() == ArticleModel::Active) {
        return;
    }
    
    disconnect(m_articlesModel, SIGNAL(statusChanged(ArticleModel::Status)), this, SLOT(nextUnreadArticle()));
    
    const QModelIndex index =
    m_articlesView->currentIndex().isValid() ? m_articlesView->currentIndex()
                                             : m_articlesProxyModel->index(0, 0, QModelIndex());
    
    QModelIndexList list = m_articlesProxyModel->match(index, ArticleModel::ReadRole, false, 1);
    
    if (!list.isEmpty()) {
        setCurrentArticle(list.first());
        return;
    }
    
    list = m_subscriptionsModel->match(m_subscriptionsView->currentIndex(), SubscriptionModel::ReadRole, false, 1);
    
    if (!list.isEmpty()) {
        connect(m_articlesModel, SIGNAL(statusChanged(ArticleModel::Status)), this, SLOT(nextUnreadArticle()));
        setCurrentSubscription(list.first());
        return;
    }
    
    statusBar()->showMessage(tr("No unread articles"));
}

void MainWindow::nextArticle() {
    const int currentRow = m_articlesView->currentIndex().row();
    
    if (currentRow < (m_articlesProxyModel->rowCount() - 1)) {
        setCurrentArticle(m_articlesProxyModel->index(currentRow + 1, 0));
    }
}

void MainWindow::previousArticle() {
    const int currentRow = m_articlesView->currentIndex().row();
    
    if (currentRow > 0) {
        setCurrentArticle(m_articlesProxyModel->index(currentRow - 1, 0));
    }
}

void MainWindow::toggleCurrentArticleRead() {
    const int id = m_articlesView->currentIndex().data(ArticleModel::IdRole).toInt();
    
    if (id > 0) {
        Database::markArticleRead(id, !m_articlesView->currentIndex().data(ArticleModel::ReadRole).toBool());
    }
}

void MainWindow::toggleCurrentArticleFavourite() {
    const int id = m_articlesView->currentIndex().data(ArticleModel::IdRole).toInt();
    
    if (id > 0) {
        Database::markArticleFavourite(id, !m_articlesView->currentIndex().data(ArticleModel::FavouriteRole).toBool());
    }
}

void MainWindow::deleteCurrentArticle() {
    const int id = m_articlesView->currentIndex().data(ArticleModel::IdRole).toInt();
    
    if (id > 0) {
        Database::deleteArticle(id);
    }
}

void MainWindow::copyCurrentArticleUrl() {
    QApplication::clipboard()->setText(m_articlesView->currentIndex().data(ArticleModel::UrlRole).toString());
}

void MainWindow::openCurrentArticleInTab() {
    openUrlInTab(m_articlesView->currentIndex().data(ArticleModel::UrlRole).toString());
}

void MainWindow::openCurrentArticleInBrowser() {
    m_browser->setUrl(m_articlesView->currentIndex().data(ArticleModel::UrlRole).toString());
}

void MainWindow::openCurrentArticleExternally() {
    openUrlExternally(m_articlesView->currentIndex().data(ArticleModel::UrlRole).toString());
}

void MainWindow::copyCurrentEnclosureUrl() {
    if (QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        QApplication::clipboard()->setText(item->text());
    }
}

void MainWindow::openCurrentEnclosureInTab() {
    if (QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        openUrlInTab(item->text());
    }
}

void MainWindow::openCurrentEnclosureInBrowser() {
    if (QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        m_browser->setUrl(item->text());
    }
}

void MainWindow::openCurrentEnclosureExternally() {
    if (QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        openUrlExternally(item->text());
    }
}

void MainWindow::downloadCurrentEnclosure() {
    if (QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        Transfers::instance()->addDownloadTransfer(item->text(), m_subscriptionsView->currentIndex()
                                                   .data(SubscriptionModel::IdRole).toInt());
    }
}

void MainWindow::setCurrentSubscription(const QModelIndex &index) {
    if (index != m_subscriptionsView->currentIndex()) {
        m_subscriptionsView->setCurrentIndex(index);
    }
    
    m_infoLabel->setText(tr("<b>Feed:</b> <a href='%1'>%2</a><br><b>Source:</b> <a href='%3'>%3</a>")
                           .arg(index.data(SubscriptionModel::UrlRole).toString())
                           .arg(index.data(SubscriptionModel::TitleRole).toString())
                           .arg(index.data(SubscriptionModel::SourceRole).toString()));
    m_browser->setHtml(index.data(SubscriptionModel::DescriptionRole).toString());
    m_enclosuresLabel->hide();
    m_enclosuresView->hide();
    m_articlesModel->load(index.data(SubscriptionModel::IdRole).toInt());
}

void MainWindow::setCurrentArticle(const QModelIndex &index) {
    if (index != m_articlesView->currentIndex()) {
        m_articlesView->setCurrentIndex(index);
        return;
    }
    
    const QString author = index.data(ArticleModel::AuthorRole).toString();
    const QStringList categories = index.data(ArticleModel::CategoriesRole).toStringList();
    const bool isRead = index.data(ArticleModel::ReadRole).toBool();
    
    m_infoLabel->setText(tr("<b>Author:</b> %1<br><b>Categories:</b> %2")
                           .arg(author.isEmpty() ? tr("Unknown") : author)
                           .arg(categories.isEmpty() ? tr("None") : categories.join(", ")));
    m_browser->setHtml(index.data(ArticleModel::BodyRole).toString());
    m_enclosuresModel->clear();
    
    const QVariantList enclosures = index.data(ArticleModel::EnclosuresRole).toList();
    
    if (enclosures.isEmpty()) {
        m_enclosuresLabel->hide();
        m_enclosuresView->hide();
    }
    else {
        foreach (QVariant enclosure, enclosures) {
            const QVariantMap map = enclosure.toMap();
            const int size = map.value("length").toLongLong();
            
            QList<QStandardItem*> items;
            items << new QStandardItem(map.value("url").toString());
            QStandardItem *typeItem = new QStandardItem(size > 0 ? QString("%1  %2").arg(Utils::formatBytes(size))
                                                                                    .arg(map.value("type").toString())
                                                                 : map.value("type").toString());
            typeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            items << typeItem;
            
            m_enclosuresModel->appendRow(items);
        }
        
        const int count = enclosures.size();        
        m_enclosuresLabel->setText(count == 1 ? tr("1 enclosure") : tr("%1 enclosures").arg(count));
        m_enclosuresLabel->show();
        m_enclosuresView->setFixedHeight(m_enclosuresView->sizeHintForRow(0) * qMin(4, count + 1));
        m_enclosuresView->show();
    }
    
    if (!isRead) {
        Database::markArticleRead(index.data(ArticleModel::IdRole).toInt(), true);
    }
}

void MainWindow::searchArticles(const QString &query) {
    m_subscriptionsView->setCurrentIndex(m_subscriptionsModel->index(0));
    m_articlesModel->search(query);
}

void MainWindow::showSubscriptionContextMenu(const QPoint &pos) {
    if (m_subscriptionsView->currentIndex().isValid()) {
        m_subscriptionMenu->popup(m_subscriptionsView->mapToGlobal(pos), m_updateSubscriptionAction);
    }
}

void MainWindow::showArticleContextMenu(const QPoint &pos) {
    if (m_articlesView->currentIndex().isValid()) {
        m_articleContextMenu->popup(m_articlesView->mapToGlobal(pos), m_toggleArticleReadAction);
    }
}

void MainWindow::showEnclosureContextMenu(const QPoint &pos) {
    if (m_enclosuresView->currentIndex().isValid()) {
        m_enclosureContextMenu->popup(m_enclosuresView->mapToGlobal(pos), m_copyEnclosureUrlAction);
    }
}

void MainWindow::openUrlExternally(const QString &url) {
    if (m_urlOpenerModel->rowCount() == 0) {
        m_urlOpenerModel->load();
    }
        
    if (!m_urlOpenerModel->open(url)) {
        QDesktopServices::openUrl(url);
    }
}

void MainWindow::openUrlInTab(const QString &url) {
    Browser *browser = new Browser(url, m_stack);
    connect(browser, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(browser, SIGNAL(openUrlInTab(QString)), this, SLOT(openUrlInTab(QString)));
    connect(browser, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    m_stack->addWidget(browser);
    m_tabs->addTab(url);
    m_tabs->show();
}

void MainWindow::updateTabText(const QString &text) {
    if (Browser *browser = qobject_cast<Browser*>(sender())) {
        m_tabs->setTabText(m_stack->indexOf(browser), text);
    }
}

void MainWindow::closeTab(int index) {
    if (index > 0) {
        if (QWidget *widget = m_stack->widget(index)) {
            m_stack->removeWidget(widget);
            m_tabs->removeTab(index);
            widget->deleteLater();
            
            if (m_tabs->count() == 1) {
                m_tabs->hide();
            }
        }
    }
}

void MainWindow::closeCurrentTab() {
    closeTab(m_tabs->currentIndex());
}

void MainWindow::showDownloadsTab() {
    if (m_downloadsTab) {
        const int index = m_stack->indexOf(m_downloadsTab);
        m_stack->setCurrentIndex(index);
        m_tabs->setCurrentIndex(index);
    }
    else {
        m_downloadsTab = new DownloadsView(m_stack);
        m_stack->addWidget(m_downloadsTab);
        m_stack->setCurrentWidget(m_downloadsTab);
        m_tabs->addTab(tr("Downloads"));
        m_tabs->setCurrentIndex(m_stack->indexOf(m_downloadsTab));
        m_tabs->show();
    }
}

void MainWindow::showSearchDialog() {
    SearchDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        searchArticles(dialog.query());
    }   
}

void MainWindow::showSettingsDialog() {
    SettingsDialog(this).exec();
}

void MainWindow::onSubscriptionsCountChanged(int count) {
    const bool enable = (count > 2);
    m_updateAllSubscriptionsAction->setEnabled(enable);
    m_markAllSubscriptionsReadAction->setEnabled(enable);
    m_updateSubscriptionAction->setEnabled(enable);
    m_markSubscriptionReadAction->setEnabled(enable);
    m_deleteSubscriptionAction->setEnabled(enable);
    m_subscriptionPropertiesAction->setEnabled(enable);
}

void MainWindow::onSubscriptionsStatusChanged(Subscriptions::Status status) {
    const bool active = (status == Subscriptions::Active);    
    m_updateAllSubscriptionsAction->setVisible(!active);
    m_cancelSubscriptionUpdatesAction->setVisible(active);
}

void MainWindow::onArticlesCountChanged(int count) {
    const bool enable = (count > 0);
    m_markSubscriptionReadAction->setEnabled(enable);
    m_nextArticleAction->setEnabled(enable);
    m_previousArticleAction->setEnabled(enable);
    m_toggleArticleReadAction->setEnabled(enable);
    m_toggleArticleFavouriteAction->setEnabled(enable);
    m_deleteArticleAction->setEnabled(enable);
    m_copyArticleUrlAction->setEnabled(enable);
    m_openArticleInTabAction->setEnabled(enable);
    m_openArticleInBrowserAction->setEnabled(enable);
    m_openArticleExternallyAction->setEnabled(enable);    
}

void MainWindow::onCurrentTabChanged(int index) {
    m_stack->setCurrentIndex(index);
    m_closeTabAction->setEnabled(index > 0);
}
