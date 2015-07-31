TEMPLATE = app
TARGET = cutenews

#DEFINES += CUTENEWS_DEBUG

QT += network sql xml

INCLUDEPATH += src/base

HEADERS += \
    src/base/article.h \
    src/base/articlemodel.h \
    src/base/database.h \
    src/base/json.h \
    src/base/networkproxytypemodel.h \
    src/base/selectionmodel.h \
    src/base/settings.h \
    src/base/subscription.h \
    src/base/subscriptionmodel.h \
    src/base/subscriptions.h \
    src/base/subscriptionsourcetypemodel.h \
    src/base/transfer.h \
    src/base/transfers.h \
    src/base/urlopener.h \
    src/base/urlopenermodel.h \
    src/base/utils.h \
    src/base/viewmodemodel.h

SOURCES += \
    src/base/article.cpp \
    src/base/articlemodel.cpp \
    src/base/database.cpp \
    src/base/json.cpp \
    src/base/selectionmodel.cpp \
    src/base/settings.cpp \
    src/base/subscription.cpp \
    src/base/subscriptionmodel.cpp \
    src/base/subscriptions.cpp \
    src/base/transfer.cpp \
    src/base/transfers.cpp \
    src/base/urlopener.cpp \
    src/base/utils.cpp

maemo5 {
    DEFINES += NO_SQLITE_FOREIGN_KEYS
    
    QT += declarative
    
    INCLUDEPATH += src/maemo5
    
    HEADERS += \
        src/base/transfermodel.h \
        src/maemo5/definitions.h \
        src/maemo5/userinterfacemodel.h
    
    SOURCES += \
        src/base/transfermodel.cpp \
        src/maemo5/main.cpp
    
    qml.files += \
        src/maemo5/qml/main.qml \
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
        src/maemo5/qml/NetworkProxyDialog.qml \
        src/maemo5/qml/OssoView.qml \
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
    
    target.path = /opt/cutenews/bin
    
    INSTALLS += qml desktop icon   
}

INSTALLS += target
