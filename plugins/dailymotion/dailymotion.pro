TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-dailymotion.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-dailymotion.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-dailymotion.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-dailymotion.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    dailymotion_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Dailymotion
    DEPLOYMENT += \
        dailymotion_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-dailymotion.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-dailymotion.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
