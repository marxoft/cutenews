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

#ifndef NYTIMESENCLOSUREREQUEST_H
#define NYTIMESENCLOSUREREQUEST_H

#include "enclosurerequest.h"
#include <QVariantMap>

class QNetworkAccessManager;
class QNetworkReply;

class NytimesEnclosureRequest : public EnclosureRequest
{
    Q_OBJECT

public:
    explicit NytimesEnclosureRequest(QObject *parent = 0);

    virtual QString errorString() const;

    virtual EnclosureResult result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void checkEnclosure();

private:
    void setErrorString(const QString &e);

    void setResult(const EnclosureResult &r);
    
    void setStatus(Status s);

    void followRedirect(const QString &url, const char *slot);

    static QString getRedirect(const QNetworkReply *reply);
    
    QNetworkAccessManager* networkAccessManager();

    static const int MAX_REDIRECTS;

    static const QByteArray USER_AGENT;

    static const QString VIDEO_JSON_URL;

    static const QStringList VIDEO_FORMATS;

    QNetworkAccessManager *m_nam;

    QString m_errorString;

    EnclosureResult m_result;
    
    Status m_status;

    int m_redirects;
    
    QVariantMap m_settings;
};

#endif // NYTIMESENCLOSUREREQUEST_H
