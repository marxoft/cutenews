/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cutenews.h"
#include "database.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "logger.h"
#include "pluginmanager.h"
#include "settings.h"
#include "subscriptions.h"
#include "transfers.h"
#include "urlopenermodel.h"
#include "webserver.h"
#include <QApplication>
#include <QIcon>
#include <QThread>
#if QT_VERSION < 0x050000
#include <QSsl>
#include <QSslConfiguration>
#endif

void registerTypes() {
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("cuteNews");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setWindowIcon(QIcon::fromTheme("cutenews"));
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
    app.setQuitOnLastWindowClosed(false);
    
    const QStringList args = app.arguments();
    const int verbosity = args.indexOf("-v") + 1;
    
    if ((verbosity > 1) && (verbosity < args.size())) {
        Logger::setVerbosity(qMax(1, args.at(verbosity).toInt()));
    }
    else {
        Logger::setFileName(Settings::loggerFileName());
        Logger::setVerbosity(Settings::loggerVerbosity());
    }

#if QT_VERSION < 0x050000
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
#endif
    
    initDatabase();
    registerTypes();

    QScopedPointer<CuteNews> cutenews(CuteNews::instance());
    QScopedPointer<DBNotify> notify(DBNotify::instance());
    QScopedPointer<PluginManager> plugins(PluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<Transfers> transfers(Transfers::instance());
    QScopedPointer<UrlOpenerModel> opener(UrlOpenerModel::instance());
    QScopedPointer<WebServer> server(WebServer::instance());
        
    QThread thread;
    thread.start();
    DBConnection::setAsynchronousThread(&thread);
    
    Settings::setNetworkProxy();
    subscriptions.data()->setOfflineModeEnabled(Settings::offlineModeEnabled());
    server.data()->setPort(Settings::webInterfacePort());
    server.data()->setUsername(Settings::webInterfaceUsername());
    server.data()->setPassword(Settings::webInterfacePassword());
    server.data()->setAuthenticationEnabled(Settings::webInterfaceAuthenticationEnabled());
    plugins.data()->load();
    transfers.data()->load();
    opener.data()->load();
    
    if ((Settings::updateSubscriptionsOnStartup()) && (!Settings::offlineModeEnabled())) {
        subscriptions.data()->updateAll();
    }
    
    if (Settings::webInterfaceEnabled()) {
        server.data()->start();
    }
        
    if (!args.contains("--nogui")) {
        cutenews.data()->showWindow();
    }

    QObject::connect(settings.data(), SIGNAL(offlineModeEnabledChanged(bool)),
                     subscriptions.data(), SLOT(setOfflineModeEnabled(bool)));
    QObject::connect(settings.data(), SIGNAL(maximumConcurrentTransfersChanged(int)),
                     transfers.data(), SLOT(setMaximumConcurrentTransfers(int)));
    QObject::connect(settings.data(), SIGNAL(webInterfaceAuthenticationEnabledChanged(bool)),
                     server.data(), SLOT(setAuthenticationEnabled(bool)));
    QObject::connect(settings.data(), SIGNAL(webInterfaceUsernameChanged(QString)),
                     server.data(), SLOT(setUsername(QString)));
    QObject::connect(settings.data(), SIGNAL(webInterfacePasswordChanged(QString)),
                     server.data(), SLOT(setPassword(QString)));
    QObject::connect(settings.data(), SIGNAL(webInterfacePortChanged(int)), server.data(), SLOT(setPort(int)));
    QObject::connect(settings.data(), SIGNAL(webInterfaceEnabledChanged(bool)),
                     server.data(), SLOT(setRunning(bool)));
    QObject::connect(&app, SIGNAL(lastWindowClosed()), cutenews.data(), SLOT(quit()));
    
    return app.exec();
}
