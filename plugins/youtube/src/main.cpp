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

#include "youtube.h"
#include <QCoreApplication>
#include <QStringList>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    YouTube youtube;
    
    const QStringList args = app.arguments();
    QString errorString;
    
    int idx = args.indexOf("-e");
    
    if (idx != -1) {
        const QString enclosure = args.value(idx + 1);
        
        if (enclosure.isEmpty()) {
            errorString = QObject::tr("No enclosure url specified");
        }
        else {
            QString format("18");
            idx = args.indexOf("-f");
            
            if (idx != -1) {
                const QString f = args.value(idx + 1);
                
                if (!f.isEmpty()) {
                    format = f;
                }
            }
            
            youtube.getVideoStreamUrl(enclosure, format);
            return app.exec();
        }
    }
    else {
        idx = args.indexOf("-q");
        errorString = QObject::tr("No query specified");
        
        if (idx != -1) {
            const QString query = args.value(idx + 1);

            if (!query.isEmpty()) {
                QString queryType("search");
                QString safeSearch("none");
                int maxResults = 20;
                
                idx = args.indexOf("-t");
                
                if (idx != -1) {
                    const QString t = args.value(idx + 1);
                    
                    if (!t.isEmpty()) {
                        queryType = t;
                    }
                }
                
                idx = args.indexOf("-s");
                
                if (idx != -1) {
                    const QString s = args.value(idx + 1);
                    
                    if (!s.isEmpty()) {
                        safeSearch = s;
                    }
                }
                
                idx = args.indexOf("-m");
                
                if (idx != -1) {
                    maxResults = qBound(5, args.value(idx + 1).toInt(), 50);
                }
                
                if (queryType == "channel") {
                    youtube.getChannelVideos(query, safeSearch, maxResults);
                }
                else {
                    youtube.searchVideos(query, safeSearch, maxResults);
                }
                
                return app.exec();
            }
        }
    }
    
    std::cout << QString("{\"error\": \"%1\"}").arg(errorString).toUtf8().constData();
    return 1;
}
