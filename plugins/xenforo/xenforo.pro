QT += core network
QT -= gui
CONFIG += plugin
TARGET = cutenews-xenforo
TEMPLATE = lib

DEFINES += XENFORO_DEBUG

HEADERS += \
    xenforofeedplugin.h \
    xenforofeedrequest.h

SOURCES += \
    xenforofeedrequest.cpp

symbian {
    TARGET.UID3 = 0xF1D13C2B
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../src
    HEADERS += \
        ../src/enclosurerequest.h \
        ../src/feedplugin.h \
        ../src/feedrequest.h
    
    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    xenforo_deployment.pkg_prerules += vendorinfo

    config.sources = "$$TARGET".json
    config.path = !:/cutenews/plugins

    lib.sources = "$$TARGET".dll
    lib.path = !:/sys/bin

    stub.sources = "$$TARGET".qtplugin
    stub.path = !:/cutenews/plugins

    DEPLOYMENT.display_name = cuteNews XenForo
    DEPLOYMENT += \
        xenforo_deployment \
        config \
        lib \
        stub

} else:maemo5 {
    CONFIG += link_prl
    LIBS += -L/usr/lib -lqhtmlparser
    PKGCONFIG += libqhtmlparser
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
    CONFIG += link_prl
    LIBS += -L/usr/lib -lqhtmlparser
    PKGCONFIG += libqhtmlparser
    INCLUDEPATH += ../src
    HEADERS += \
        ../src/enclosurerequest.h \
        ../src/feedplugin.h \
        ../src/feedrequest.h
    
    config.files = "$$TARGET".json
    config.path = /usr/share/cutenews/plugins

    target.path = /usr/share/cutenews/plugins
    
    INSTALLS += \
        target \
        config
}
