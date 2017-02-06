TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-dropbox.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-dropbox.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-dropbox.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-dropbox.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    dropbox_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews Dropbox
    DEPLOYMENT += \
        dropbox_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-dropbox.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-dropbox.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
