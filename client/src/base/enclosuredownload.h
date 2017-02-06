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

#ifndef ENCLOSUREDOWNLOAD_H
#define ENCLOSUREDOWNLOAD_H

#include "transfer.h"

class EnclosureDownload : public Transfer
{
    Q_OBJECT
    
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString customCommand READ customCommand WRITE setCustomCommand NOTIFY customCommandChanged)
    Q_PROPERTY(bool customCommandOverrideEnabled READ customCommandOverrideEnabled WRITE setCustomCommandOverrideEnabled
               NOTIFY customCommandOverrideEnabledChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool usePlugin READ usePlugin WRITE setUsePlugin NOTIFY usePluginChanged)
    Q_PROPERTY(QVariantMap pluginSettings READ pluginSettings WRITE setPluginSettings NOTIFY pluginSettingsChanged)

public:
    explicit EnclosureDownload(const QVariantMap &properties, QObject *parent = 0);

    Q_INVOKABLE virtual QVariant data(int role) const;
    Q_INVOKABLE virtual bool setData(int role, const QVariant &value);
            
    QString category() const;
    void setCategory(const QString &c);
    
    QString customCommand() const;
    void setCustomCommand(const QString &c);
    bool customCommandOverrideEnabled() const;
    void setCustomCommandOverrideEnabled(bool enabled);
    
    QString downloadPath() const;
    
    QString fileName() const;
    
    bool usePlugin() const;
    void setUsePlugin(bool enabled);
    QVariantMap pluginSettings() const;
    void setPluginSettings(const QVariantMap &settings);
    
protected:
    virtual void load(const QVariantMap &result);
    
Q_SIGNALS:
    void categoryChanged();
    void customCommandChanged();
    void customCommandOverrideEnabledChanged();
    void downloadPathChanged();
    void fileNameChanged();
    void usePluginChanged();
    void pluginSettingsChanged();

private:
    void setDownloadPath(const QString &path);
    
    void setFileName(const QString &fn);
        
    void updateCategory(const QString &c);
    
    void updateCustomCommand(const QString &c);
    void updateCustomCommandOverrideEnabled(bool enabled);
    
    void updateUsePlugin(bool enabled);
    void updatePluginSettings(const QVariantMap &settings);
    
    QString m_category;
    
    QString m_customCommand;
    bool m_customCommandOverrideEnabled;
    
    QString m_downloadPath;
        
    QString m_fileName;
    
    bool m_usePlugin;
    QVariantMap m_pluginSettings;
};

#endif // ENCLOSUREDOWNLOAD_H
