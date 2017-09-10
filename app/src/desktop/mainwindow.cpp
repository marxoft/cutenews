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
#include "articlerequest.h"
#include "browserpage.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "plugindialog.h"
#include "pluginmanager.h"
#include "pluginsettings.h"
#include "qdatetimedialog.h"
#include "settings.h"
#include "settingsdialog.h"
#include "subscription.h"
#include "subscriptiondialog.h"
#include "subscriptionmodel.h"
#include "transfers.h"
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
    m_subscriptionsMenu(new QMenu(tr("&Subscriptions"), this)),
    m_newSubscriptionMenu(new QMenu(tr("New source"), this)),
    m_subscriptionMenu(new QMenu(tr("&Subscription"), this)),
    m_articleMenu(new QMenu(tr("&Article"), this)),
    m_articleContextMenu(new QMenu(this)),
    m_enclosureContextMenu(new QMenu(this)),
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
    m_openArticleInTabAction(new QAction(tr("Open in &tab"), this)),
    m_openArticleInBrowserAction(new QAction(tr("Open in &browser"), this)),
    m_openArticleExternallyAction(new QAction(tr("Open &externally"), this)),
    m_openArticleWithPluginAction(new QAction(tr("Open externally using &plugin"), this)),
    m_downloadArticleAction(new QAction(tr("&Download"), this)),
    m_downloadArticleWithPluginAction(new QAction(tr("Download using &plugin"), this)),
    m_copyEnclosureUrlAction(new QAction(tr("&Copy URL"), this)),
    m_openEnclosureInTabAction(new QAction(tr("Open in &tab"), this)),
    m_openEnclosureInBrowserAction(new QAction(tr("Open in &browser"), this)),
    m_openEnclosureExternallyAction(new QAction(tr("Open &externally"), this)),
    m_openEnclosureWithPluginAction(new QAction(tr("Open externally using &plugin"), this)),
    m_downloadEnclosureAction(new QAction(tr("&Download"), this)),
    m_downloadEnclosureWithPluginAction(new QAction(tr("Download using &plugin"), this)),
    m_transfersAction(new QAction(QIcon::fromTheme("folder-download"), tr("Show &downloads"), this)),
    m_closeTabAction(new QAction(QIcon::fromTheme("view-close"), tr("Close &tab"), this)),
    m_searchAction(new QAction(QIcon::fromTheme("edit-find"), tr("&Search all articles"), this)),
    m_deleteAction(new QAction(QIcon::fromTheme("edit-delete"), tr("&Delete read articles"), this)),
    m_pluginsAction(new QAction(QIcon::fromTheme("view-refresh"), tr("&Load plugins"), this)),
    m_settingsAction(new QAction(QIcon::fromTheme("document-properties"), tr("&Preferences"), this)),
    m_aboutAction(new QAction(QIcon::fromTheme("help-contents"), tr("&About"), this)),
    m_horizontalSplitter(new QSplitter(Qt::Horizontal, this)),
    m_verticalSplitter(new QSplitter(Qt::Vertical, this)),
    m_stack(new QStackedWidget(this)),
    m_tabs(new QTabBar(this)),
    m_subscriptionsView(new QTreeView(this)),
    m_articlesView(new QTreeView(this)),
    m_enclosuresView(new QTreeView(this)),
    m_infoLabel(new QLabel(this)),
    m_enclosuresLabel(new QLabel(this)),
    m_messageLabel(new QLabel(this)),
    m_browser(new BrowserPage(this)),
    m_tabsContainer(new QWidget(this)),
    m_articleContainer(new QWidget(this)),
    m_tabsLayout(new QVBoxLayout(m_tabsContainer)),
    m_articleLayout(new QVBoxLayout(m_articleContainer)),
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
    
    if (PluginManager::instance()->count() > 0) {
        const FeedPluginList plugins = PluginManager::instance()->plugins();
        
        for (int i = 0; i < plugins.size(); i++) {
            const FeedPluginConfig *config = plugins.at(i).config;

            if (config->supportsFeeds()) {
                m_newSubscriptionMenu->addAction(config->displayName())->setData(config->id());
            }
        }
    }
    else {
        m_newSubscriptionMenu->setEnabled(false);
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
    m_articleMenu->addAction(m_openArticleWithPluginAction);
    m_articleMenu->addAction(m_downloadArticleAction);
    m_articleMenu->addAction(m_downloadArticleWithPluginAction);
    
    m_articleContextMenu->addAction(m_toggleArticleReadAction);
    m_articleContextMenu->addAction(m_toggleArticleFavouriteAction);
    m_articleContextMenu->addAction(m_deleteArticleAction);
    m_articleContextMenu->addSeparator();
    m_articleContextMenu->addAction(m_copyArticleUrlAction);
    m_articleContextMenu->addAction(m_openArticleInTabAction);
    m_articleContextMenu->addAction(m_openArticleInBrowserAction);
    m_articleContextMenu->addAction(m_openArticleExternallyAction);
    m_articleContextMenu->addAction(m_openArticleWithPluginAction);
    m_articleContextMenu->addAction(m_downloadArticleAction);
    m_articleContextMenu->addAction(m_downloadArticleWithPluginAction);
    
    m_enclosureContextMenu->addAction(m_copyEnclosureUrlAction);
    m_enclosureContextMenu->addAction(m_openEnclosureInTabAction);
    m_enclosureContextMenu->addAction(m_openEnclosureInBrowserAction);
    m_enclosureContextMenu->addAction(m_openEnclosureExternallyAction);
    m_enclosureContextMenu->addAction(m_openEnclosureWithPluginAction);
    m_enclosureContextMenu->addAction(m_downloadEnclosureAction);
    m_enclosureContextMenu->addAction(m_downloadEnclosureWithPluginAction);
    
    m_viewMenu->addAction(m_transfersAction);
    m_viewMenu->addAction(m_closeTabAction);
    
    m_toolsMenu->addAction(m_searchAction);
    m_toolsMenu->addAction(m_deleteAction);
    m_toolsMenu->addAction(m_pluginsAction);
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
    
    m_searchAction->setShortcut(QKeySequence(tr("Ctrl+F")));
    m_pluginsAction->setShortcut(QKeySequence(tr("Ctrl+L")));
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
#else
    m_articlesView->header()->setResizeMode(0, QHeaderView::Fixed);
#endif
    m_articlesView->header()->restoreState(Settings::articlesHeaderViewState());
    
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

    m_messageLabel->setMargin(6);
    
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
    connect(Subscriptions::instance(), SIGNAL(statusTextChanged(QString)), this, SLOT(showMessage(QString)));
    connect(DBNotify::instance(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(DBNotify::instance(), SIGNAL(readArticlesDeleted(int)), this, SLOT(onReadArticlesDeleted(int)));
    connect(Settings::instance(), SIGNAL(offlineModeEnabledChanged(bool)),
            this, SLOT(onOfflineModeEnabledChanged(bool)));
    
    connect(m_subscriptionsModel, SIGNAL(countChanged(int)), this, SLOT(onSubscriptionsCountChanged(int)));
    connect(m_articlesModel, SIGNAL(countChanged(int)), this, SLOT(onArticlesCountChanged(int)));
    
    connect(m_newSubscriptionMenu, SIGNAL(triggered(QAction*)), this, SLOT(newSubscriptionRequested(QAction*)));
    
    connect(m_updateAllSubscriptionsAction, SIGNAL(triggered()), Subscriptions::instance(), SLOT(updateAll()));
    connect(m_cancelSubscriptionUpdatesAction, SIGNAL(triggered()), Subscriptions::instance(), SLOT(cancel()));
    connect(m_offlineModeAction, SIGNAL(triggered(bool)), Settings::instance(), SLOT(setOfflineModeEnabled(bool)));
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
    connect(m_openArticleWithPluginAction, SIGNAL(triggered()), this, SLOT(openCurrentArticleWithPlugin()));
    connect(m_downloadArticleAction, SIGNAL(triggered()), this, SLOT(downloadCurrentArticle()));
    connect(m_downloadArticleWithPluginAction, SIGNAL(triggered()), this, SLOT(downloadCurrentArticleWithPlugin()));
    
    connect(m_copyEnclosureUrlAction, SIGNAL(triggered()), this, SLOT(copyCurrentEnclosureUrl()));
    connect(m_openEnclosureInTabAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureInTab()));
    connect(m_openEnclosureInBrowserAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureInBrowser()));
    connect(m_openEnclosureExternallyAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureExternally()));
    connect(m_openEnclosureWithPluginAction, SIGNAL(triggered()), this, SLOT(openCurrentEnclosureWithPlugin()));
    connect(m_downloadEnclosureAction, SIGNAL(triggered()), this, SLOT(downloadCurrentEnclosure()));
    connect(m_downloadEnclosureWithPluginAction, SIGNAL(triggered()), this, SLOT(downloadCurrentEnclosureWithPlugin()));
    
    connect(m_transfersAction, SIGNAL(triggered()), this, SLOT(showTransfersTab()));
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));
    
    connect(m_searchAction, SIGNAL(triggered()), this, SLOT(showSearchDialog()));
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(showDeleteDialog()));
    connect(m_pluginsAction, SIGNAL(triggered()), this, SLOT(loadPlugins()));
    connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));

    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    
    connect(m_subscriptionsView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showSubscriptionContextMenu(QPoint)));
    connect(m_subscriptionsView, SIGNAL(activated(QModelIndex)), this, SLOT(setCurrentSubscription(QModelIndex)));
    connect(m_subscriptionsView, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentSubscription(QModelIndex)));
    
    connect(m_articlesView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showArticleContextMenu(QPoint)));
    connect(m_articlesView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openCurrentArticleInBrowser()));
    connect(m_articlesView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(setCurrentArticle(QModelIndex)));
    
    connect(m_enclosuresView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showEnclosureContextMenu(QPoint)));
    connect(m_enclosuresView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openCurrentEnclosureInBrowser()));
    
    connect(m_browser, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(m_browser, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(m_browser, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(m_browser, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(m_browser, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(m_browser, SIGNAL(downloadUrlWithPlugin(QString)), this, SLOT(downloadUrlWithPlugin(QString)));
    connect(m_browser, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(m_browser, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(m_browser, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(m_browser, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    
    connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));
    connect(m_tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    
    connect(m_infoLabel, SIGNAL(linkActivated(QString)), this, SLOT(openUrlExternally(QString)));
    
    onSubscriptionsStatusChanged(Subscriptions::instance()->status());
    onOfflineModeEnabledChanged(Settings::offlineModeEnabled());
    
    m_subscriptionsModel->load();

    restoreGeometry(Settings::mainWindowGeometry());
    restoreState(Settings::mainWindowState());
    m_horizontalSplitter->restoreState(Settings::mainWindowHorizontalSplitterState());
    m_verticalSplitter->restoreState(Settings::mainWindowVerticalSplitterState());
}

void MainWindow::closeEvent(QCloseEvent *e) {
    Settings::setMainWindowGeometry(saveGeometry());
    Settings::setMainWindowState(saveState());
    Settings::setMainWindowHorizontalSplitterState(m_horizontalSplitter->saveState());
    Settings::setMainWindowVerticalSplitterState(m_verticalSplitter->saveState());
    Settings::setArticlesHeaderViewState(m_articlesView->header()->saveState());
    QMainWindow::closeEvent(e);
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

void MainWindow::openCurrentArticleInTab() {
    const QModelIndex index = m_articlesView->currentIndex();
    openUrlInTab(index.data(Article::TitleRole).toString(), index.data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleInBrowser() {
    m_browser->setUrl(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleExternally() {
    openUrlExternally(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::openCurrentArticleWithPlugin() {
    openUrlWithPlugin(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::downloadCurrentArticle() {
    downloadUrl(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::downloadCurrentArticleWithPlugin() {
    downloadUrlWithPlugin(m_articlesView->currentIndex().data(Article::UrlRole).toString());
}

void MainWindow::copyCurrentEnclosureUrl() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        QApplication::clipboard()->setText(item->text());
    }
}

void MainWindow::openCurrentEnclosureInTab() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        openUrlInTab(item->text(), item->text());
    }
}

void MainWindow::openCurrentEnclosureInBrowser() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        m_browser->setUrl(item->text());
    }
}

void MainWindow::openCurrentEnclosureExternally() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        openUrlExternally(item->text());
    }
}

void MainWindow::openCurrentEnclosureWithPlugin() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        openUrlWithPlugin(item->text());
    }
}

void MainWindow::downloadCurrentEnclosure() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        downloadUrl(item->text());
    }
}

void MainWindow::downloadCurrentEnclosureWithPlugin() {
    if (const QStandardItem *item = m_enclosuresModel->item(m_enclosuresView->currentIndex().row(), 0)) {
        downloadUrlWithPlugin(item->text());
    }
}

void MainWindow::setCurrentSubscription(const QModelIndex &index) {
    if (index != m_subscriptionsView->currentIndex()) {
        m_subscriptionsView->setCurrentIndex(index);
    }
    
    if (index.data(Subscription::SourceTypeRole) == Subscription::Plugin) {
        const QString pluginId = index.data(Subscription::SourceRole).toMap().value("pluginId").toString();
        const FeedPluginConfig *config = PluginManager::instance()->getConfig(pluginId);
        m_infoLabel->setText(tr("<b>Feed:</b> <a href='%1'>%2</a><br><b>Source:</b> %3")
                             .arg(index.data(Subscription::UrlRole).toString())
                             .arg(index.data(Subscription::TitleRole).toString())
                             .arg(config ? config->displayName() : tr("Plugin")));
        
    }
    else {
        m_infoLabel->setText(tr("<b>Feed:</b> <a href='%1'>%2</a><br><b>Source:</b> <a href='%3'>%3</a>")
                             .arg(index.data(Subscription::UrlRole).toString())
                             .arg(index.data(Subscription::TitleRole).toString())
                             .arg(index.data(Subscription::SourceRole).toString()));
    }
    
    m_browser->setHtml(index.data(Subscription::DescriptionRole).toString());
    m_enclosuresLabel->hide();
    m_enclosuresView->hide();
    
    const QString id = index.data(Subscription::IdRole).toString();
    m_articlesModel->load(id);
    
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
    
    const QString author = index.data(Article::AuthorRole).toString();
    const QStringList categories = index.data(Article::CategoriesRole).toStringList();
    const bool isRead = index.data(Article::ReadRole).toBool();
    
    m_infoLabel->setText(tr("<b>Author:</b> %1<br><b>Categories:</b> %2")
                           .arg(author.isEmpty() ? tr("Unknown") : author)
                           .arg(categories.isEmpty() ? tr("None") : categories.join(", ")));
    m_browser->setHtml(index.data(Article::BodyRole).toString(), QString("file://"));
    m_enclosuresModel->clear();
    
    const QVariantList enclosures = index.data(Article::EnclosuresRole).toList();
    
    if (enclosures.isEmpty()) {
        m_enclosuresLabel->hide();
        m_enclosuresView->hide();
    }
    else {
        foreach (const QVariant &enclosure, enclosures) {
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
    
    const bool pluginEnabled = PluginManager::instance()->enclosureIsSupported(index.data(Article::UrlRole).toString());
    m_openArticleWithPluginAction->setEnabled(pluginEnabled);
    m_downloadArticleWithPluginAction->setEnabled(pluginEnabled);
    
    if (!isRead) {
        m_articlesModel->setData(m_articlesProxyModel->mapToSource(index), true, Article::ReadRole);
    }
}

void MainWindow::searchArticles(const QString &query) {
    m_subscriptionsView->setCurrentIndex(m_subscriptionsModel->index(0));
    m_articlesModel->search(query);
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

void MainWindow::showEnclosureContextMenu(const QPoint &pos) {
    if (m_enclosuresView->currentIndex().isValid()) {
        const bool pluginEnabled =
            PluginManager::instance()->enclosureIsSupported(m_enclosuresView->currentIndex().data().toString());
        m_openEnclosureWithPluginAction->setEnabled(pluginEnabled);
        m_downloadEnclosureWithPluginAction->setEnabled(pluginEnabled);
        m_enclosureContextMenu->popup(m_enclosuresView->mapToGlobal(pos), m_copyEnclosureUrlAction);
    }
}

void MainWindow::openArticleInTab(const QString &, const QString &url) {
    if (ArticleRequest *request = PluginManager::instance()->articleRequest(url)) {
        showMessage(tr("Retrieving article from %1").arg(url));
        connect(request, SIGNAL(finished(ArticleRequest*)), this, SLOT(onArticleRequestFinished(ArticleRequest*)));

        if (const FeedPluginConfig *config = PluginManager::instance()->getConfigForArticle(url)) {
            PluginSettings settings(config->id());
            request->getArticle(url, settings.values());
        }
        else {
            request->getArticle(url, QVariantMap());
        }
    }
    else {
        showError(tr("No plugin found for article URL %1").arg(url));
    }
}
void MainWindow::openUrlInTab(const QString &title, const QString &url) {
    BrowserPage *browser = new BrowserPage(url, m_stack);
    browser->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(browser, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(browser, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(browser, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(browser, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(browser, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(browser, SIGNAL(downloadUrlWithPlugin(QString)), this, SLOT(downloadUrlWithPlugin(QString)));
    connect(browser, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(browser, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(browser, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(browser, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    connect(browser, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    m_stack->addWidget(browser);
    m_tabs->addTab(title);
    m_tabs->show();
}

void MainWindow::openUrlExternally(const QString &url) {
    UrlOpenerModel::instance()->open(url);
}

void MainWindow::openUrlWithPlugin(const QString &url) {
    UrlOpenerModel::instance()->openWithPlugin(url);
}

void MainWindow::downloadUrl(const QString &url) {
    Transfers::instance()->addEnclosureDownload(url, false);
}

void MainWindow::downloadUrlWithPlugin(const QString &url) {
    Transfers::instance()->addEnclosureDownload(url, true);
}

void MainWindow::showHtmlInTab(const QString &title, const QString &html, const QString &baseUrl) {
    BrowserPage *browser = new BrowserPage(m_stack);
    browser->setAttribute(Qt::WA_DeleteOnClose, true);
    browser->setHtml(html, baseUrl);
    connect(browser, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(browser, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(browser, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(browser, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(browser, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(browser, SIGNAL(downloadUrlWithPlugin(QString)), this, SLOT(downloadUrlWithPlugin(QString)));
    connect(browser, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(browser, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(browser, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(browser, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    connect(browser, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    m_stack->addWidget(browser);
    m_tabs->addTab(title);
    m_tabs->show();
}

void MainWindow::showTextInTab(const QString &title, const QString &text, const QString &baseUrl) {
    BrowserPage *browser = new BrowserPage(m_stack);
    browser->setAttribute(Qt::WA_DeleteOnClose, true);
    browser->setText(text, baseUrl);
    connect(browser, SIGNAL(openArticleInTab(QString, QString)), this, SLOT(openArticleInTab(QString, QString)));
    connect(browser, SIGNAL(openUrlInTab(QString, QString)), this, SLOT(openUrlInTab(QString, QString)));
    connect(browser, SIGNAL(openUrlExternally(QString)), this, SLOT(openUrlExternally(QString)));
    connect(browser, SIGNAL(openUrlWithPlugin(QString)), this, SLOT(openUrlWithPlugin(QString)));
    connect(browser, SIGNAL(downloadUrl(QString)), this, SLOT(downloadUrl(QString)));
    connect(browser, SIGNAL(downloadUrlWithPlugin(QString)), this, SLOT(downloadUrlWithPlugin(QString)));
    connect(browser, SIGNAL(showHtmlInTab(QString, QString, QString)),
            this, SLOT(showHtmlInTab(QString, QString, QString)));
    connect(browser, SIGNAL(showTextInTab(QString, QString, QString)),
            this, SLOT(showTextInTab(QString, QString, QString)));
    connect(browser, SIGNAL(error(QString)), this, SLOT(showError(QString)));
    connect(browser, SIGNAL(information(QString)), this, SLOT(showMessage(QString)));
    connect(browser, SIGNAL(titleChanged(QString)), this, SLOT(updateTabText(QString)));
    m_stack->addWidget(browser);
    m_tabs->addTab(title);
    m_tabs->show();
}

void MainWindow::updateTabText(const QString &text) {
    if (QWidget* widget = qobject_cast<QWidget*>(sender())) {
        m_tabs->setTabText(m_stack->indexOf(widget), text);
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

void MainWindow::showSettingsDialog() {
    SettingsDialog(this).exec();
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

void MainWindow::showAboutDialog() {
    AboutDialog(this).exec();
}

void MainWindow::showMessage(const QString &message) {
    m_messageLabel->setText(message);
}

void MainWindow::showError(const QString &errorString) {
    QMessageBox::critical(this, tr("Error"), errorString);
}

void MainWindow::loadPlugins() {
    const int count = PluginManager::instance()->load();
    
    if (count > 0) {
        m_newSubscriptionMenu->clear();
        m_newSubscriptionMenu->setEnabled(true);
        const FeedPluginList plugins = PluginManager::instance()->plugins();
    
        for (int i = 0; i < plugins.size(); i++) {
            const FeedPluginConfig *config = plugins.at(i).config;
            
            if (config->supportsFeeds()) {
                m_newSubscriptionMenu->addAction(config->displayName())->setData(config->id());
            }
        }
        
        showMessage(tr("%1 new plugins loaded").arg(count));
    }
    else {
        showMessage(tr("No new plugins loaded"));
    }
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

void MainWindow::onArticleRequestFinished(ArticleRequest *request) {
    switch (request->status()) {
    case ArticleRequest::Ready:
        showMessage(tr("Article retrieved"));
        showHtmlInTab(request->resultTitle(), request->resultBody());
        break;
    case ArticleRequest::Error:
        showMessage(tr("Error retrieving article"));
        showError(request->errorString());
        break;
    default:
        break;
    }

    request->deleteLater();
}

void MainWindow::onArticlesCountChanged(int count) {
    const bool enable = (count > 0);
    const QString id = m_subscriptionsView->currentIndex().data(Subscription::IdRole).toString();
    m_markSubscriptionReadAction->setEnabled((enable) && (id != ALL_ARTICLES_SUBSCRIPTION_ID)
                                             && (id != FAVOURITES_SUBSCRIPTION_ID));
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

void MainWindow::onReadArticlesDeleted(int count) {
    if (count > 0) {
        showMessage(tr("%1 read articles deleted").arg(count));
    }
    else {
        showMessage(tr("No read articles deleted"));
    }
}

void MainWindow::onCurrentTabChanged(int index) {
    m_stack->setCurrentIndex(index);
    m_closeTabAction->setEnabled(index > 0);
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
