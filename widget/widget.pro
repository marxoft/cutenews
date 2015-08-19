TEMPLATE = subdirs

maemo5 {
    desktop.files = desktop/maemo5/cutenews.desktop
    desktop.path = /usr/share/applications/hildon-home
    
    qml.files = src/maemo5/qml/Widget.qml
    qml.path = /opt/cutenews/qml
    
    scripts.files = src/maemo5/scripts/showwidget
    scripts.path = /opt/cutenews/bin

    INSTALLS += desktop qml scripts
}
