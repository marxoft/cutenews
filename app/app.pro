TEMPLATE = app
TARGET = cutenews

DEFINES += CUTENEWS_DEBUG

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
    src/base/settings.h \
    src/base/subscription.h \
    src/base/subscriptionmodel.h \
    src/base/subscriptionplugins.h \
    src/base/subscriptions.h \
    src/base/subscriptionsourcetypemodel.h \
    src/base/transfer.h \
    src/base/transfers.h \
    src/base/urlopenermodel.h \
    src/base/utils.h \
    src/base/viewmodemodel.h

SOURCES += \
    src/base/article.cpp \
    src/base/articlemodel.cpp \
    src/base/database.cpp \
    src/base/feedparser.cpp \
    src/base/json.cpp \
    src/base/opmlparser.cpp \
    src/base/selectionmodel.cpp \
    src/base/settings.cpp \
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
        src/maemo5/userinterfacemodel.h
    
    SOURCES += \
        src/base/transfermodel.cpp \
        src/maemo5/cutenews.cpp \
        src/maemo5/main.cpp
    
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
        src/maemo5/qml/MainWindow.qml \
        src/maemo5/qml/NetworkProxyDialog.qml \
        src/maemo5/qml/OssoView.qml \
        src/maemo5/qml/PluginDialog.qml \
        src/maemo5/qml/SearchDialog.qml \
        src/maemo5/qml/SearchWindow.qml \
        src/maemo5/qml/SettingsDialog.qml \
        src/maemo5/qml/SubscriptionDelegate.qml \
        src/maemo5/qml/SubscriptionDialog.qml \
        src/maemo5/qml/TouchView.qml \
        src/maemo5/qml/UrlOpenerDelegate.qml \
        src/maemo5/qml/UrlOpenerDialog.qml \
        src/maemo5/qml/UrlOpenersDialog.qml
    
    qml.path = /opt/cutenews/qml
    
    desktop.files = desktop/maemo5/cutenews.desktop
    desktop.path = /usr/share/applications/hildon
    
    icon.files = desktop/maemo5/64/cutenews.png
    icon.path = /usr/share/icons/hicolor/64x64/apps
    
    scripts.files = src/maemo5/scripts/showwindow
    scripts.path = /opt/cutenews/bin
    
    dbus_service.files = \
        dbus/org.marxoft.cutenews.service \
        dbus/org.marxoft.cutenews.subscriptions.service
    
    dbus_service.path = /usr/share/dbus-1/services
    
    dbus_interface.files = \
        dbus/org.marxoft.cutenews.xml \
        dbus/org.marxoft.cutenews.subscriptions.xml
    
    dbus_interface.path = /usr/share/dbus-1/interfaces
    
    target.path = /opt/cutenews/bin
    
    INSTALLS += \
        qml \
        desktop \
        icon \
        scripts \
        dbus_service \
        dbus_interface

} else:unix {
    DEFINES += USE_FAVICONS WIDGETS_UI
    
    QT += dbus widgets webkitwidgets
    
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
        dbus/org.marxoft.cutenews.service \
        dbus/org.marxoft.cutenews.subscriptions.service
    
    dbus_service.path = /usr/share/dbus-1/services
    
    target.path = /opt/cutenews/bin
    
    INSTALLS += \
        desktop \
        icon64 \
        icon48 \
        icon22 \
        icon16 \
        dbus_service
}

INSTALLS += target
