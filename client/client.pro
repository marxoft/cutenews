TEMPLATE = app
TARGET = cutenews-client

QT += network

INCLUDEPATH += \
    src/base \
    src/plugins

HEADERS += \
    src/base/article.h \
    src/base/articlemodel.h \
    src/base/categorynamemodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/dbconnection.h \
    src/base/dbnotify.h \
    src/base/download.h \
    src/base/enclosuredownload.h \
    src/base/json.h \
    src/base/loggerverbositymodel.h \
    src/base/opmlparser.h \
    src/base/requests.h \
    src/base/selectionmodel.h \
    src/base/serversettings.h \
    src/base/subscription.h \
    src/base/subscriptionmodel.h \
    src/base/subscriptions.h \
    src/base/transfer.h \
    src/base/transfermodel.h \
    src/base/transferprioritymodel.h \
    src/base/updateintervaltypemodel.h \
    src/base/urlopenermodel.h \
    src/base/utils.h \
    src/plugins/feedpluginconfig.h \
    src/plugins/pluginconfigmodel.h \
    src/plugins/pluginmanager.h

SOURCES += \
    src/base/article.cpp \
    src/base/articlemodel.cpp \
    src/base/dbconnection.cpp \
    src/base/dbnotify.cpp \
    src/base/download.cpp \
    src/base/enclosuredownload.cpp \
    src/base/json.cpp \
    src/base/opmlparser.cpp \
    src/base/selectionmodel.cpp \
    src/base/serversettings.cpp \
    src/base/subscription.cpp \
    src/base/subscriptionmodel.cpp \
    src/base/subscriptions.cpp \
    src/base/transfer.cpp \
    src/base/transfermodel.cpp \
    src/base/urlopenermodel.cpp \
    src/base/utils.cpp \
    src/plugins/feedpluginconfig.cpp \
    src/plugins/pluginconfigmodel.cpp \
    src/plugins/pluginmanager.cpp

