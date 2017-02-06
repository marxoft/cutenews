TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-imagetwist.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-imagetwist.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-imagetwist.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-imagetwist.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    imagetwist_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews ImageTwist
    DEPLOYMENT += \
        imagetwist_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-imagetwist.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-imagetwist.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
