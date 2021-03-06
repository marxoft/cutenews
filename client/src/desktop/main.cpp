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

#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "logger.h"
#include "mainwindow.h"
#include "pluginmanager.h"
#include "settings.h"
#include "subscriptions.h"
#include "transfermodel.h"
#include "urlopenermodel.h"
#include <QApplication>
#include <QIcon>
#if QT_VERSION < 0x050000
#include <QSsl>
#include <QSslConfiguration>
#endif

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("cutenews-client");
    app.setApplicationName("cutenews-client");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setWindowIcon(QIcon::fromTheme("cutenews-client"));
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
    
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

    QScopedPointer<DBNotify> notify(DBNotify::instance());
    QScopedPointer<PluginManager> plugins(PluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<Subscriptions> subscriptions(Subscriptions::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlOpenerModel> opener(UrlOpenerModel::instance());
    
    MainWindow window;
    window.show();
        
    return app.exec();
}
