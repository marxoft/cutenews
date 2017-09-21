TEMPLATE = app
TARGET = cutenews

QT += network script sql

INCLUDEPATH += \
    src/base \
    src/plugins

HEADERS += \
    src/base/article.h \
    src/base/articlemodel.h \
    src/base/categorymodel.h \
    src/base/categorynamemodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/database.h \
    src/base/dbconnection.h \
    src/base/dbnotify.h \
    src/base/diskcache.h \
    src/base/download.h \
    src/base/enclosuredownload.h \
    src/base/feedparser.h \
    src/base/json.h \
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
    src/plugins/articlerequest.h \
    src/plugins/enclosurerequest.h \
    src/plugins/externalarticlerequest.h \
    src/plugins/externalenclosurerequest.h \
    src/plugins/externalfeedplugin.h \
    src/plugins/externalfeedrequest.h \
    src/plugins/feedplugin.h \
    src/plugins/feedpluginconfig.h \
    src/plugins/feedrequest.h \
    src/plugins/javascriptarticlerequest.h \
    src/plugins/javascriptenclosurerequest.h \
    src/plugins/javascriptfeedplugin.h \
    src/plugins/javascriptfeedrequest.h \
    src/plugins/javascriptglobalobject.h \
    src/plugins/pluginconfigmodel.h \
    src/plugins/pluginmanager.h \
    src/plugins/pluginsettings.h \
    src/plugins/xmlhttprequest.h

SOURCES += \
    src/base/article.cpp \
    src/base/articlemodel.cpp \
    src/base/categorymodel.cpp \
    src/base/dbconnection.cpp \
    src/base/dbnotify.cpp \
    src/base/diskcache.cpp \
    src/base/download.cpp \
    src/base/enclosuredownload.cpp \
    src/base/feedparser.cpp \
    src/base/json.cpp \
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
    src/plugins/externalarticlerequest.cpp \
    src/plugins/externalenclosurerequest.cpp \
    src/plugins/externalfeedplugin.cpp \
    src/plugins/externalfeedrequest.cpp \
    src/plugins/feedpluginconfig.cpp \
    src/plugins/javascriptarticlerequest.cpp \
    src/plugins/javascriptenclosurerequest.cpp \
    src/plugins/javascriptfeedplugin.cpp \
    src/plugins/javascriptfeedrequest.cpp \
    src/plugins/javascriptglobalobject.cpp \
    src/plugins/pluginconfigmodel.cpp \
    src/plugins/pluginmanager.cpp \
    src/plugins/pluginsettings.cpp \
    src/plugins/xmlhttprequest.cpp

