TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-imgcloud.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-imgcloud.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-imgcloud.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-imgcloud.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    imgcloud_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Image Cloud
    DEPLOYMENT += \
        imgcloud_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-imgcloud.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-imgcloud.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
