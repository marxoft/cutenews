QT += core network
QT -= gui
CONFIG += plugin
TARGET = cutenews-nytimes
TEMPLATE = lib

HEADERS += \
    feedparser.h \
    json.h \
    nytimesarticlerequest.h \
    nytimesenclosurerequest.h \
    nytimesfeedplugin.h \
    nytimesfeedrequest.h

SOURCES += \
    feedparser.cpp \
    json.cpp \
    nytimesarticlerequest.cpp \
    nytimesenclosurerequest.cpp \
    nytimesfeedrequest.cpp

maemo5 {
    CONFIG += link_prl
    LIBS += -L/opt/lib -lqhtmlparser
    PKGCONFIG += libqhtmlparser
    INCLUDEPATH += /usr/include/cutenews
    HEADERS += \
        /usr/include/cutenews/articlerequest.h \
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
    CONFIG += link_prl
    LIBS += -L/usr/lib -lqhtmlparser
    PKGCONFIG += libqhtmlparser
    INCLUDEPATH += /usr/include/cutenews
    HEADERS += \
        /usr/include/cutenews/articlerequest.h \
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
