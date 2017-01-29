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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QStringList>
#include <QVariant>

struct Category {
    QString name;
    QString path;
};

class Settings : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString loggerFileName READ loggerFileName WRITE setLoggerFileName NOTIFY loggerFileNameChanged)
    Q_PROPERTY(int loggerVerbosity READ loggerVerbosity WRITE setLoggerVerbosity NOTIFY loggerVerbosityChanged)
    Q_PROPERTY(int readArticleExpiry READ readArticleExpiry WRITE setReadArticleExpiry NOTIFY readArticleExpiryChanged)
    Q_PROPERTY(int screenOrientation READ screenOrientation WRITE setScreenOrientation NOTIFY screenOrientationChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(bool serverAuthenticationEnabled READ serverAuthenticationEnabled WRITE setServerAuthenticationEnabled
               NOTIFY serverAuthenticationEnabledChanged)
    Q_PROPERTY(QString serverPassword READ serverPassword WRITE setServerPassword NOTIFY serverPasswordChanged)
    Q_PROPERTY(QString serverUsername READ serverUsername WRITE setServerUsername NOTIFY serverUsernameChanged)
    
public:
    ~Settings();
    
    static Settings* instance();
    
    static QString loggerFileName();
    static int loggerVerbosity();
    
    static int readArticleExpiry();
    
    static int screenOrientation();
    
    static QString serverAddress();
    static bool serverAuthenticationEnabled();
    static QString serverPassword();
    static QString serverUsername();
            
    Q_INVOKABLE static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

public Q_SLOTS:
    static void setLoggerFileName(const QString &fileName);
    static void setLoggerVerbosity(int verbosity);
        
    static void setReadArticleExpiry(int expiry);
    
    static void setScreenOrientation(int orientation);
    
    static void setServerAddress(const QString &address);
    static void setServerAuthenticationEnabled(bool enabled);
    static void setServerPassword(const QString &password);
    static void setServerUsername(const QString &username);
        
    static void setValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void loggerFileNameChanged(const QString &fileName);
    void loggerVerbosityChanged(int verbosity);
    void readArticleExpiryChanged(int expiry);
    void screenOrientationChanged(int orientation);
    void serverAddressChanged(const QString &address);
    void serverAuthenticationEnabledChanged(bool enabled);
    void serverPasswordChanged(const QString &password);
    void serverUsernameChanged(const QString &username);

private:
    Settings();
    
    static Settings *self;
};
    
#endif // SETTINGS_H
