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
#include "aboutdialog.h"
#include "article.h"
#include "articlemodel.h"
#include "articlepage.h"
#include "articlerequest.h"
#include "browserpage.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "downloaddialog.h"
#include "plugindialog.h"
#include "pluginmanager.h"
#include "qdatetimedialog.h"
#include "serversettings.h"
#include "settings.h"
#include "settingsdialog.h"
#include "subscription.h"
#include "subscriptiondialog.h"
#include "subscriptionmodel.h"
#include "transfermodel.h"
#include "transferspage.h"
#include "urlopenermodel.h"
#include "utils.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabBar>
#include <QTreeView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_subscriptionsModel(new SubscriptionModel(this)),
    m_articlesModel(new ArticleModel(this)),
    m_articlesProxyModel(new QSortFilterProxyModel(this)),
    m_subscriptionsMenu(new QMenu(tr("&Subscriptions"), this)),
    m_newSubscriptionMenu(new QMenu(tr("New source"), this)),
    m_subscriptionMenu(new QMenu(tr("&Subscription"), this)),
    m_articleMenu(new QMenu(tr("&Article"), this)),
    m_articleContextMenu(new QMenu(this)),
    m_viewMenu(new QMenu(tr("&View"), this)),
    m_toolsMenu(new QMenu(tr("&Tools"), this)),
    m_helpMenu(new QMenu(tr("&Help"), this)),
    m_topToolBar(new QToolBar(this)),
    m_bottomToolBar(new QToolBar(this)),
    m_updateAllSubscriptionsAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Update all"), this)),
    m_cancelSubscriptionUpdatesAction(new QAction(QIcon::fromTheme("process-stop"), tr("&Cancel updates"), this)),
    m_offlineModeAction(new QAction(QIcon::fromTheme("network-transmit-receive"), tr("Work &offline"), this)),
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
    m_openArticleUrlInTabAction(new QAction(tr("Open URL in &tab"), this)),
    m_openArticleUrlInBrowserAction(new QAction(tr("Open URL in &browser"), this)),
    m_openArticleUrlExternallyAction(new QAction(tr("Open URL &externally"), this)),
    m_openArticleUrlWithPluginAction(new QAction(tr("Open URL &externally via plugin"), this)),
    m_openArticleInTabWithPluginAction(new QAction(tr("Open article in &tab via plugin"), this)),
    m_openArticleInBrowserWithPluginAction(new QAction(tr("Open article in &browser via plugin"), this)),
    m_downloadArticleUrlAction(new QAction(tr("&Download URL"), this)),
    m_transfersAction(new QAction(QIcon::fromTheme("folder-download"), tr("Show &downloads"), this)),
    m_closeTabAction(new QAction(QIcon::fromTheme("view-close"), tr("Close &tab"), this)),
    m_reloadAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Reload data"), this)),
    m_searchAction(new QAction(QIcon::fromTheme("edit-find"), tr("&Search all articles"), this)),
    m_articleAction(new QAction(QIcon::fromTheme("folder-remote"), tr("&Fetch article from URL"), this)),
    m_deleteAction(new QAction(QIcon::fromTheme("edit-delete"), tr("&Delete read articles"), this)),
    m_settingsAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Preferences"), this)),
    m_aboutAction(new QAction(QIcon::fromTheme("help-contents"), tr("&About"), this)),
    m_horizontalSplitter(new QSplitter(Qt::Horizontal, this)),
    m_verticalSplitter(new QSplitter(Qt::Vertical, this)),
    m_stack(new QStackedWidget(this)),
    m_tabs(new QTabBar(this)),
    m_subscriptionsView(new QTreeView(this)),
    m_articlesView(new QTreeView(this)),
    m_messageLabel(new QLabel(this)),
    m_articlePage(new ArticlePage(this)),
    m_tabsContainer(new QWidget(this)),
    m_tabsLayout(new QVBoxLayout(m_tabsContainer)),
    m_transfersPage(0)
{
    setWindowTitle("cuteNews");
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setCentralWidget(m_horizontalSplitter);
    addToolBar(Qt::TopToolBarArea, m_topToolBar);
    addToolBar(Qt::BottomToolBarArea, m_bottomToolBar);
    
    menuBar()->addMenu(m_subscriptionsMenu);
    menuBar()->addMenu(m_subscriptionMenu);
    menuBar()->addMenu(m_articleMenu);
    menuBar()->addMenu(m_viewMenu);
    menuBar()->addMenu(m_toolsMenu);
    menuBar()->addMenu(m_helpMenu);
    
    m_articlesProxyModel->setSourceModel(m_articlesModel);
    m_articlesProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_articlesProxyModel->setSortRole(Article::DateRole);
        
    m_subscriptionsMenu->addAction(m_updateAllSubscriptionsAction);
    m_subscriptionsMenu->addAction(m_cancelSubscriptionUpdatesAction);
    m_subscriptionsMenu->addAction(m_offlineModeAction);
    m_subscriptionsMenu->addAction(m_markAllSubscriptionsReadAction);
    m_subscriptionsMenu->addAction(m_newSubscriptionAction);
    m_subscriptionsMenu->addMenu(m_newSubscriptionMenu);
    m_subscriptionsMenu->addAction(m_importSubscriptionsAction);
    m_subscriptionsMenu->addAction(m_quitAction);
        
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
    m_articleMenu->addAction(m_openArticleUrlInTabAction);
    m_articleMenu->addAction(m_openArticleUrlInBrowserAction);
    m_articleMenu->addAction(m_openArticleUrlExternallyAction);
    m_articleMenu->addAction(m_openArticleUrlWithPluginAction);
    m_articleMenu->addAction(m_openArticleInTabWithPluginAction);
    m_articleMenu->addAction(m_openArticleInBrowserWithPluginAction);
    m_articleMenu->addAction(m_downloadArticleUrlAction);
    
    m_articleContextMenu->addAction(m_toggleArticleReadAction);
    m_articleContextMenu->addAction(m_toggleArticleFavouriteAction);
    m_articleContextMenu->addAction(m_deleteArticleAction);
    m_articleContextMenu->addSeparator();
    m_articleContextMenu->addAction(m_copyArticleUrlAction);
    m_articleContextMenu->addAction(m_openArticleUrlInTabAction);
    m_articleContextMenu->addAction(m_openArticleUrlInBrowserAction);
    m_articleContextMenu->addAction(m_openArticleUrlExternallyAction);
    m_articleContextMenu->addAction(m_openArticleUrlWithPluginAction);
    m_articleContextMenu->addAction(m_openArticleInTabWithPluginAction);
    m_articleContextMenu->addAction(m_openArticleInBrowserWithPluginAction);
    m_articleContextMenu->addAction(m_downloadArticleUrlAction);
    
    m_viewMenu->addAction(m_transfersAction);
    m_viewMenu->addAction(m_closeTabAction);
    
    m_toolsMenu->addAction(m_reloadAction);
    m_toolsMenu->addAction(m_searchAction);
    m_toolsMenu->addAction(m_articleAction);
    m_toolsMenu->addAction(m_deleteAction);
    m_toolsMenu->addAction(m_settingsAction);

    m_helpMenu->addAction(m_aboutAction);

    m_topToolBar->setObjectName("mainWindowTopToolBar");
    m_topToolBar->setWindowTitle(tr("Top toolbar"));
    m_topToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_topToolBar->setMovable(false);
    m_topToolBar->addAction(m_newSubscriptionAction);
    m_topToolBar->addAction(m_updateAllSubscriptionsAction);
    m_topToolBar->addAction(m_cancelSubscriptionUpdatesAction);
    m_topToolBar->addSeparator();
    m_topToolBar->addAction(m_markSubscriptionReadAction);
    m_topToolBar->addAction(m_previousArticleAction);
    m_topToolBar->addAction(m_nextArticleAction);
    m_topToolBar->addAction(m_nextUnreadArticleAction);
    m_topToolBar->addSeparator();
    m_topToolBar->addAction(m_searchAction);
    m_topToolBar->addAction(m_articleAction);

    m_bottomToolBar->setObjectName("mainWindowBottomToolBar");
    m_bottomToolBar->setWindowTitle(tr("Bottom toolbar"));
    m_bottomToolBar->setAllowedAreas(Qt::BottomToolBarArea);
    m_bottomToolBar->setMovable(false);
    m_bottomToolBar->addAction(m_offlineModeAction);
    m_bottomToolBar->addWidget(m_messageLabel);
    
    m_updateAllSubscriptionsAction->setShortcut(QKeySequence(tr("Ctrl+U")));
    m_updateAllSubscriptionsAction->setToolTip(tr("Update all subscriptions"));
    m_cancelSubscriptionUpdatesAction->setShortcut(QKeySequence(tr("Ctrl+Shift+U")));
    m_cancelSubscriptionUpdatesAction->setToolTip(tr("Cancel all subscription updates"));
    m_offlineModeAction->setCheckable(true);
    m_offlineModeAction->setPriority(QAction::LowPriority);
    m_newSubscriptionAction->setToolTip(tr("Add a new subscription"));
    m_importSubscriptionsAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    m_quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));
    
    m_markSubscriptionReadAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    m_markSubscriptionReadAction->setToolTip(tr("Mark all articles in the current subscription as read"));
    
    m_nextUnreadArticleAction->setShortcut(QKeySequence(tr("Ctrl+N")));
    m_nextUnreadArticleAction->setToolTip(tr("Go to the next unread article"));
    m_nextArticleAction->setToolTip(tr("Go to the next article in the current subscription"));
    m_nextArticleAction->setPriority(QAction::LowPriority);
    m_previousArticleAction->setShortcut(QKeySequence(tr("Ctrl+Shift+N")));
    m_previousArticleAction->setToolTip(tr("Go to the previous article in the current subscription"));
    m_previousArticleAction->setPriority(QAction::LowPriority);
    m_toggleArticleReadAction->setShortcut(QKeySequence(tr("Ctrl+M")));
    m_toggleArticleFavouriteAction->setShortcut(QKeySequence(tr("Ctrl+T")));
    
    m_transfersAction->setShortcut(QKeySequence(tr("Ctrl+D")));
    m_closeTabAction->setShortcut(QKeySequence(tr("Ctrl+W")));
    m_closeTabAction->setEnabled(false);
    
    m_reloadAction->setShortcut(QKeySequence(tr("Ctrl+Shift+R")));
    m_reloadAction->setToolTip(tr("Reload all data from the server"));
    m_searchAction->setShortcut(QKeySequence(tr("Ctrl+F")));
    m_articleAction->setShortcut(QKeySequence(tr("Ctrl+G")));
    m_settingsAction->setShortcut(QKeySequence(tr("Ctrl+P")));
    
    m_horizontalSplitter->addWidget(m_subscriptionsView);
    m_horizontalSplitter->addWidget(m_tabsContainer);
    m_horizontalSplitter->setStretchFactor(1, 1);
            
    m_verticalSplitter->addWidget(m_articlesView);
    m_verticalSplitter->addWidget(m_articlePage);
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
#else
    m_articlesView->header()->setResizeMode(0, QHeaderView::Fixed);
