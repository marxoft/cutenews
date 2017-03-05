QT += core network
CONFIG += plugin
TARGET = cutenews-youtube
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += \
    youtubeenclosurerequest.h \
    youtubefeedplugin.h \
    youtubefeedrequest.h

SOURCES += \
    youtubeenclosurerequest.cpp \
    youtubefeedrequest.cpp

maemo5 {
    LIBS += -L/opt/lib -lqyoutube
    CONFIG += link_prl
    PKGCONFIG += libqyoutube
    
    INCLUDEPATH += /usr/include/cutenews
    HEADERS += \
        /usr/include/cutenews/enclosurerequest.h \
        /usr/include/cutenews/feedplugin.h \
        /usr/include/cutenews/feedrequest.h
    
    config.files = "$$TARGET".json
    config.path = /opt/cutenews/plugins

    target.path = /opt/cutenews/plugins
    
    INSTALLS += \
        target \
        config

} else:unix {
    LIBS += -L/usr/lib -lqyoutube
    CONFIG += link_prl
    PKGCONFIG += libqyoutube
    
    INCLUDEPATH += /usr/include/cutenews
    HEADERS += \
        /usr/include/cutenews/enclosurerequest.h \
        /usr/include/cutenews/feedplugin.h \
        /usr/include/cutenews/feedrequest.h
    
    config.files = "$$TARGET".json
    config.path = /usr/share/cutenews/plugins

    target.path = /usr/share/cutenews/plugins
    
    INSTALLS += \
        target \
        config
}
