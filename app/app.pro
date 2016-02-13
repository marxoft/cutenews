TEMPLATE = app
TARGET = cutenews

#DEFINES += CUTENEWS_DEBUG

QT += network sql

INCLUDEPATH += src/base

HEADERS += \
    src/base/article.h \
    src/base/articlemodel.h \
    src/base/database.h \
    src/base/feedparser.h \
    src/base/json.h \
    src/base/networkproxytypemodel.h \
    src/base/opmlparser.h \
    src/base/selectionmodel.h \
    src/base/subscription.h \
    src/base/subscriptionmodel.h \
    src/base/subscriptionplugins.h \
    src/base/subscriptions.h \
    src/base/subscriptionsourcetypemodel.h \
    src/base/transfer.h \
    src/base/transfers.h \
    src/base/urlopenermodel.h \
    src/base/utils.h

SOURCES += \
    src/base/article.cpp \
    src/base/articlemodel.cpp \
    src/base/database.cpp \
    src/base/feedparser.cpp \
    src/base/json.cpp \
    src/base/opmlparser.cpp \
    src/base/selectionmodel.cpp \
    src/base/subscription.cpp \
    src/base/subscriptionmodel.cpp \
    src/base/subscriptionplugins.cpp \
    src/base/subscriptions.cpp \
    src/base/transfer.cpp \
    src/base/transfers.cpp \
    src/base/urlopenermodel.cpp \
    src/base/utils.cpp

