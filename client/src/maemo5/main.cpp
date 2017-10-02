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
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "logger.h"
#include "pluginmanager.h"
#include "serversettings.h"
#include "settings.h"
#include "subscriptions.h"
#include "transfermodel.h"
#include "urlopenermodel.h"
#include <QApplication>
#include <QSsl>
#include <QSslConfiguration>

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cutenews-client");
    app.setApplicationName("cutenews-client");
    app.setApplicationVersion(VERSION_NUMBER);
    
    const QStringList args = app.arguments();
    const int verbosity = args.indexOf("-v") + 1;
    
    if ((verbosity > 1) && (verbosity < args.size())) {
        Logger::setVerbosity(qMax(1, args.at(verbosity).toInt()));
    }
    else {
        Logger::setFileName(Settings::loggerFileName());
        Logger::setVerbosity(Settings::loggerVerbosity());
    }
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);
    
    QScopedPointer<CuteNews> cutenews(CuteNews::instance());
    QScopedPointer<DBNotify> notify(DBNotify::instance());
    QScopedPointer<PluginManager> plugins(PluginManager::instance());
    QScopedPointer<ServerSettings> serversettings(ServerSettings::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlOpenerModel> opener(UrlOpenerModel::instance());

    cutenews.data()->loadData();
    opener.data()->load();

    if (args.contains("--window")) {
        cutenews.data()->showWindow();
    }
    
    if (args.contains("--widget")) {
        cutenews.data()->showWidget();
    }

    QObject::connect(settings.data(), SIGNAL(serverAddressChanged(QString)), cutenews.data(), SLOT(loadData()));
    return app.exec();
}
