TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-rapidvideo.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-rapidvideo.js
    plugin.path = /opt/cutenews/plugins
    
    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-rapidvideo.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-rapidvideo.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    rapidvideo_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Rapidvideo
    DEPLOYMENT += \
        rapidvideo_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-rapidvideo.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-rapidvideo.js
    plugin.path = /usr/share/cutenews/plugins
    
    INSTALLS += \
        config \
        plugin
}