maemo5 {
    DEFINES += NO_SQLITE_FOREIGN_KEYS
    
    QT += declarative dbus
    
    INCLUDEPATH += src/maemo5
    
    HEADERS += \
        src/base/transfermodel.h \
        src/maemo5/cutenews.h \
        src/maemo5/definitions.h \
        src/maemo5/eventfeed.h \
        src/maemo5/settings.h \
        src/maemo5/userinterfacemodel.h \
        src/maemo5/viewmodemodel.h
    
    SOURCES += \
        src/base/transfermodel.cpp \
        src/maemo5/cutenews.cpp \
        src/maemo5/eventfeed.cpp \
        src/maemo5/main.cpp \
        src/maemo5/settings.cpp
    
    qml.files += \
        src/maemo5/qml/AboutDialog.qml \
        src/maemo5/qml/AllArticlesWindow.qml \
        src/maemo5/qml/ArticleDelegate.qml \
        src/maemo5/qml/ArticlesWindow.qml \
        src/maemo5/qml/ArticleWindow.qml \
        src/maemo5/qml/DownloadDelegate.qml \
        src/maemo5/qml/DownloadPropertiesDialog.qml \
        src/maemo5/qml/DownloadsWindow.qml \
        src/maemo5/qml/EnclosureDelegate.qml \
        src/maemo5/qml/EnclosuresDialog.qml \
        src/maemo5/qml/FavouritesWindow.qml \
        src/maemo5/qml/ListSelectorButton.qml \
        src/maemo5/qml/MainWindow.qml \
        src/maemo5/qml/NetworkProxyDialog.qml \
        src/maemo5/qml/OssoView.qml \
        src/maemo5/qml/PluginDialog.qml \
        src/maemo5/qml/PopupLoader.qml \
        src/maemo5/qml/SearchDialog.qml \
        src/maemo5/qml/SearchWindow.qml \
        src/maemo5/qml/SettingsDialog.qml \
        src/maemo5/qml/SubscriptionDelegate.qml \
        src/maemo5/qml/SubscriptionDialog.qml \
        src/maemo5/qml/TouchView.qml \
        src/maemo5/qml/UrlOpenerDelegate.qml \
        src/maemo5/qml/UrlOpenerDialog.qml \
        src/maemo5/qml/UrlOpenersDialog.qml \
        src/maemo5/qml/Widget.qml
    
    qml.path = /opt/cutenews/qml
    
    desktop.files = desktop/maemo5/cutenews.desktop
    desktop.path = /usr/share/applications/hildon
    
    widget.files = desktop/widget/cutenews.desktop
    widget.path = /usr/share/applications/hildon-home
    
    icon.files = desktop/maemo5/64/cutenews.png
    icon.path = /usr/share/icons/hicolor/64x64/apps
    
    scripts.files = \
        src/maemo5/scripts/showwidget \
        src/maemo5/scripts/showwindow
    
    scripts.path = /opt/cutenews/bin
    
    dbus_service.files = \
        dbus/maemo5/org.marxoft.cutenews.service \
        dbus/maemo5/org.marxoft.cutenews.subscriptions.service
    
    dbus_service.path = /usr/share/dbus-1/services
    
    dbus_interface.files = \
        dbus/maemo5/org.marxoft.cutenews.xml \
        dbus/maemo5/org.marxoft.cutenews.subscriptions.xml
    
    dbus_interface.path = /usr/share/dbus-1/interfaces
    
    target.path = /opt/cutenews/bin
    
    INSTALLS += \
        qml \
        desktop \
        widget \
        icon \
        scripts \
        dbus_service \
        dbus_interface

} else:contains(MEEGO_EDITION,harmattan) {
    DEFINES += NO_SQLITE_FOREIGN_KEYS
    
    QT += declarative dbus opengl
    
    CONFIG += \
        qdeclarative-boostable \
        libtuiclient \
        shareuiinterface-maemo-meegotouch \
        share-ui-common \
        mdatauri
    
    INCLUDEPATH += src/harmattan
    
    HEADERS += \
        src/harmattan/activecolormodel.h \
        src/harmattan/cutenews.h \
        src/harmattan/definitions.h \
        src/harmattan/eventfeed.h \
        src/harmattan/maskeditem.h \
        src/harmattan/maskeffect.h \
        src/harmattan/screenorientationmodel.h \
        src/harmattan/settings.h \
        src/harmattan/shareui.h \
        src/harmattan/viewmodemodel.h
    
    SOURCES += \
        src/harmattan/cutenews.cpp \
        src/harmattan/eventfeed.cpp \
        src/harmattan/main.cpp \
        src/harmattan/maskeditem.cpp \
        src/harmattan/maskeffect.cpp \
        src/harmattan/settings.cpp \
        src/harmattan/shareui.cpp
    
    qml.files += \
        src/harmattan/qml/AboutPage.qml \
        src/harmattan/qml/AllArticlesPage.qml \
        src/harmattan/qml/AppearanceSettingsPage.qml \
        src/harmattan/qml/AppWindow.qml \
        src/harmattan/qml/ArticleDelegate.qml \
        src/harmattan/qml/ArticlePage.qml \
        src/harmattan/qml/ArticlesHeader.qml \
        src/harmattan/qml/ArticlesPage.qml \
        src/harmattan/qml/BackToolIcon.qml \
        src/harmattan/qml/ColorCountBubble.qml \
        src/harmattan/qml/DrillDownDelegate.qml \
        src/harmattan/qml/EnclosureDelegate.qml \
        src/harmattan/qml/FavouritesPage.qml \
        src/harmattan/qml/FileBrowserDelegate.qml \
        src/harmattan/qml/FileBrowserDialog.qml \
        src/harmattan/qml/GeneralSettingsPage.qml \
        src/harmattan/qml/Icon.qml \
        src/harmattan/qml/LabelDelegate.qml \
        src/harmattan/qml/ListItem.qml \
        src/harmattan/qml/main.qml \
        src/harmattan/qml/MainPage.qml \
        src/harmattan/qml/MyBusyIndicator.qml \
        src/harmattan/qml/MyButton.qml \
        src/harmattan/qml/MyCheckBox.qml \
        src/harmattan/qml/MyInfoBanner.qml \
        src/harmattan/qml/MyPage.qml \
        src/harmattan/qml/MyProgressBar.qml \
        src/harmattan/qml/MySelectionDialog.qml \
        src/harmattan/qml/MySheet.qml \
        src/harmattan/qml/MySlider.qml \
        src/harmattan/qml/MySwitch.qml \
        src/harmattan/qml/MyTextArea.qml \
        src/harmattan/qml/MyTextField.qml \
        src/harmattan/qml/MyToolIcon.qml \
        src/harmattan/qml/NetworkSettingsPage.qml \
        src/harmattan/qml/PageHeader.qml \
        src/harmattan/qml/PluginDialog.qml \
        src/harmattan/qml/SearchDialog.qml \
        src/harmattan/qml/SearchPage.qml \
        src/harmattan/qml/SeparatorLabel.qml \
        src/harmattan/qml/SettingsPage.qml \
        src/harmattan/qml/SubscriptionDelegate.qml \
        src/harmattan/qml/SubscriptionDialog.qml \
        src/harmattan/qml/ValueDialog.qml \
        src/harmattan/qml/ValueListItem.qml \
        src/harmattan/qml/ValueSelector.qml
    
    qml.path = /opt/cutenews/qml
    
    images.files = $$files(src/harmattan/qml/images/*.png)
    images.path = /opt/cutenews/qml/images
    
    desktop.files = desktop/harmattan/cutenews.desktop
    desktop.path = /usr/share/applications
    
    icon80.files = desktop/harmattan/80/cutenews.png
    icon80.path = /usr/share/icons/hicolor/80x80/apps
    
    icon64.files = desktop/harmattan/64/cutenews.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    
    #splash.files = desktop/harmattan/splash/*.png
    #splash.path = /opt/cutenews/splash
        
    dbus_service.files = \
        dbus/harmattan/org.marxoft.cutenews.service \
        dbus/harmattan/org.marxoft.cutenews.subscriptions.service
    
    dbus_service.path = /usr/share/dbus-1/services
    
    dbus_interface.files = \
        dbus/harmattan/org.marxoft.cutenews.xml \
        dbus/harmattan/org.marxoft.cutenews.subscriptions.xml
    
    dbus_interface.path = /usr/share/dbus-1/interfaces
    
    target.path = /opt/cutenews/bin
    
    INSTALLS += \
        qml \
        images \
        desktop \
        icon80 \
        icon64 \
        #splash \
        dbus_service \
        dbus_interface
        
} else:unix {
    DEFINES += USE_FAVICONS WIDGETS_UI
    
    QT += dbus
    
    lessThan(QT_MAJOR_VERSION,5) {
        QT += webkit
    }
    else {
        QT += widgets webkitwidgets
    }
    
    INCLUDEPATH += src/desktop
    
    HEADERS += \
        src/base/transfermodel.h \
        src/desktop/browser.h \
        src/desktop/cutenews.h \
        src/desktop/definitions.h \
        src/desktop/downloadsview.h \
        src/desktop/mainwindow.h \
        src/desktop/plugindialog.h \
        src/desktop/searchdialog.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/subscriptiondialog.h
    
    SOURCES += \
        src/base/transfermodel.cpp \
        src/desktop/browser.cpp \
        src/desktop/cutenews.cpp \
        src/desktop/downloadsview.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/plugindialog.cpp \
        src/desktop/searchdialog.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/subscriptiondialog.cpp
    
    desktop.files = desktop/desktop/cutenews.desktop
    desktop.path = /usr/share/applications
    
    icon64.files = desktop/desktop/64/cutenews.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    
    icon48.files = desktop/desktop/48/cutenews.png
    icon48.path = /usr/share/icons/hicolor/48x48/apps
    
    icon22.files = desktop/desktop/22/cutenews.png
    icon22.path = /usr/share/icons/hicolor/22x22/apps
    
    icon16.files = desktop/desktop/16/cutenews.png
    icon16.path = /usr/share/icons/hicolor/16x16/apps
    
    dbus_service.files = \
        dbus/desktop/org.marxoft.cutenews.service \
        dbus/desktop/org.marxoft.cutenews.subscriptions.service
    
    dbus_service.path = /usr/share/dbus-1/services
    
    dbus_interface.files = \
        dbus/desktop/org.marxoft.cutenews.xml \
        dbus/desktop/org.marxoft.cutenews.subscriptions.xml
    
    dbus_interface.path = /usr/share/dbus-1/interfaces
    
    target.path = /opt/cutenews/bin
    
    INSTALLS += \
        desktop \
        icon64 \
        icon48 \
        icon22 \
        icon16 \
        dbus_service \
        dbus_interface
}

INSTALLS += target
