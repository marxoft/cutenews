TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-pixroute.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-pixroute.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-pixroute.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-pixroute.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    pixroute_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Pix Route
    DEPLOYMENT += \
        pixroute_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-pixroute.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-pixroute.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
