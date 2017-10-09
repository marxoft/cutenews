/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef ENCLOSURESERVER_H
#define ENCLOSURESERVER_H

#include <QObject>
#include <QHash>

class EnclosureRequest;
class QHttpRequest;
class QHttpResponse;

class EnclosureServer : public QObject
{
    Q_OBJECT

public:
    explicit EnclosureServer(QObject *parent = 0);
    
    bool handleRequest(QHttpRequest *request, QHttpResponse *response);

private Q_SLOTS:
    void onEnclosureRequestFinished(EnclosureRequest *request);
    void onResponseDone();

private:
    void addResponse(EnclosureRequest *request, QHttpResponse *response);
    QHttpResponse* getResponse(EnclosureRequest *request);
    void removeResponse(QHttpResponse *response);
    
    QHash<EnclosureRequest*, QHttpResponse*> m_responses;
};

#endif // ENCLOSURESERVER_H
