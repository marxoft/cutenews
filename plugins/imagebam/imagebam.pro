TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-imagebam.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-imagebam.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-imagebam.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-imagebam.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    imagebam_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews ImageBam
    DEPLOYMENT += \
        imagebam_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-imagebam.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-imagebam.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
