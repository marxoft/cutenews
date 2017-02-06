TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-sendvid.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-sendvid.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-sendvid.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-sendvid.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    sendvid_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Sendvid
    DEPLOYMENT += \
        sendvid_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-sendvid.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-sendvid.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