maemo5 {
    DEFINES += \
        NO_SQLITE_FOREIGN_KEYS \
        DBUS_INTERFACE
    
    QT += declarative
    
    INCLUDEPATH += src/maemo5
    
    HEADERS += \
        src/maemo5/cachingnetworkaccessmanager.h \
        src/maemo5/cachingnetworkaccessmanagerfactory.h \
        src/maemo5/cutenews.h \
        src/maemo5/definitions.h \
        src/maemo5/eventfeed.h \
        src/maemo5/logger.h \
        src/maemo5/settings.h \
        src/maemo5/subscriptionsourcetypemodel.h
    
    SOURCES += \
        src/maemo5/cachingnetworkaccessmanager.cpp \
        src/maemo5/cutenews.cpp \
        src/maemo5/eventfeed.cpp \
        src/maemo5/logger.cpp \
        src/maemo5/main.cpp \
        src/maemo5/settings.cpp
    
    qml.files += \
        src/maemo5/qml/AboutDialog.qml \
        src/maemo5/qml/ArticleDelegate.qml \
        src/maemo5/qml/ArticleRequestWindow.qml \
        src/maemo5/qml/ArticleSettingsDialog.qml \
        src/maemo5/qml/ArticlesWindow.qml \
        src/maemo5/qml/ArticleWindow.qml \
        src/maemo5/qml/CategoriesDialog.qml \
        src/maemo5/qml/CategoryDelegate.qml \
        src/maemo5/qml/CategoryDialog.qml \
        src/maemo5/qml/DateSelectorButton.qml \
        src/maemo5/qml/DeleteDialog.qml \
        src/maemo5/qml/DownloadDialog.qml \
        src/maemo5/qml/EnclosureDelegate.qml \
        src/maemo5/qml/EnclosuresDialog.qml \
        src/maemo5/qml/EnclosureSettingsDialog.qml \
        src/maemo5/qml/ListSelectorButton.qml \
        src/maemo5/qml/LogDialog.qml \
        src/maemo5/qml/MainWindow.qml \
        src/maemo5/qml/NetworkProxyDialog.qml \
        src/maemo5/qml/OpenDialog.qml \
        src/maemo5/qml/PluginDialog.qml \
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
        dbus_interface \
        target

} else:symbian {
    DEFINES += NO_SQLITE_FOREIGN_KEYS
    
    TARGET.UID3 = 0xC77EA21C
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000
    
    VERSION = 1.1.0
    ICON = desktop/symbian/cutenews.svg
    
    MMP_RULES += "DEBUGGABLE_UDEBONLY"
    
    QT += declarative
    CONFIG += qtcomponents
    
    INCLUDEPATH += src/symbian
    
    HEADERS += \
        src/symbian/cachingnetworkaccessmanager.h \
        src/symbian/cachingnetworkaccessmanagerfactory.h \
        src/symbian/clipboard.h \
        src/symbian/cutenews.h \
        src/symbian/definitions.h \
        src/symbian/logger.h \
        src/symbian/maskeditem.h \
        src/symbian/maskeffect.h \
        src/symbian/screenorientationmodel.h \
        src/symbian/settings.h \
        src/symbian/subscriptionsourcetypemodel.h
    
    SOURCES += \
        src/symbian/cachingnetworkaccessmanager.cpp \
        src/symbian/clipboard.cpp \
        src/symbian/cutenews.cpp \
        src/symbian/logger.cpp \
        src/symbian/main.cpp \
        src/symbian/maskeditem.cpp \
        src/symbian/maskeffect.cpp \
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
        src/symbian/qml/CategorySettingsPage.qml \
        src/symbian/qml/CommandSubscriptionPage.qml \
        src/symbian/qml/DateSelector.qml \
        src/symbian/qml/DeletePage.qml \
        src/symbian/qml/DownloadPage.qml \
        src/symbian/qml/DualTextDelegate.qml \
        src/symbian/qml/EditCategoryPage.qml \
        src/symbian/qml/EditPage.qml \
        src/symbian/qml/EnclosureDelegate.qml \
        src/symbian/qml/FileBrowserPage.qml \
        src/symbian/qml/GeneralSettingsPage.qml \
        src/symbian/qml/HeaderLabel.qml \
        src/symbian/qml/IconImage.qml \
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
        src/symbian/qml/NetworkSettingsPage.qml \
        src/symbian/qml/PluginSettingsPage.qml \
        src/symbian/qml/PluginsSettingsPage.qml \
        src/symbian/qml/PluginSubscriptionPage.qml \
        src/symbian/qml/PopupLoader.qml \
        src/symbian/qml/SettingsPage.qml \
        src/symbian/qml/SubscriptionDelegate.qml \
        src/symbian/qml/TextDelegate.qml \
        src/symbian/qml/TextInputPage.qml \
        src/symbian/qml/TimeSelector.qml \
        src/symbian/qml/TransferDelegate.qml \
        src/symbian/qml/TransferSettingsPage.qml \
        src/symbian/qml/TransfersPage.qml \
        src/symbian/qml/UrlSubscriptionPage.qml \
        src/symbian/qml/ValueDialog.qml \
        src/symbian/qml/ValueListItem.qml \
        src/symbian/qml/ValueSelector.qml
    
    qml.path = !:/Private/c77ea21c/qml
    
    images.sources = \
        src/symbian/qml/images/close.svg \
        src/symbian/qml/images/cutenews.png \
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
    
    images.path = !:/Private/c77ea21c/qml/images
    
    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    qtcomponentsdep = "; Default dependency to Qt Quick Components for Symbian library" \
        "(0x200346DE), 1, 1, 0, {\"Qt Quick components for Symbian\"}"

    cutenews_deployment.pkg_prerules += vendorinfo qtcomponentsdep
    
    DEPLOYMENT.display_name = cuteNews
    
    DEPLOYMENT += \
        cutenews_deployment \
        qml \
        images

} else:unix {
    DEFINES += \
        USE_FAVICONS \
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
        src/desktop/articlesettingspage.h \
        src/desktop/browserpage.h \
        src/desktop/cachingnetworkaccessmanager.h \
        src/desktop/categorysettingspage.h \
        src/desktop/customcommanddialog.h \
        src/desktop/cutenews.h \
        src/desktop/definitions.h \
        src/desktop/enclosuresettingspage.h \
        src/desktop/generalsettingspage.h \
        src/desktop/interfacesettingspage.h \
        src/desktop/logger.h \
        src/desktop/mainwindow.h \
        src/desktop/networksettingspage.h \
        src/desktop/page.h \
        src/desktop/plugindialog.h \
        src/desktop/pluginsettingspage.h \
        src/desktop/qdatetimedialog.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/settingspage.h \
        src/desktop/subscriptiondialog.h \
        src/desktop/subscriptionsourcetypemodel.h \
        src/desktop/transferspage.h \
        src/desktop/urlopenersettingspage.h
    
    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/articlesettingspage.cpp \
        src/desktop/browserpage.cpp \
        src/desktop/cachingnetworkaccessmanager.cpp \
        src/desktop/categorysettingspage.cpp \
        src/desktop/customcommanddialog.cpp \
        src/desktop/cutenews.cpp \
        src/desktop/enclosuresettingspage.cpp \
        src/desktop/generalsettingspage.cpp \
        src/desktop/interfacesettingspage.cpp \
        src/desktop/logger.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/networksettingspage.cpp \
        src/desktop/page.cpp \
        src/desktop/plugindialog.cpp \
        src/desktop/pluginsettingspage.cpp \
        src/desktop/qdatetimedialog.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/settingspage.cpp \
        src/desktop/subscriptiondialog.cpp \
        src/desktop/transferspage.cpp \
        src/desktop/urlopenersettingspage.cpp
    
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
        web_interface \
        target
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
        src/webif/serverresponse.h \
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
