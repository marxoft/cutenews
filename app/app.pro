TEMPLATE = app
TARGET = cutenews

QT += network script sql

INCLUDEPATH += \
    src/base \
    src/plugins

HEADERS += \
    src/base/article.h \
    src/base/articlemodel.h \
    src/base/cachingnetworkaccessmanager.h \
    src/base/categorymodel.h \
    src/base/categorynamemodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/database.h \
    src/base/dbconnection.h \
    src/base/dbnotify.h \
    src/base/download.h \
    src/base/enclosuredownload.h \
    src/base/feedparser.h \
    src/base/json.h \
    src/base/logger.h \
    src/base/loggerverbositymodel.h \
    src/base/networkproxytypemodel.h \
    src/base/opmlparser.h \
    src/base/selectionmodel.h \
    src/base/subscription.h \
    src/base/subscriptionmodel.h \
    src/base/subscriptions.h \
    src/base/transfer.h \
    src/base/transfermodel.h \
    src/base/transferprioritymodel.h \
    src/base/transfers.h \
    src/base/updateintervaltypemodel.h \
    src/base/urlopenermodel.h \
    src/base/utils.h \
    src/plugins/enclosurerequest.h \
    src/plugins/externalenclosurerequest.h \
    src/plugins/externalfeedplugin.h \
    src/plugins/externalfeedrequest.h \
    src/plugins/feedplugin.h \
    src/plugins/feedpluginconfig.h \
    src/plugins/feedrequest.h \
    src/plugins/javascriptenclosurerequest.h \
    src/plugins/javascriptenclosurerequestglobalobject.h \
    src/plugins/javascriptfeedplugin.h \
    src/plugins/javascriptfeedrequest.h \
    src/plugins/javascriptfeedrequestglobalobject.h \
    src/plugins/pluginmanager.h \
    src/plugins/xmlhttprequest.h

SOURCES += \
    src/base/article.cpp \
    src/base/articlemodel.cpp \
    src/base/cachingnetworkaccessmanager.cpp \
    src/base/categorymodel.cpp \
    src/base/dbconnection.cpp \
    src/base/dbnotify.cpp \
    src/base/download.cpp \
    src/base/enclosuredownload.cpp \
    src/base/feedparser.cpp \
    src/base/json.cpp \
    src/base/logger.cpp \
    src/base/opmlparser.cpp \
    src/base/selectionmodel.cpp \
    src/base/subscription.cpp \
    src/base/subscriptionmodel.cpp \
    src/base/subscriptions.cpp \
    src/base/transfer.cpp \
    src/base/transfermodel.cpp \
    src/base/transfers.cpp \
    src/base/urlopenermodel.cpp \
    src/base/utils.cpp \
    src/plugins/externalenclosurerequest.cpp \
    src/plugins/externalfeedplugin.cpp \
    src/plugins/externalfeedrequest.cpp \
    src/plugins/feedpluginconfig.cpp \
    src/plugins/javascriptenclosurerequest.cpp \
    src/plugins/javascriptenclosurerequestglobalobject.cpp \
    src/plugins/javascriptfeedplugin.cpp \
    src/plugins/javascriptfeedrequest.cpp \
    src/plugins/javascriptfeedrequestglobalobject.cpp \
    src/plugins/pluginmanager.cpp \
    src/plugins/xmlhttprequest.cpp

maemo5 {
    DEFINES += \
        NO_SQLITE_FOREIGN_KEYS \
        DBUS_INTERFACE
    
    QT += declarative
    
    INCLUDEPATH += src/maemo5
    
    HEADERS += \
        src/maemo5/cachingnetworkaccessmanagerfactory.h \
        src/maemo5/cutenews.h \
        src/maemo5/definitions.h \
        src/maemo5/eventfeed.h \
        src/maemo5/settings.h \
        src/maemo5/subscriptionsourcetypemodel.h \
        src/maemo5/userinterfacemodel.h \
        src/maemo5/viewmodemodel.h
    
    SOURCES += \
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
        src/maemo5/qml/CategoriesDialog.qml \
        src/maemo5/qml/CategoryDelegate.qml \
        src/maemo5/qml/CategoryDialog.qml \
        src/maemo5/qml/EnclosureDelegate.qml \
        src/maemo5/qml/EnclosuresDialog.qml \
        src/maemo5/qml/FavouritesWindow.qml \
        src/maemo5/qml/ListSelectorButton.qml \
        src/maemo5/qml/LogDialog.qml \
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
        src/maemo5/qml/TransferDelegate.qml \
        src/maemo5/qml/TransferPropertiesDialog.qml \
        src/maemo5/qml/TransfersWindow.qml \
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

} else:unix {
    DEFINES += \
        USE_FAVICONS \
        WIDGETS_UI \
        DBUS_INTERFACE \
        WEB_INTERFACE
    
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
        src/desktop/customcommanddialog.h \
        src/desktop/cutenews.h \
        src/desktop/definitions.h \
        src/desktop/mainwindow.h \
        src/desktop/plugindialog.h \
        src/desktop/searchdialog.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/subscriptiondialog.h \
        src/desktop/subscriptionsourcetypemodel.h \
        src/desktop/transfersview.h
    
    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/browser.cpp \
        src/desktop/customcommanddialog.cpp \
        src/desktop/cutenews.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/plugindialog.cpp \
        src/desktop/searchdialog.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/subscriptiondialog.cpp \
        src/desktop/transfersview.cpp
    
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
    
    web_interface.files = webif
    web_interface.path = /usr/share/cutenews
        
    target.path = /usr/bin
    
    INSTALLS += \
        desktop \
        icon64 \
        icon48 \
        icon22 \
        icon16 \
        dbus_service \
        dbus_interface \
        web_interface
}

contains(DEFINES, DBUS_INTERFACE) {
    QT += dbus
}

contains(DEFINES, WEB_INTERFACE) {
    INCLUDEPATH += \
        src/qhttpserver \
        src/webif
    
    HEADERS += \
        src/qhttpserver/http_parser.h \
        src/qhttpserver/qhttpconnection.h \
        src/qhttpserver/qhttprequest.h \
        src/qhttpserver/qhttpresponse.h \
        src/qhttpserver/qhttpserver.h \
        src/qhttpserver/qhttpserverapi.h \
        src/qhttpserver/qhttpserverfwd.h \
        src/webif/articleserver.h \
        src/webif/fileserver.h \
        src/webif/pluginserver.h \
        src/webif/settingsserver.h \
        src/webif/transferserver.h \
        src/webif/subscriptionserver.h \
        src/webif/webserver.h
    
    SOURCES += \
        src/qhttpserver/http_parser.c \
        src/qhttpserver/qhttpconnection.cpp \
        src/qhttpserver/qhttprequest.cpp \
        src/qhttpserver/qhttpresponse.cpp \
        src/qhttpserver/qhttpserver.cpp \
        src/webif/articleserver.cpp \
        src/webif/fileserver.cpp \
        src/webif/pluginserver.cpp \
        src/webif/settingsserver.cpp \
        src/webif/subscriptionserver.cpp \
        src/webif/transferserver.cpp \
        src/webif/webserver.cpp
}

INSTALLS += target
