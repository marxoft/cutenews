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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "subscriptions.h"
#include <QMainWindow>
#include <QPointer>

class Article;
class ArticleModel;
class ArticlePage;
class ArticleRequest;
class BrowserPage;
class SubscriptionModel;
class TransfersPage;
class QLabel;
class QMenu;
class QModelIndex;
class QSortFilterProxyModel;
class QSplitter;
class QStackedWidget;
class QTabBar;
class QTreeView;
class QToolBar;
class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *e);

private Q_SLOTS:
    void reloadData();
    void newSubscriptionRequested(QAction *action = 0);
    void importSubscriptions();
    void markAllSubscriptionsRead();
    
    void updateCurrentSubscription();
    void markCurrentSubscriptionRead();
    void deleteCurrentSubscription();
    void showCurrentSubscriptionProperties();
    
    void nextUnreadArticle();
    void nextArticle();
    void previousArticle();
    void toggleCurrentArticleRead();
    void toggleCurrentArticleFavourite();
    void deleteCurrentArticle();
    void copyCurrentArticleUrl();
    void openCurrentArticleUrlInTab();
    void openCurrentArticleUrlInBrowser();
    void openCurrentArticleUrlExternally();
    void openCurrentArticleUrlWithPlugin();
    void openCurrentArticleInTabWithPlugin();
    void openCurrentArticleInBrowserWithPlugin();
    void downloadCurrentArticleUrl();
    
    void setCurrentSubscription(const QModelIndex &index);
    void setCurrentArticle(const QModelIndex &index);
    
    void searchArticles(const QString &query);
    void fetchArticle(const QString &url, bool openInTab = true);
    void deleteReadArticles(int expiryDate);
    
    void showSubscriptionContextMenu(const QPoint &pos);
    void showArticleContextMenu(const QPoint &pos);
    
    void openArticleInTab(const QString &title, const QString &url);
    void openUrlInTab(const QString &title, const QString &url);
    void openUrlExternally(const QString &url);
    void openUrlWithPlugin(const QString &url);
    void downloadUrl(const QString &url);
    void showHtmlInTab(const QString &title, const QString &html, const QString &baseUrl = QString());
    void showTextInTab(const QString &title, const QString &text, const QString &baseUrl = QString());

    void updateTabText(const QString &text);
    void closeTab(int index);
    void closeCurrentTab();
    
    void showTransfersTab();

    void showSearchDialog();
    void showArticleDialog();
    void showDeleteDialog();
    void showSettingsDialog();
    void showAboutDialog();
    
    void showMessage(const QString &message);
    void showError(const QString &errorString);

    void onSubscriptionsCountChanged(int count);
    void onSubscriptionsStatusChanged(Subscriptions::Status status);
    void onArticleRequestFinished(ArticleRequest *request);
    void onArticlesCountChanged(int count);
    void onReadArticlesDeleted(int count);
    
    void onCurrentTabChanged(int index);

    void onEnableJavaScriptInBrowserChanged(bool enabled);
    void onOfflineModeEnabledChanged(bool enabled);
    
    void onPluginsLoaded(int count);
    
    void onServerAddressChanged(const QString &address);

private:    
    ArticlePage* addArticleTab(const QString &title);
    BrowserPage* addBrowserTab(const QString &title);

    SubscriptionModel *m_subscriptionsModel;
    ArticleModel *m_articlesModel;
    QSortFilterProxyModel *m_articlesProxyModel;
    
    QMenu *m_subscriptionsMenu;
    QMenu *m_newSubscriptionMenu;
    QMenu *m_subscriptionMenu;
    QMenu *m_articleMenu;
    QMenu *m_articleContextMenu;
    QMenu *m_viewMenu;
    QMenu *m_toolsMenu;
    QMenu *m_helpMenu;
        
    QToolBar *m_topToolBar;
    QToolBar *m_bottomToolBar;
    
    QAction *m_updateAllSubscriptionsAction;
    QAction *m_cancelSubscriptionUpdatesAction;
    QAction *m_offlineModeAction;
    QAction *m_markAllSubscriptionsReadAction;
    QAction *m_newSubscriptionAction;
    QAction *m_importSubscriptionsAction;
    QAction *m_quitAction;
    
    QAction *m_updateSubscriptionAction;
    QAction *m_markSubscriptionReadAction;
    QAction *m_deleteSubscriptionAction;
    QAction *m_subscriptionPropertiesAction;
    
    QAction *m_nextUnreadArticleAction;
    QAction *m_nextArticleAction;
    QAction *m_previousArticleAction;
    QAction *m_toggleArticleReadAction;
    QAction *m_toggleArticleFavouriteAction;
    QAction *m_deleteArticleAction;
    QAction *m_copyArticleUrlAction;
    QAction *m_openArticleUrlInTabAction;
    QAction *m_openArticleUrlInBrowserAction;
    QAction *m_openArticleUrlExternallyAction;
    QAction *m_openArticleUrlWithPluginAction;
    QAction *m_openArticleInTabWithPluginAction;
    QAction *m_openArticleInBrowserWithPluginAction;
    QAction *m_downloadArticleUrlAction;
    
    QAction *m_transfersAction;
    QAction *m_closeTabAction;
    
    QAction *m_reloadAction;
    QAction *m_searchAction;
    QAction *m_articleAction;
    QAction *m_deleteAction;
    QAction *m_settingsAction;

    QAction *m_aboutAction;
    
    QSplitter *m_horizontalSplitter;
    QSplitter *m_verticalSplitter;
    
    QStackedWidget *m_stack;
    
    QTabBar *m_tabs;
    
    QTreeView *m_subscriptionsView;
    QTreeView *m_articlesView;
    QTreeView *m_enclosuresView;    
    
    QLabel *m_messageLabel;
    
    ArticlePage *m_articlePage;

    QWidget *m_tabsContainer;
    
    QVBoxLayout *m_tabsLayout;
    
    QPointer<TransfersPage> m_transfersPage;
};

#endif // MAINWINDOW_H
