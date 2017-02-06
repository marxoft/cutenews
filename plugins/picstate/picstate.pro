TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-picstate.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-picstate.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-picstate.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-picstate.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    picstate_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews PicState
    DEPLOYMENT += \
        picstate_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-picstate.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-picstate.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
