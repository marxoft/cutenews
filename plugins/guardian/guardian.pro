QT += core network
QT -= gui
CONFIG += plugin
TARGET = cutenews-guardian
TEMPLATE = lib

DEFINES += GUARDIAN_DEBUG

HEADERS += \
    feedparser.h \
    guardianfeedplugin.h \
    guardianfeedrequest.h

SOURCES += \
    feedparser.cpp \
    guardianfeedrequest.cpp

symbian {
    TARGET.UID3 = 0xB2F4AC1B
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
    guardian_deployment.pkg_prerules += vendorinfo

    config.sources = "$$TARGET".json
    config.path = !:/cutenews/plugins

    lib.sources = "$$TARGET".dll
    lib.path = !:/sys/bin

    stub.sources = "$$TARGET".qtplugin
    stub.path = !:/cutenews/plugins

    DEPLOYMENT.display_name = cuteNews The Guardian
    DEPLOYMENT += \
        guardian_deployment \
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
