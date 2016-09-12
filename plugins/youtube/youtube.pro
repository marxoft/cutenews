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

symbian {
    TARGET.UID3 = 0xA24B1C1A
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
    youtube_deployment.pkg_prerules += vendorinfo

    config.sources = "$$TARGET".json
    config.path = !:/cutenews/plugins

    lib.sources = "$$TARGET".dll
    lib.path = !:/sys/bin

    stub.sources = "$$TARGET".qtplugin
    stub.path = !:/cutenews/plugins

    DEPLOYMENT.display_name = cuteNews YouTube
    DEPLOYMENT += \
        youtube_deployment \
        config \
        lib \
        stub

} else:maemo5 {
    LIBS += -L/usr/lib -lqyoutube
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
