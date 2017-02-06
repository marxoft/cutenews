TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-depicme.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-depicme.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-depicme.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-depicme.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    depicme_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Depic.me
    DEPLOYMENT += \
        depicme_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-depicme.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-depicme.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
