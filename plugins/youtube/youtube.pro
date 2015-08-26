TEMPLATE = app
TARGET = youtube
QT += network
LIBS += -L/usr/lib -lqyoutube
CONFIG += link_prl
PKGCONFIG += libqyoutube

HEADERS += src/youtube.h
SOURCES += src/youtube.cpp src/main.cpp

target.path = /opt/cutenews/plugins/youtube

plugin.files = youtube.plugin
plugin.path = /opt/cutenews/plugins

INSTALLS += target plugin