maemo5 {
    QT += declarative
    
    INCLUDEPATH += src/maemo5
    
    HEADERS += \
        src/maemo5/definitions.h \
        src/maemo5/logger.h \
        src/maemo5/settings.h \
        src/maemo5/subscriptionsourcetypemodel.h
    
    SOURCES += \
        src/maemo5/logger.cpp \
        src/maemo5/main.cpp \
        src/maemo5/settings.cpp
    
    qml.files += \
        src/maemo5/qml/AboutDialog.qml \
        src/maemo5/qml/ArticleDelegate.qml \
        src/maemo5/qml/ArticlesWindow.qml \
        src/maemo5/qml/ArticleWindow.qml \
        src/maemo5/qml/DateSelectorButton.qml \
        src/maemo5/qml/DeleteDialog.qml \
        src/maemo5/qml/DownloadDialog.qml \
        src/maemo5/qml/EnclosureDelegate.qml \
        src/maemo5/qml/EnclosuresDialog.qml \
        src/maemo5/qml/ListSelectorButton.qml \
        src/maemo5/qml/LogDialog.qml \
        src/maemo5/qml/MainWindow.qml \
        src/maemo5/qml/PluginDialog.qml \
        src/maemo5/qml/PopupLoader.qml \
        src/maemo5/qml/SearchDialog.qml \
        src/maemo5/qml/SettingsDialog.qml \
        src/maemo5/qml/ShortcutsDialog.qml \
        src/maemo5/qml/SubscriptionDelegate.qml \
        src/maemo5/qml/SubscriptionDialog.qml \
        src/maemo5/qml/TimeSelectorButton.qml \
        src/maemo5/qml/TransferDelegate.qml \
        src/maemo5/qml/TransfersWindow.qml \
        src/maemo5/qml/UrlOpenerDelegate.qml \
        src/maemo5/qml/UrlOpenerDialog.qml \
        src/maemo5/qml/UrlOpenersDialog.qml
    
    qml.path = /opt/cutenews-client/qml
    
    desktop.files = desktop/maemo5/cutenews-client.desktop
    desktop.path = /usr/share/applications/hildon
        
    icon.files = desktop/maemo5/64/cutenews-client.png
    icon.path = /usr/share/icons/hicolor/64x64/apps
    
    target.path = /opt/cutenews-client/bin
    
    INSTALLS += \
        qml \
        desktop \
        icon \
        target

} else:symbian {    
    TARGET.UID3 = 0xD77EA21D
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000
    
    VERSION = 1.0.0
    ICON = desktop/symbian/cutenews-client.svg
    
    MMP_RULES += "DEBUGGABLE_UDEBONLY"
    
    QT += declarative
    CONFIG += qtcomponents
    
    INCLUDEPATH += src/symbian
    
    HEADERS += \
        src/symbian/clipboard.h \
        src/symbian/definitions.h \
        src/symbian/logger.h \
        src/symbian/screenorientationmodel.h \
        src/symbian/settings.h \
        src/symbian/subscriptionsourcetypemodel.h
    
    SOURCES += \
        src/symbian/clipboard.cpp \
        src/symbian/logger.cpp \
        src/symbian/main.cpp \
        src/symbian/settings.cpp
    
    qml.sources = \
        src/symbian/qml/AboutPage.qml \
        src/symbian/qml/AboutPluginPage.qml \
        src/symbian/qml/AboutPluginsPage.qml \
        src/symbian/qml/AppWindow.qml \
        src/symbian/qml/ArticleDelegate.qml \
        src/symbian/qml/ArticlePage.qml \
        src/symbian/qml/ArticlesPage.qml \
        src/symbian/qml/BackToolButton.qml \
        src/symbian/qml/ClientSettingsPage.qml \
        src/symbian/qml/CommandSubscriptionPage.qml \
        src/symbian/qml/DateSelector.qml \
        src/symbian/qml/DeletePage.qml \
        src/symbian/qml/DownloadPage.qml \
        src/symbian/qml/DualTextDelegate.qml \
        src/symbian/qml/EditPage.qml \
        src/symbian/qml/EnclosureDelegate.qml \
        src/symbian/qml/FileBrowserPage.qml \
        src/symbian/qml/HeaderLabel.qml \
        src/symbian/qml/ImportPage.qml \
        src/symbian/qml/KeyNavFlickable.qml \
        src/symbian/qml/LocalFileSubscriptionPage.qml \
        src/symbian/qml/LoggingSettingsPage.qml \
        src/symbian/qml/LogPage.qml \
        src/symbian/qml/main.qml \
        src/symbian/qml/MainPage.qml \
        src/symbian/qml/MyButton.qml \
        src/symbian/qml/MyContextMenu.qml \
        src/symbian/qml/MyDialog.qml \
        src/symbian/qml/MyFlickable.qml \
        src/symbian/qml/MyInfoBanner.qml \
        src/symbian/qml/MyListItem.qml \
        src/symbian/qml/MyListItemText.qml \
        src/symbian/qml/MyListView.qml \
        src/symbian/qml/MyMenu.qml \
        src/symbian/qml/MyPage.qml \
        src/symbian/qml/MyQueryDialog.qml \
        src/symbian/qml/MySearchBox.qml \
        src/symbian/qml/MySelectionDialog.qml \
        src/symbian/qml/MyStatusBar.qml \
        src/symbian/qml/MySwitch.qml \
        src/symbian/qml/MyTextField.qml \
        src/symbian/qml/MyToolButton.qml \
        src/symbian/qml/PluginSubscriptionPage.qml \
        src/symbian/qml/PopupLoader.qml \
        src/symbian/qml/ServerSettingsPage.qml \
        src/symbian/qml/SettingsPage.qml \
        src/symbian/qml/SubscriptionDelegate.qml \
        src/symbian/qml/TextDelegate.qml \
        src/symbian/qml/TextInputPage.qml \
        src/symbian/qml/TimeSelector.qml \
        src/symbian/qml/TransferDelegate.qml \
        src/symbian/qml/TransfersPage.qml \
        src/symbian/qml/UrlSubscriptionPage.qml \
        src/symbian/qml/ValueDialog.qml \
        src/symbian/qml/ValueListItem.qml \
        src/symbian/qml/ValueSelector.qml
    
    qml.path = !:/Private/d77ea21d/qml
    
    images.sources = \
        src/symbian/qml/images/close.svg \
        src/symbian/qml/images/cutenews-client.png \
        src/symbian/qml/images/document.svg \
        src/symbian/qml/images/download.svg \
        src/symbian/qml/images/favourite.svg \
        src/symbian/qml/images/folder.svg \
        src/symbian/qml/images/frame.png \
        src/symbian/qml/images/mask.png \
        src/symbian/qml/images/ok.svg \
        src/symbian/qml/images/read.svg \
        src/symbian/qml/images/up.png \
        src/symbian/qml/images/upload.svg 
    
    images.path = !:/Private/d77ea21d/qml/images
    
    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    qtcomponentsdep = "; Default dependency to Qt Quick Components for Symbian library" \
        "(0x200346DE), 1, 1, 0, {\"Qt Quick components for Symbian\"}"

    cutenewsclient_deployment.pkg_prerules += vendorinfo qtcomponentsdep
    
    DEPLOYMENT.display_name = cuteNews client
    
    DEPLOYMENT += \
        cutenewsclient_deployment \
        qml \
        images

} else:unix {
    lessThan(QT_MAJOR_VERSION, 5) {
        QT += webkit
    }
    else {
        QT += widgets webkitwidgets
    }
    
    INCLUDEPATH += src/desktop
    
    HEADERS += \
        src/desktop/aboutdialog.h \
        src/desktop/browser.h \
        src/desktop/clientsettingspage.h \
        src/desktop/customcommanddialog.h \
        src/desktop/definitions.h \
        src/desktop/logger.h \
        src/desktop/mainwindow.h \
        src/desktop/page.h \
        src/desktop/plugindialog.h \
        src/desktop/qdatetimedialog.h \
        src/desktop/serversettingspage.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/settingspage.h \
        src/desktop/subscriptiondialog.h \
        src/desktop/subscriptionsourcetypemodel.h \
        src/desktop/transferspage.h \
        src/desktop/urlopenersettingspage.h
    
    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/browser.cpp \
        src/desktop/clientsettingspage.cpp \
        src/desktop/customcommanddialog.cpp \
        src/desktop/logger.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/page.cpp \
        src/desktop/plugindialog.cpp \
        src/desktop/qdatetimedialog.cpp \
        src/desktop/serversettingspage.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/settingspage.cpp \
        src/desktop/subscriptiondialog.cpp \
        src/desktop/transferspage.cpp \
        src/desktop/urlopenersettingspage.cpp
    
    desktop.files = desktop/desktop/cutenews-client.desktop
    desktop.path = /usr/share/applications
    
    icon64.files = desktop/desktop/64/cutenews-client.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    
    icon48.files = desktop/desktop/48/cutenews-client.png
    icon48.path = /usr/share/icons/hicolor/48x48/apps
    
    icon22.files = desktop/desktop/22/cutenews-client.png
    icon22.path = /usr/share/icons/hicolor/22x22/apps
    
    icon16.files = desktop/desktop/16/cutenews-client.png
    icon16.path = /usr/share/icons/hicolor/16x16/apps
            
    target.path = /usr/bin
    
    INSTALLS += \
        desktop \
        icon64 \
        icon48 \
        icon22 \
        icon16 \
        target
}
