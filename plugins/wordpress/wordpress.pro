TEMPLATE = subdirs

maemo5 {
    config.files = cutenews-wordpress.json
    config.path = /opt/cutenews/plugins

    plugin.files = cutenews-wordpress.js
    plugin.path = /opt/cutenews/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = cutenews-wordpress.json
    config.path = !:/cutenews/plugins

    plugin.sources = cutenews-wordpress.js
    plugin.path = !:/cutenews/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    wordpress_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = cuteNews WordPress
    DEPLOYMENT += \
        wordpress_deployment \
        config \
        plugin

} else:unix {
    config.files = cutenews-wordpress.json
    config.path = /usr/share/cutenews/plugins

    plugin.files = cutenews-wordpress.js
    plugin.path = /usr/share/cutenews/plugins

    INSTALLS += \
        config \
        plugin
}
