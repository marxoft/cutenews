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

#include "webserver.h"
#include "articleserver.h"
#include "definitions.h"
#include "enclosureserver.h"
#include "fileserver.h"
#include "pluginserver.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpserver.h"
#include "settingsserver.h"
#include "subscriptionserver.h"
#include "transferserver.h"

WebServer* WebServer::self = 0;

WebServer::WebServer() :
    QObject(),
    m_server(0),
    m_articleServer(0),
    m_enclosureServer(0),
    m_subscriptionServer(0),
    m_fileServer(0),
    m_port(8080),
    m_authenticationEnabled(false),
    m_status(Idle)
{
}

WebServer::~WebServer() {
    self = 0;
}

WebServer* WebServer::instance() {
    return self ? self : self = new WebServer;
}

QString WebServer::address() const {
    return m_server ? m_server->serverAddress().toString() : QString();
}

int WebServer::port() const {
    return m_port;
}

void WebServer::setPort(int p) {
    if (p != port()) {
        m_port = p;
        emit portChanged();
        
        if (isRunning()) {
            stop();
            start();
        }
    }
}

bool WebServer::authenticationEnabled() const {
    return m_authenticationEnabled;
}

void WebServer::setAuthenticationEnabled(bool enabled) {
    if (enabled != authenticationEnabled()) {
        m_authenticationEnabled = enabled;
        emit authenticationEnabledChanged();
    }
}

QString WebServer::username() const {
    return m_username;
}

void WebServer::setUsername(const QString &u) {
    if (u != username()) {
        m_username = u;
        m_auth = QByteArray(u.toUtf8() + ":" + password().toUtf8()).toBase64();
        emit usernameChanged();
    }
}

QString WebServer::password() const {
    return m_password;
}

void WebServer::setPassword(const QString &p) {
    if (p != password()) {
        m_password = p;
        m_auth = QByteArray(username().toUtf8() + ":" + p.toUtf8()).toBase64();
        emit passwordChanged();
    }
}

bool WebServer::isRunning() const {
    return m_status == Active;
}

void WebServer::setRunning(bool enabled) {
    if (enabled != isRunning()) {
        if (enabled) {
            start();
        }
        else {
            stop();
        }
    }
}

WebServer::Status WebServer::status() const {
    return m_status;
}

void WebServer::setStatus(WebServer::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool WebServer::start() {
    init();
    
    if (m_server->listen(port())) {
        setStatus(Active);
        return true;
    }
    
    setStatus(Error);
    return false;
}

void WebServer::stop() {
    if (m_server) {
        if (isRunning()) {
            m_server->close();
            setStatus(Idle);
        }
    }
}

void WebServer::init() {
    if (!m_server) {
        m_server = new QHttpServer(this);
        connect(m_server, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)),
                this, SLOT(onNewRequest(QHttpRequest*,QHttpResponse*)));
    }
    
    if (!m_articleServer) {
        m_articleServer = new ArticleServer(this);
    }

    if (!m_enclosureServer) {
        m_enclosureServer = new EnclosureServer(this);
    }
    
    if (!m_subscriptionServer) {
        m_subscriptionServer = new SubscriptionServer(this);
    }

    if (!m_fileServer) {
        m_fileServer = new FileServer(this);
    }
}

void WebServer::onNewRequest(QHttpRequest *request, QHttpResponse *response) {
    if (authenticationEnabled()) {
        const QByteArray auth = request->header("authorization").section(" ", -1).toUtf8();
        
        if (auth != m_auth) {
            response->setHeader("WWW-Authenticate", "Basic realm=\"cuteNews\"");
            response->setHeader("Content-Length", "0");
            response->writeHead(QHttpResponse::STATUS_UNAUTHORIZED);
            response->end();
            return;
        }
    }
    
    request->storeBody();
    
    if (request->successful()) {
        handleRequest(request, response);
    }
    else {
        m_requests.insert(request, response);
        connect(request, SIGNAL(end()), this, SLOT(onRequestEnd()));
    }    
}

void WebServer::onRequestEnd() {
    if (QHttpRequest *request = qobject_cast<QHttpRequest*>(sender())) {
        if (QHttpResponse *response = m_requests.take(request)) {
            handleRequest(request, response);
        }
    }
}

void WebServer::handleRequest(QHttpRequest *request, QHttpResponse *response) {
    if (request->path().startsWith("/articles", Qt::CaseInsensitive)) {
        if (m_articleServer->handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/enclosures", Qt::CaseInsensitive)) {
        if (m_enclosureServer->handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/subscriptions", Qt::CaseInsensitive)) {
        if (m_subscriptionServer->handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/plugins", Qt::CaseInsensitive)) {
        if (PluginServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/transfers", Qt::CaseInsensitive)) {
        if (TransferServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (request->path().startsWith("/settings", Qt::CaseInsensitive)) {
        if (SettingsServer::handleRequest(request, response)) {
            return;
        }
    }
    else if (m_fileServer->handleRequest(request, response)) {
        return;
    }
    
    response->setHeader("Content-Length", "0");
    response->writeHead(QHttpResponse::STATUS_NOT_FOUND);
    response->end();
}
