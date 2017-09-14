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

#include "nytimesenclosurerequest.h"
#include "json.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#include <QStringList>
#ifdef NYTIMES_DEBUG
#include <QDebug>
#endif

const int NytimesEnclosureRequest::MAX_REDIRECTS = 8;

const QByteArray NytimesEnclosureRequest::USER_AGENT("Wget/1.13.4 (linux-gnu)");

const QString NytimesEnclosureRequest::VIDEO_JSON_URL("http://www.nytimes.com/svc/video/api/v2/video/");

const QStringList NytimesEnclosureRequest::VIDEO_FORMATS = QStringList() << "video_1080p_mp4" << "video_720p_mp4"
    << "video_480p_mp4" << "video_360p_mp4" << "video_240p_mp4";

NytimesEnclosureRequest::NytimesEnclosureRequest(QObject *parent) :
    EnclosureRequest(parent),
    m_nam(0),
    m_status(Idle),
    m_redirects(0)
{
}

QString NytimesEnclosureRequest::errorString() const {
    return m_errorString;
}

void NytimesEnclosureRequest::setErrorString(const QString &e) {
    m_errorString = e;
}

EnclosureResult NytimesEnclosureRequest::result() const {
    return m_result;
}

void NytimesEnclosureRequest::setResult(const EnclosureResult &r) {
    m_result = r;
}

EnclosureRequest::Status NytimesEnclosureRequest::status() const {
    return m_status;
}

void NytimesEnclosureRequest::setStatus(EnclosureRequest::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

bool NytimesEnclosureRequest::cancel() {
    if (status() == Active) {
        setStatus(Canceled);
        emit finished(this);
    }

    return true;
}

bool NytimesEnclosureRequest::getEnclosure(const QString &url, const QVariantMap &settings) {
    if (status() == Active) {
        return false;
    }

    setStatus(Active);
    setErrorString(QString());
    setResult(EnclosureResult());
    m_settings = settings;
    m_redirects = 0;
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesEnclosureRequest::getEnclosure(). URL:" << url;
#endif
    QString videoId;
    const QRegExp re("(/(\\d+)/|videoId=(\\d+))");

    if (re.indexIn(url) != -1) {
        videoId = re.cap(2);
    }

    if (videoId.isEmpty()) {
        setErrorString(tr("No video id found"));
        setStatus(Error);
        emit finished(this);
        return false;
    }
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesEnclosureRequest::getEnclosure(). Fetching video JSON for id:" << videoId;
#endif
    QNetworkRequest request(VIDEO_JSON_URL + videoId);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(checkEnclosure()));
    connect(this, SIGNAL(finished(EnclosureRequest*)), reply, SLOT(deleteLater()));
    return true;
}

void NytimesEnclosureRequest::checkEnclosure() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        setErrorString(tr("Network error"));
        setStatus(Error);
        emit finished(this);
        return;
    }

    const QString redirect = getRedirect(reply);

    if (!redirect.isEmpty()) {
        if (m_redirects < MAX_REDIRECTS) {
            reply->deleteLater();
            followRedirect(redirect, SLOT(checkEnclosure()));
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Error);
            emit finished(this);
        }
        
        return;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        setStatus(Canceled);
        emit finished(this);
        return;
    default:
        setErrorString(reply->errorString());
        setStatus(Error);
        emit finished(this);
        return;
    }

    const QVariantMap json = QtJson::Json::parse(QString::fromUtf8(reply->readAll())).toMap();
    const QVariantList formats = json.value("renditions").toList();

    if (formats.isEmpty()) {
        setErrorString(tr("No video formats found"));
        setStatus(Error);
        emit finished(this);
        return;
    }

    const int index = qMax(0, VIDEO_FORMATS.indexOf(m_settings.value("videoFormat", "video_1080p_mp4").toString()));
    const int count = VIDEO_FORMATS.size();

    for (int i = index; i < count; i++) {
        foreach (const QVariant &f, formats) {
            const QVariantMap format = f.toMap();

            if (format.value("type") == VIDEO_FORMATS.at(i)) {
                const QString url = format.value("url").toString();

                if (!url.isEmpty()) {
                    m_result.fileName = url.mid(url.lastIndexOf("/") + 1);
                    m_result.request.setUrl(url);
                    setStatus(Ready);
                    emit finished(this);
                    return;
                }
            }
        }
    }

    setErrorString(tr("No video formats found"));
    setStatus(Error);
    emit finished(this);
}

void NytimesEnclosureRequest::followRedirect(const QString &url, const char *slot) {
#ifdef NYTIMES_DEBUG
    qDebug() << "NytimesEnclosureRequest::followRedirect(). URL:" << url;
#endif
    ++m_redirects;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, slot);
    connect(this, SIGNAL(finished(EnclosureRequest*)), reply, SLOT(deleteLater()));
}

QString NytimesEnclosureRequest::getRedirect(const QNetworkReply *reply) {
    QString redirect = QString::fromUtf8(reply->rawHeader("Location"));

    if ((!redirect.isEmpty()) && (!redirect.startsWith("http"))) {
        const QUrl url = reply->url();
        
        if (redirect.startsWith("/")) {
            redirect.prepend(url.scheme() + "://" + url.authority());
        }
        else {
            redirect.prepend(url.scheme() + "://" + url.authority() + "/");
        }
    }
    
    return redirect;
}

QNetworkAccessManager* NytimesEnclosureRequest::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}
