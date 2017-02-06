TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-openload.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-openload.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-openload.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-openload.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    openload_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Openload
    DEPLOYMENT += \
        openload_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-openload.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-openload.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
