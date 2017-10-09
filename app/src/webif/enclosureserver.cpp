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

#include "enclosureserver.h"
#include "json.h"
#include "pluginmanager.h"
#include "pluginsettings.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "serverresponse.h"
#include "utils.h"

static QVariantMap enclosureResultToMap(const EnclosureResult &result) {
    QVariantMap enclosure;
    enclosure["fileName"] = result.fileName;
    enclosure["operation"] = result.operation;
    enclosure["data"] = result.data;
    QVariantMap request;
    request["url"] = result.request.url();
    QVariantMap headers;

    foreach (const QByteArray &header, result.request.rawHeaderList()) {
        headers[header] = result.request.rawHeader(header);
    }

    enclosure["request"] = request;
    return enclosure;
}

EnclosureServer::EnclosureServer(QObject *parent) :
    QObject(parent)
{
}

bool EnclosureServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    const QStringList parts = request->path().split("/", QString::SkipEmptyParts);
    
    if ((parts.size() != 2) || (parts.first().compare("enclosures", Qt::CaseInsensitive) != 0)) {
        return false;
    }

    if (request->method() == QHttpRequest::HTTP_GET) {
        const QString url = QString::fromUtf8(QByteArray::fromBase64(parts.at(1).toUtf8()));
        FeedPluginConfig *config = PluginManager::instance()->getConfigForEnclosure(url);

        if (config) {
            EnclosureRequest *request = PluginManager::instance()->enclosureRequest(url, this);

            if (request) {
                addResponse(request, response);
                PluginSettings settings(config->id());
                request->getEnclosure(url, settings.values());
                return true;
            }
        }

        writeResponse(response, QHttpResponse::STATUS_BAD_REQUEST);
        return true;
    }
    
    writeResponse(response, QHttpResponse::STATUS_METHOD_NOT_ALLOWED);
    return true;
}

void EnclosureServer::addResponse(EnclosureRequest *request, QHttpResponse *response) {
    m_responses.insert(request, response);
    connect(request, SIGNAL(finished(EnclosureRequest*)), this, SLOT(onEnclosureRequestFinished(EnclosureRequest*)));
    connect(response, SIGNAL(done()), this, SLOT(onResponseDone()));
}

QHttpResponse* EnclosureServer::getResponse(EnclosureRequest *request) {
    return m_responses.value(request);
}

void EnclosureServer::removeResponse(QHttpResponse *response) {
    if (EnclosureRequest *request = m_responses.key(response)) {
        m_responses.remove(request);
        request->deleteLater();
        disconnect(response, 0, this, 0);
    }
}

void EnclosureServer::onEnclosureRequestFinished(EnclosureRequest *request) {
    if (QHttpResponse *response = getResponse(request)) {
        if (request->status() == EnclosureRequest::Ready) {
            writeResponse(response, QHttpResponse::STATUS_OK,
                    QtJson::Json::serialize(enclosureResultToMap(request->result())));
        }
        else {
            writeResponse(response, QHttpResponse::STATUS_INTERNAL_SERVER_ERROR);
        }
    }
}

void EnclosureServer::onResponseDone() {
    if (QHttpResponse *response = qobject_cast<QHttpResponse*>(sender())) {
        removeResponse(response);
    }
}