#endif
    m_articlesView->header()->restoreState(Settings::articlesHeaderViewState());
    
    m_messageLabel->setMargin(6);
    m_messageLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    
    m_tabsLayout->addWidget(m_tabs);
    m_tabsLayout->addWidget(m_stack);
    m_tabsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabsLayout->setStretch(1, 1);
        
    connect(Subscriptions::instance(), SIGNAL(statusChanged(Subscriptions::Status)),
            this, SLOT(onSubscriptionsStatusChanged(Subscriptions::Status)));
    connect(Subscriptions::instance(), SIGNAL(statusTextChanged(QString)), this, SLOT(showMessage(QString)));
    connect(DBNotify::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(DBNotify::instance(), SIGNAL(readArticlesDeleted(int)), this, SLOT(onReadArticlesDeleted(int)));
    connect(Settings::instance(), SIGNAL(enableJavaScriptInBrowserChanged(bool)),
            this, SLOT(onEnableJavaScriptInBrowserChanged(bool)));
    connect(Settings::instance(), SIGNAL(serverAddressChanged(QString)), this, SLOT(onServerAddressChanged(QString)));
    connect(ServerSettings::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(ServerSettings::instance(), SIGNAL(offlineModeEnabledChanged(bool)),
            this, SLOT(onOfflineModeEnabledChanged(bool)));
    connect(PluginManager::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(PluginManager::instance(), SIGNAL(loaded(int)), this, SLOT(onPluginsLoaded(int)));
    connect(TransferModel::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    
    connect(m_subscriptionsModel, SIGNAL(countChanged(int)), this, SLOT(onSubscriptionsCountChanged(int)));
    connect(m_articlesModel, SIGNAL(countChanged(int)), this, SLOT(onArticlesCountChanged(int)));
    
    connect(m_newSubscriptionMenu, SIGNAL(triggered(QAction*)), this, SLOT(newSubscriptionRequested(QAction*)));
    
    connect(m_updateAllSubscriptionsAction, SIGNAL(triggered()), Subscriptions::instance(), SLOT(updateAll()));
    connect(m_cancelSubscriptionUpdatesAction, SIGNAL(triggered()), Subscriptions::instance(), SLOT(cancel()));
    connect(m_offlineModeAction, SIGNAL(triggered(bool)),
            ServerSettings::instance(), SLOT(setOfflineModeEnabled(bool)));
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
    connect(m_openArticleUrlInTabAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleUrlInTab()));
    connect(m_openArticleUrlInBrowserAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleUrlInBrowser()));
    connect(m_openArticleUrlExternallyAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleUrlExternally()));
    connect(m_openArticleUrlWithPluginAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleUrlWithPlugin()));
    connect(m_openArticleInTabWithPluginAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleInTabWithPlugin()));
    connect(m_openArticleInBrowserWithPluginAction, SIGNAL(triggered()),
            this, SLOT(openCurrentArticleInBrowserWithPlugin()));
    connect(m_downloadArticleUrlAction, SIGNAL(triggered()), this, SLOT(downloadCurrentArticleUrl()));

    connect(m_transfersAction, SIGNAL(triggered()), this, SLOT(showTransfersTab()));
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));
    
    connect(m_reloadAction, SIGNAL(triggered()), this, SLOT(reloadData()));
    connect(m_searchAction, SIGNAL(triggered()), this, SLOT(showSearchDialog()));
    connect(m_articleAction, SIGNAL(triggered()), this, SLOT(showArticleDialog()));
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(showDeleteDialog()));
    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));

    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    
    connect(m_subscriptionsView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showSubscriptionContextMenu(QPoint)));
    connect(m_subscriptionsView, SIGNAL(activated(QModelIndex)), this, SLOT(setCurrentSubscription(QModelIndex)));
    connect(m_subscriptionsView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentSubscription(QModelIndex)));
    
    connect(m_articlesView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showArticleContextMenu(QPoint)));
    connect(m_articlesView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openCurrentArticleUrlInBrowser()));
    connect(m_articlesView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(setCurrentArticle(QModelIndex)));
    
    connect(m_articlePage, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(m_articlePage, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(m_articlePage, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(m_articlePage, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(m_articlePage, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(m_articlePage, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(m_articlePage, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(m_articlePage, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(m_articlePage, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));
    connect(m_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    
    restoreGeometry(Settings::mainWindowGeometry());
    restoreState(Settings::mainWindowState());
    m_horizontalSplitter->restoreState(Settings::mainWindowHorizontalSplitterState());
    m_verticalSplitter->restoreState(Settings::mainWindowVerticalSplitterState());
    
    UrlOpenerModel::instance()->load();
    onServerAddressChanged(Settings::serverAddress());
    onSubscriptionsCountChanged(0);
    onSubscriptionsStatusChanged(Subscriptions::instance()->status());
    onArticlesCountChanged(0);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    Settings::setMainWindowGeometry(saveGeometry());
    Settings::setMainWindowState(saveState());
    Settings::setMainWindowHorizontalSplitterState(m_horizontalSplitter->saveState());
    Settings::setMainWindowVerticalSplitterState(m_verticalSplitter->saveState());
    Settings::setArticlesHeaderViewState(m_articlesView->header()->saveState());
    QMainWindow::closeEvent(e);
}

void MainWindow::reloadData() {
    Subscriptions::instance()->getStatus(Subscriptions::DefaultStatusInterval);
    ServerSettings::instance()->load();
    PluginManager::instance()->load();
    m_articlesModel->clear();
    m_subscriptionsModel->load();
}

void MainWindow::newSubscriptionRequested(QAction *action) {
    if ((action) && (!action->data().isNull())) {
        PluginDialog(action->data().toString(), this).exec();
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

void MainWindow::markAllSubscriptionsRead() {
    DBConnection *connection = DBConnection::connection();
    connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
    connection->markAllSubscriptionsRead();
}

void MainWindow::updateCurrentSubscription() {
    Subscriptions::instance()->update(m_subscriptionsView->currentIndex().data(Subscription::IdRole).toString());
}

void MainWindow::markCurrentSubscriptionRead() {
    m_subscriptionsModel->setData(m_subscriptionsView->currentIndex(), true, Subscription::ReadRole);
}

void MainWindow::deleteCurrentSubscription() {
    if (QMessageBox::question(this, tr("Unsubscribe?"), tr("Do you want to unsubscribe from '%1'?")
                              .arg(m_subscriptionsView->currentIndex().data(Subscription::TitleRole).toString()))
        == QMessageBox::Yes) {
        m_subscriptionsModel->remove(m_subscriptionsView->currentIndex().row());
    }
}

void MainWindow::showCurrentSubscriptionProperties() {
    const QString id = m_subscriptionsView->currentIndex().data(Subscription::IdRole).toString();
    const int sourceType = m_subscriptionsView->currentIndex().data(Subscription::SourceTypeRole).toInt();
    
    if (sourceType == Subscription::Plugin) {
        PluginDialog(QString(), id, this).exec();
    }
    else {
        SubscriptionDialog(id, this).exec();
    }
}

void MainWindow::nextUnreadArticle() {
    if (m_articlesModel->status() != ArticleModel::Ready) {
        return;
    }
    
    disconnect(m_articlesModel, SIGNAL(statusChanged(ArticleModel::Status)), this, SLOT(nextUnreadArticle()));
    
    const QModelIndex index =
    m_articlesView->currentIndex().isValid() ? m_articlesView->currentIndex()
                                             : m_articlesProxyModel->index(0, 0, QModelIndex());
    
    QModelIndexList list = m_articlesProxyModel->match(index, Article::ReadRole, false, 1);
    
    if (!list.isEmpty()) {
        setCurrentArticle(list.first());
        return;
    }
    
    list = m_subscriptionsModel->match(m_subscriptionsView->currentIndex(), Subscription::ReadRole, false, 1);
    
    if (!list.isEmpty()) {
        connect(m_articlesModel, SIGNAL(statusChanged(ArticleModel::Status)), this, SLOT(nextUnreadArticle()));
        setCurrentSubscription(list.first());
        return;
    }
    
    showMessage(tr("No unread articles"));
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
    const QModelIndex index = m_articlesProxyModel->mapToSource(m_articlesView->currentIndex());
    m_articlesModel->setData(index, !index.data(Article::ReadRole).toBool(), Article::ReadRole);
}

void MainWindow::toggleCurrentArticleFavourite() {
    const QModelIndex index = m_articlesProxyModel->mapToSource(m_articlesView->currentIndex());
    m_articlesModel->setData(index, !index.data(Article::FavouriteRole).toBool(), Article::FavouriteRole);
}

void MainWindow::deleteCurrentArticle() {
    m_articlesModel->remove(m_articlesProxyModel->mapToSource(m_articlesView->currentIndex()).row());
}

void MainWindow::copyCurrentArticleUrl() {
    QApplication::clipboard()->setText(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleUrlInTab() {
    const QModelIndex index = m_articlesView->currentIndex();
    openUrlInTab(index.data(Article::TitleRole).toString(), index.data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleUrlInBrowser() {
    m_articlePage->setUrl(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleUrlExternally() {
    openUrlExternally(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleUrlWithPlugin() {
    openUrlWithPlugin(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleInTabWithPlugin() {
    fetchArticle(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleInBrowserWithPlugin() {
    fetchArticle(m_articlesView->currentIndex().data(Article::UrlRole).toString(), false);
}

void MainWindow::downloadCurrentArticleUrl() {
    downloadUrl(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::setCurrentSubscription(const QModelIndex &index) {
    if (index != m_subscriptionsView->currentIndex()) {
        m_subscriptionsView->setCurrentIndex(index);
    }
    
    if (index.data(Subscription::SourceTypeRole) == Subscription::Plugin) {
        const QString pluginId = index.data(Subscription::SourceRole).toMap().value("pluginId").toString();
        const FeedPluginConfig *config = PluginManager::instance()->getConfig(pluginId);
        m_articlePage->setHeader(tr("<b>Feed:</b> <a href='%1'>%2</a><br><b>Source:</b> %3")
                .arg(index.data(Subscription::UrlRole).toString()).arg(index.data(Subscription::TitleRole).toString())
                .arg(config ? config->displayName() : tr("Plugin")));
        
    }
    else {
        m_articlePage->setHeader(tr("<b>Feed:</b> <a href='%1'>%2</a><br><b>Source:</b> <a href='%3'>%3</a>")
                .arg(index.data(Subscription::UrlRole).toString()).arg(index.data(Subscription::TitleRole).toString())
                .arg(index.data(Subscription::SourceRole).toString()));
    }
    
    m_articlePage->setEnclosures(QVariantList());
    m_articlePage->setHtml(index.data(Subscription::DescriptionRole).toString());
    
    const QString id = index.data(Subscription::IdRole).toString();
    m_articlesModel->load(id);

    m_subscriptionMenu->setEnabled(true);
    
    if ((id == ALL_ARTICLES_SUBSCRIPTION_ID) || (id == FAVOURITES_SUBSCRIPTION_ID)) {
        m_updateSubscriptionAction->setEnabled(false);
        m_markSubscriptionReadAction->setEnabled(false);
        m_deleteSubscriptionAction->setEnabled(false);
        m_subscriptionPropertiesAction->setEnabled(false);
    }
    else {
        m_updateSubscriptionAction->setEnabled(true);
        m_markSubscriptionReadAction->setEnabled(true);
        m_deleteSubscriptionAction->setEnabled(true);
        m_subscriptionPropertiesAction->setEnabled(true);
    }
}

void MainWindow::setCurrentArticle(const QModelIndex &index) {
    if (index != m_articlesView->currentIndex()) {
        m_articlesView->setCurrentIndex(index);
        return;
    }
    
    const QString title = index.data(Article::TitleRole).toString();
    const QString author = index.data(Article::AuthorRole).toString();
    const QString date = index.data(Article::DateStringRole).toString();
    const QStringList categories = index.data(Article::CategoriesRole).toStringList();
    const QString url = index.data(Article::UrlRole).toString();
    const bool isRead = index.data(Article::ReadRole).toBool();
    
    m_articlePage->setHeader(tr("<b>Title:</b> %1<br><b>Author:</b> %2<br><b>Date:</b> %3<br><b>Categories:</b> %4")
            .arg(title.isEmpty() ? tr("Unknown") : title).arg(author.isEmpty() ? tr("Unknown") : author)
            .arg(date.isEmpty() ? tr("Unknown") : date)
            .arg(categories.isEmpty() ? tr("None") : categories.join(", ")));
    m_articlePage->setHtml(index.data(Article::BodyRole).toString(), QString("file://"));
    m_articlePage->setEnclosures(index.data(Article::EnclosuresRole).toList());

    const FeedPluginConfig *articleConfig = PluginManager::instance()->getConfigForArticle(url);
    const FeedPluginConfig *enclosureConfig = PluginManager::instance()->getConfigForEnclosure(url);

    m_articleMenu->setEnabled(true);
    m_articleContextMenu->setEnabled(true);
    m_nextUnreadArticleAction->setEnabled(true);
    m_nextArticleAction->setEnabled(true);
    m_previousArticleAction->setEnabled(true);
    m_toggleArticleReadAction->setEnabled(true);
    m_toggleArticleFavouriteAction->setEnabled(true);
    m_deleteArticleAction->setEnabled(true);
    m_copyArticleUrlAction->setEnabled(true);
    m_openArticleUrlInTabAction->setEnabled(true);
    m_openArticleUrlInBrowserAction->setEnabled(true);
    m_openArticleUrlExternallyAction->setEnabled(true);
    m_downloadArticleUrlAction->setEnabled(true);

    if (articleConfig) {
        m_openArticleInTabWithPluginAction->setEnabled(true);
        m_openArticleInTabWithPluginAction->setText(tr("Open article in &tab via %1")
                .arg(articleConfig->displayName()));
        m_openArticleInBrowserWithPluginAction->setEnabled(true);
        m_openArticleInBrowserWithPluginAction->setText(tr("Open article in &browser via %1")
                .arg(articleConfig->displayName()));
    }
    else {
        m_openArticleInTabWithPluginAction->setEnabled(false);
        m_openArticleInTabWithPluginAction->setText(tr("Open article in &tab via plugin"));
        m_openArticleInBrowserWithPluginAction->setEnabled(false);
        m_openArticleInBrowserWithPluginAction->setText(tr("Open article in &browser via plugin"));
    }

    if (enclosureConfig) {
        m_openArticleUrlWithPluginAction->setEnabled(true);
        m_openArticleUrlWithPluginAction->setText(tr("Open URL externally via &%1")
                .arg(enclosureConfig->displayName()));
    }
    else {
        m_openArticleUrlWithPluginAction->setEnabled(false);
        m_openArticleUrlWithPluginAction->setText(tr("Open URL externally via &plugin"));
    }
    
    if (!isRead) {
        m_articlesModel->setData(m_articlesProxyModel->mapToSource(index), true, Article::ReadRole);
    }
}

void MainWindow::searchArticles(const QString &query) {
    m_subscriptionsView->setCurrentIndex(m_subscriptionsModel->index(0));
    m_articlesModel->search(query);
}

void MainWindow::fetchArticle(const QString &url, bool openInTab) {
    if (PluginManager::instance()->articleIsSupported(url)) {
        showMessage(tr("Fetching article from %1").arg(url));
        ArticleRequest *request = new ArticleRequest(this);
        connect(request, SIGNAL(finished(ArticleRequest*)), this, SLOT(onArticleRequestFinished(ArticleRequest*)));
        request->setProperty("openintab", openInTab);
        request->getArticle(url);
    }
    else {
        showError(tr("No plugin found for article URL %1").arg(url));
    }
}

void MainWindow::deleteReadArticles(int expiryDate) {
    showMessage(tr("Deleting read articles"));
    DBConnection *connection = DBConnection::connection();
    connect(connection, SIGNAL(finished(DBConnection*)), connection, SLOT(deleteLater()));
    connection->deleteReadArticles(expiryDate);
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

void MainWindow::openArticleInTab(const QString &, const QString &url) {
    fetchArticle(url);
}

void MainWindow::openUrlInTab(const QString &title, const QString &url) {
    BrowserPage *browser = addBrowserTab(title);
    browser->setUrl(url);
}

void MainWindow::openUrlExternally(const QString &url) {
    UrlOpenerModel::instance()->open(url);
}

void MainWindow::openUrlWithPlugin(const QString &url) {
    UrlOpenerModel::instance()->openWithPlugin(url);
}

void MainWindow::downloadUrl(const QString &url) {
    DownloadDialog dialog(url, this);

    if (dialog.exec() == QDialog::Accepted) {
        TransferModel::instance()->addEnclosureDownload(dialog.url(), dialog.customCommand(),
                dialog.customCommandOverrideEnabled(), dialog.category(), dialog.priority(), dialog.usePlugin());
    }
}

void MainWindow::showHtmlInTab(const QString &title, const QString &html, const QString &baseUrl) {
    BrowserPage *browser = addBrowserTab(title);
    browser->setHtml(html, baseUrl);
}

void MainWindow::showTextInTab(const QString &title, const QString &text, const QString &baseUrl) {
    BrowserPage *browser = addBrowserTab(title);
    browser->setText(text, baseUrl);
}

ArticlePage* MainWindow::addArticleTab(const QString &title) {
    ArticlePage *page = new ArticlePage(m_stack);
    page->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(page, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(page, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(page, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(page, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(page, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(page, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(page, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(page, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(page, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    connect(page, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    m_stack->addWidget(page);
    m_tabs->addTab(title);
    m_tabs->show();
    return page;
}

BrowserPage* MainWindow::addBrowserTab(const QString &title) {
    BrowserPage *page = new BrowserPage(m_stack);
    page->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(page, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(page, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(page, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(page, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(page, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(page, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(page, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(page, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(page, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    connect(page, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    m_stack->addWidget(page);
    m_tabs->addTab(title);
    m_tabs->show();
    return page;
}

void MainWindow::updateTabText(const QString &text) {
    if (!text.isEmpty()) {
        if (QWidget* widget = qobject_cast<QWidget*>(sender())) {
            m_tabs->setTabText(m_stack->indexOf(widget), text);
        }
    }
}

void MainWindow::closeTab(int index) {
    if (index > 0) {
        if (QWidget *widget = m_stack->widget(index)) {
            m_stack->removeWidget(widget);
            m_tabs->removeTab(index);
            widget->close();
            
            if (m_tabs->count() == 1) {
                m_tabs->hide();
            }
        }
    }
}

void MainWindow::closeCurrentTab() {
    closeTab(m_tabs->currentIndex());
}

void MainWindow::showTransfersTab() {
    if (m_transfersPage) {
        const int index = m_stack->indexOf(m_transfersPage);
        m_stack->setCurrentIndex(index);
        m_tabs->setCurrentIndex(index);
    }
    else {
        m_transfersPage = new TransfersPage(m_stack);
        m_transfersPage->setAttribute(Qt::WA_DeleteOnClose, true);
        m_stack->addWidget(m_transfersPage);
        m_stack->setCurrentWidget(m_transfersPage);
        m_tabs->addTab(tr("Downloads"));
        m_tabs->setCurrentIndex(m_stack->indexOf(m_transfersPage));
        m_tabs->show();
    }
}

void MainWindow::showSearchDialog() {
    QString query = QInputDialog::getText(this, tr("Search all articles"), tr("Query"));
    
    if (!query.isEmpty()) {
        searchArticles(query);
    }  
}

void MainWindow::showArticleDialog() {
    const QString url = QInputDialog::getText(this, tr("Fetch article from URL"), tr("URL"));

    if (!url.isEmpty()) {
        fetchArticle(url);
    }
}

void MainWindow::showDeleteDialog() {
    const QDateTime current = QDateTime::currentDateTime();
    const int secs = current.toTime_t() - Settings::readArticleExpiry();
    QDateTimeDialog dialog(this);
    dialog.setWindowTitle(tr("Delete read articles"));
    dialog.setDateTime(QDateTime::fromTime_t(secs));
    dialog.setMaximumDateTime(current);
    
    if (dialog.exec() == QDialog::Accepted) {
        Settings::setReadArticleExpiry(dialog.dateTime().secsTo(current));
        deleteReadArticles(dialog.dateTime().toTime_t());
    }
}

void MainWindow::showSettingsDialog() {
    SettingsDialog(this).exec();
}

void MainWindow::showAboutDialog() {
    AboutDialog(this).exec();
}

void MainWindow::showMessage(const QString &message) {
    m_messageLabel->setText(m_messageLabel->fontMetrics().elidedText(message, Qt::ElideRight,
                m_messageLabel->width() - m_messageLabel->margin() * 2));
}

void MainWindow::showError(const QString &errorString) {
    QMessageBox::critical(this, tr("Error"), errorString);
}

void MainWindow::onSubscriptionsCountChanged(int count) {
    if (count <= 2) {
        m_subscriptionMenu->setEnabled(false);
        m_updateAllSubscriptionsAction->setEnabled(false);
        m_markSubscriptionReadAction->setEnabled(false);
        return;
    }

    const bool valid = m_subscriptionsView->currentIndex().isValid();
    m_updateAllSubscriptionsAction->setEnabled(true);
    m_subscriptionMenu->setEnabled(valid);
    m_updateSubscriptionAction->setEnabled(valid);
    m_markSubscriptionReadAction->setEnabled(valid);
}

void MainWindow::onSubscriptionsStatusChanged(Subscriptions::Status status) {
    const bool active = (status == Subscriptions::Active);    
    m_updateAllSubscriptionsAction->setVisible(!active);
    m_cancelSubscriptionUpdatesAction->setVisible(active);
}

void MainWindow::onArticleRequestFinished(ArticleRequest *request) {
    switch (request->status()) {
    case ArticleRequest::Ready: {
        showMessage(tr("Article fetched"));
        const QString title = request->resultTitle();
        const QString author = request->resultAuthor();
        const QString date = request->resultDateString();
        const QStringList categories = request->resultCategories();
        ArticlePage *page = request->property("openintab").toBool()
            ? addArticleTab(title.isEmpty() ? tr("Article") : title) : m_articlePage;
        page->setHeader(tr("<b>Title:</b> %1<br><b>Author:</b> %2<br><b>Date:</b> %3<br><b>Categories:</b> %4")
                .arg(title.isEmpty() ? tr("Unknown") : title).arg(author.isEmpty() ? tr("Unknown") : author)
                .arg(date.isEmpty() ? tr("Unknown") : date)
                .arg(categories.isEmpty() ? tr("None") : categories.join(", ")));
        page->setHtml(request->resultBody());
        page->setEnclosures(request->resultEnclosures());
        break;
    }
    case ArticleRequest::Error:
        showMessage(tr("Error fetching article"));
        showError(request->errorString());
        break;
    default:
        break;
    }

    request->deleteLater();
}

void MainWindow::onArticlesCountChanged(int count) {
    if (count <= 0) {
        m_articleMenu->setEnabled(false);
        m_articleContextMenu->setEnabled(false);
        m_nextUnreadArticleAction->setEnabled(false);
        m_nextArticleAction->setEnabled(false);
        m_previousArticleAction->setEnabled(false);
        return;
    }

    const bool valid = m_articlesView->currentIndex().isValid();
    m_articleMenu->setEnabled(true);
    m_articleContextMenu->setEnabled(true);
    m_nextUnreadArticleAction->setEnabled(true);
    m_nextArticleAction->setEnabled(true);
    m_previousArticleAction->setEnabled(true);
    m_toggleArticleReadAction->setEnabled(valid);
    m_toggleArticleFavouriteAction->setEnabled(valid);
    m_deleteArticleAction->setEnabled(valid);
    m_copyArticleUrlAction->setEnabled(valid);
    m_openArticleUrlInTabAction->setEnabled(valid);
    m_openArticleUrlInBrowserAction->setEnabled(valid);
    m_openArticleUrlExternallyAction->setEnabled(valid);
    m_openArticleUrlWithPluginAction->setEnabled(valid);
    m_openArticleInTabWithPluginAction->setEnabled(valid);
    m_openArticleInBrowserWithPluginAction->setEnabled(valid);
    m_downloadArticleUrlAction->setEnabled(valid);
}

void MainWindow::onReadArticlesDeleted(int count) {
    if (count > 0) {
        showMessage(tr("%1 read article(s) deleted").arg(count));
    }
    else {
        showMessage(tr("No read articles deleted"));
    }
}

void MainWindow::onCurrentTabChanged(int index) {
    m_stack->setCurrentIndex(index);
    m_closeTabAction->setEnabled(index > 0);
}

void MainWindow::onEnableJavaScriptInBrowserChanged(bool enabled) {
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, enabled);
}

void MainWindow::onOfflineModeEnabledChanged(bool enabled) {
    m_offlineModeAction->setChecked(enabled);

    if (enabled) {
        m_offlineModeAction->setIcon(QIcon::fromTheme("network-offline"));
        m_offlineModeAction->setText(tr("Work &online"));
        m_offlineModeAction->setToolTip(tr("Work online"));
    }
    else {
        m_offlineModeAction->setIcon(QIcon::fromTheme("network-transmit-receive"));
        m_offlineModeAction->setText(tr("Work &offline"));
        m_offlineModeAction->setToolTip(tr("Work offline"));
    }
}

void MainWindow::onPluginsLoaded(int count) {
    m_newSubscriptionMenu->clear();
    
    if (count == 0) {
        m_newSubscriptionMenu->setEnabled(false);
        return;
    }
    
    m_newSubscriptionMenu->setEnabled(true);
    const FeedPluginList plugins = PluginManager::instance()->plugins();
    
    for (int i = 0; i < plugins.size(); i++) {        
        if (plugins.at(i)->supportsFeeds()) {
            m_newSubscriptionMenu->addAction(plugins.at(i)->displayName())->setData(plugins.at(i)->id());
        }
    }
}

void MainWindow::onServerAddressChanged(const QString &address) {
    m_subscriptionsModel->clear();
    m_articlesModel->clear();
    m_newSubscriptionMenu->clear();
    const bool connected = !address.isEmpty();
    m_subscriptionMenu->setEnabled(connected);
    m_articleMenu->setEnabled(connected);
    m_articleContextMenu->setEnabled(connected);
    m_updateAllSubscriptionsAction->setEnabled(connected);
    m_cancelSubscriptionUpdatesAction->setEnabled(connected);
    m_offlineModeAction->setEnabled(connected);
    m_markAllSubscriptionsReadAction->setEnabled(connected);
    m_newSubscriptionAction->setEnabled(connected);
    m_importSubscriptionsAction->setEnabled(connected);
    m_nextUnreadArticleAction->setEnabled(connected);
    m_nextArticleAction->setEnabled(connected);
    m_previousArticleAction->setEnabled(connected);
    m_transfersAction->setEnabled(connected);
    m_reloadAction->setEnabled(connected);
    m_searchAction->setEnabled(connected);
    m_articleAction->setEnabled(connected);
    m_deleteAction->setEnabled(connected);
    
    if (connected) {
        Subscriptions::instance()->getStatus(Subscriptions::DefaultStatusInterval);
        ServerSettings::instance()->load();
        PluginManager::instance()->load();
        m_subscriptionsModel->load();
    }
}
