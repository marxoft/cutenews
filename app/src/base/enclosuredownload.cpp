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

#include "enclosuredownload.h"
#include "definitions.h"
#include "enclosurerequest.h"
#include "logger.h"
#include "pluginmanager.h"
#include "pluginsettings.h"
#include "settings.h"
#include "utils.h"
#include <QBuffer>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>

EnclosureDownload::EnclosureDownload(QObject *parent) :
    Transfer(Transfer::EnclosureDownload, parent),
    m_reply(0),
    m_process(0),
    m_canceled(false),
    m_category(tr("Default")),
    m_customCommandOverrideEnabled(false),
    m_usePlugin(false),
    m_redirects(0),
    m_metadataSet(false)
{
}

QVariant EnclosureDownload::data(int role) const {
    switch (role) {
    case CategoryRole:
        return category();
    case CustomCommandRole:
        return customCommand();
    case CustomCommandOverrideEnabledRole:
        return customCommandOverrideEnabled();
    case DownloadPathRole:
        return downloadPath();
    case FileNameRole:
        return fileName();
    case PluginSettingsRole:
        return pluginSettings();
    case UsePluginRole:
        return usePlugin();
    default:
        return Transfer::data(role);
    }
}

bool EnclosureDownload::setData(int role, const QVariant &value) {
    switch (role) {
    case CategoryRole:
        setCategory(value.toString());
        return true;
    case CustomCommandRole:
        setCustomCommand(value.toString());
        return true;
    case CustomCommandOverrideEnabledRole:
        setCustomCommandOverrideEnabled(value.toBool());
        return true;
    case DownloadPathRole:
        setDownloadPath(value.toString());
        return true;
    case FileNameRole:
        setFileName(value.toString());
        return true;
    case PluginSettingsRole:
        setPluginSettings(value.toMap());
        return true;
    case UsePluginRole:
        setUsePlugin(value.toBool());
        return true;
    default:
        return Transfer::setData(role, value);
    }
}

QString EnclosureDownload::category() const {
    return m_category;
}

void EnclosureDownload::setCategory(const QString &c) {
    if (c != category()) {
        m_category = c;
        emit categoryChanged();
        emit dataChanged(this, CategoryRole);
    }
}

QString EnclosureDownload::customCommand() const {
    return m_customCommand;
}

void EnclosureDownload::setCustomCommand(const QString &c) {
    if (c != customCommand()) {
        m_customCommand = c;
        emit customCommandChanged();
        emit dataChanged(this, CustomCommandRole);
    }
}

bool EnclosureDownload::customCommandOverrideEnabled() const {
    return m_customCommandOverrideEnabled;
}

void EnclosureDownload::setCustomCommandOverrideEnabled(bool enabled) {
    if (enabled != customCommandOverrideEnabled()) {
        m_customCommandOverrideEnabled = enabled;
        emit customCommandOverrideEnabledChanged();
        emit dataChanged(this, CustomCommandOverrideEnabledRole);
    }
}

QString EnclosureDownload::downloadPath() const {
    return m_downloadPath;
}

void EnclosureDownload::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        m_downloadPath = path.endsWith("/") ? path : path + "/";
        emit downloadPathChanged();
        emit dataChanged(this, DownloadPathRole);
        
        if (!fileName().isEmpty()) {
            m_file.setFileName(downloadPath() + fileName());
            setBytesTransferred(m_file.size());
            
            if ((size() > 0) && (bytesTransferred() > 0)) {
                setProgress(bytesTransferred() * 100 / size());
            }
        }
    }
}

QString EnclosureDownload::fileName() const {
    return m_fileName;
}

void EnclosureDownload::setFileName(const QString &name) {
    if (name != fileName()) {
        m_fileName = Utils::getSanitizedFileName(name);
        emit fileNameChanged();
        emit dataChanged(this, FileNameRole);
        
        if (!downloadPath().isEmpty()) {
            m_file.setFileName(downloadPath() + fileName());
            setBytesTransferred(m_file.size());
            
            if ((size() > 0) && (bytesTransferred() > 0)) {
                setProgress(bytesTransferred() * 100 / size());
            }
        }
    }    
}

bool EnclosureDownload::usePlugin() const {
    return m_usePlugin;
}

void EnclosureDownload::setUsePlugin(bool enabled) {
    if (enabled != usePlugin()) {
        m_usePlugin = enabled;
        emit usePluginChanged();
        emit dataChanged(this, UsePluginRole);
    }
}

QVariantMap EnclosureDownload::pluginSettings() const {
    return m_pluginSettings;
}

void EnclosureDownload::setPluginSettings(const QVariantMap &settings) {
    m_pluginSettings = settings;
    emit pluginSettingsChanged();
    emit dataChanged(this, PluginSettingsRole);
}

void EnclosureDownload::queue() {
    switch (status()) {
    case Canceled:
    case Completed:
    case Queued:
    case Connecting:
    case Downloading:
    case ExecutingCustomCommand:
        return;
    default:
        break;
    }
    
    setStatus(Queued);
}

void EnclosureDownload::start() {
    switch (status()) {
    case Canceled:
    case Completed:
    case Connecting:
    case Downloading:
    case ExecutingCustomCommand:
        return;
    default:
        break;
    }
    
    if (!usePlugin()) {
        startDownload(url());
        return;
    }
    
    const FeedPluginList plugins = PluginManager::instance()->plugins();

    for (int i = 0; i < plugins.size(); i++) {
        const FeedPluginConfig *config = plugins.at(i).config;

        if ((config->supportsEnclosures()) && (config->enclosureIsSupported(url()))) {
            EnclosureRequest *request = plugins.at(i).plugin->enclosureRequest(this);

            if (request) {
                setStatus(Connecting);
                connect(request, SIGNAL(finished(EnclosureRequest*)),
                        this, SLOT(onEnclosureRequestFinished(EnclosureRequest*)));
                
                if (pluginSettings().isEmpty()) {
                    PluginSettings settings(config->id(), this);
                    setPluginSettings(settings.values());
                }
                
                request->getEnclosure(url(), pluginSettings());
                return;
            }
        }
    }
    
    Logger::log("EnclosureDownload::start(). No plugin found for URL: " + url(), Logger::LowVerbosity);
    startDownload(url());
}

void EnclosureDownload::pause() {
    switch (status()) {
    case Paused:
    case Canceled:
    case Completed:
    case Connecting:
    case ExecutingCustomCommand:
        return;
    default:
        break;
    }
    
    if ((m_reply) && (m_reply->isRunning())) {
        m_canceled = false;
        m_reply->abort();
    }
    else {
        setStatus(Paused);
    }
}

void EnclosureDownload::cancel() {
    switch (status()) {
    case Canceled:
    case Completed:
    case ExecutingCustomCommand:
        return;
    default:
        break;
    }
    
    if ((m_reply) && (m_reply->isRunning())) {
        m_canceled = true;
        m_reply->abort();
    }
    else {
        m_file.remove();
        QDir().rmdir(downloadPath());        
        setStatus(Canceled);
    }
}

QProcess* EnclosureDownload::process() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCustomCommandFinished(int)));
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onCustomCommandError()));
    }
    
    return m_process;
}

void EnclosureDownload::startDownload(QNetworkRequest &request, const QByteArray &operation, const QByteArray &data) {
    Logger::log("EnclosureDownload::startDownload(). URL: " + request.url().toString(), Logger::LowVerbosity);
    QDir().mkpath(downloadPath());
    
    if (!m_file.open(m_file.exists() ? QFile::Append : QFile::WriteOnly)) {
        setErrorString(m_file.errorString());
        setStatus(Failed);
        return;
    }
    
    request.setRawHeader("User-Agent", USER_AGENT);
    
    if (bytesTransferred() > 0) {
        request.setRawHeader("Range", "bytes=" + QByteArray::number(bytesTransferred()) + "-");
    }
    
    setSpeed(0);
    setStatus(Downloading);
    m_redirects = 0;
    m_speedTime.start();

    if (!data.isEmpty()) {
        QBuffer *buffer = new QBuffer;
        buffer->setData(data);
        buffer->open(QBuffer::ReadOnly);
        m_reply = networkAccessManager()->sendCustomRequest(request, operation, buffer);
        buffer->setParent(m_reply);
    }
    else {
        m_reply = networkAccessManager()->sendCustomRequest(request, operation);
    }
    
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void EnclosureDownload::startDownload(const QString &u) {
    QNetworkRequest request(u);
    startDownload(request);
}

void EnclosureDownload::followRedirect(const QString &u) {
    Logger::log("EnclosureDownload::followRedirect(). URL: " + u, Logger::LowVerbosity);
    QDir().mkpath(downloadPath());
    
    if (!m_file.open(m_file.exists() ? QFile::Append : QFile::WriteOnly)) {
        setErrorString(m_file.errorString());
        setStatus(Failed);
        return;
    }
    
    ++m_redirects;
    QNetworkRequest request(u);
    request.setRawHeader("User-Agent", USER_AGENT);
    
    if (bytesTransferred() > 0) {
        request.setRawHeader("Range", "bytes=" + QByteArray::number(bytesTransferred()) + "-");
    }

    m_speedTime.start();
    m_reply = networkAccessManager()->get(request);
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onReplyMetaDataChanged()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReplyReadyRead()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

bool EnclosureDownload::executeCustomCommands() {
    Logger::log("EnclosureDownload::executeCustomCommands()", Logger::LowVerbosity);
    m_commands.clear();
    QString command = customCommand();
    const QString defaultCommand = Settings::customTransferCommand();
    const bool defaultEnabled = (!defaultCommand.isEmpty()) && (Settings::customTransferCommandEnabled());

    if (!command.isEmpty()) {
        const QString workingDirectory = downloadPath();
        command.replace("%f", fileName());
        m_commands << Command(workingDirectory, command);
        Logger::log(QString("EnclosureDownload::executeCustomCommands(): Adding custom command: Working directory: %1, Command: %2")
                           .arg(workingDirectory).arg(command), Logger::LowVerbosity);
    }
    
    if ((defaultEnabled) && ((command.isEmpty()) || (!customCommandOverrideEnabled()))) {
        const QString workingDirectory = downloadPath();
        command = defaultCommand;
        command.replace("%f", fileName());
        m_commands << Command(workingDirectory, command);
        Logger::log(QString("EnclosureDownload::executeCustomCommands(): Adding custom command: Working directory: %1, Command: %2")
                           .arg(workingDirectory).arg(command), Logger::LowVerbosity);
    }
    
    if (!m_commands.isEmpty()) {
        setStatus(ExecutingCustomCommand);
        executeCustomCommand(m_commands.takeFirst());
        return true;
    }
    
    return false;
}

void EnclosureDownload::executeCustomCommand(const Command &command) {
    Logger::log(QString("EnclosureDownload::executeCustomCommand(): Working directory: %1, Command: %2")
                       .arg(command.workingDirectory).arg(command.command), Logger::LowVerbosity);
    
    if (QDir(command.workingDirectory).exists()) {
        process()->setWorkingDirectory(command.workingDirectory);
    }
    
    process()->start(command.command);
}

void EnclosureDownload::moveDownloadedFiles() {
    Logger::log("EnclosureDownload::moveDownloadedFiles()", Logger::LowVerbosity);
    QDir destDir(Settings::downloadPath(category()));
    
    if (!destDir.mkpath(destDir.path())) {
        setErrorString(tr("Cannot make download path %1").arg(destDir.path()));
        setStatus(Failed);
        return;
    }
    
    QDir downDir(downloadPath());
    
    foreach (const QString &oldFileName, downDir.entryList(QDir::Files)) {
        int i = 0;
        QString newFileName = QString("%1/%2").arg(destDir.path()).arg(oldFileName);

        while ((destDir.exists(newFileName)) && (i < 100)) {
            i++;
            newFileName = (i == 1 ? QString("%1(%2)%3").arg(newFileName.left(newFileName.lastIndexOf('.')))
                                                       .arg(i).arg(newFileName.mid(newFileName.lastIndexOf('.')))
                                  : QString("%1(%2)%3").arg(newFileName.left(newFileName.lastIndexOf('(')))
                                                       .arg(i).arg(newFileName.mid(newFileName.lastIndexOf('.'))));
        }
        
        if (!destDir.rename(downDir.absoluteFilePath(oldFileName), newFileName)) {
            setErrorString(tr("Cannot rename downloaded file to %1").arg(newFileName));
            setStatus(Failed);
            return;
        }
    }
        
    downDir.rmdir(downDir.path());
    setErrorString(QString());
    setStatus(Completed);
}

void EnclosureDownload::onEnclosureRequestFinished(EnclosureRequest *request) {
    if (request->status() == EnclosureRequest::Ready) {
        EnclosureResult enclosure = request->result();
        setFileName(enclosure.fileName);
        setName(enclosure.fileName);
        startDownload(enclosure.request, enclosure.operation, enclosure.data);
    }
    else {
        setErrorString(request->errorString());
        setStatus(Failed);
    }
    
    request->deleteLater();
}

void EnclosureDownload::onReplyMetaDataChanged() {
    if ((m_metadataSet) || (m_reply->error() != QNetworkReply::NoError) ||
        (!m_reply->rawHeader("Location").isEmpty())) {
        return;
    }

    qint64 bytes = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    
    if (bytes <= 0) {
        bytes = m_reply->rawHeader("Content-Length").toLongLong();
    }
    
    if (bytes > 0) {
        setSize(bytes + bytesTransferred());
    }
    
    m_metadataSet = true;
}

void EnclosureDownload::onReplyReadyRead() {
    if (!m_metadataSet) {
        return;
    }

    const qint64 bytes = m_reply->bytesAvailable();

    if (bytes < DOWNLOAD_BUFFER_SIZE) {
        return;
    }

    if (m_file.write(m_reply->read(bytes)) == -1) {
        m_reply->deleteLater();
	m_reply = 0;
        setErrorString(tr("Cannot write to file - %1").arg(m_file.errorString()));
        setStatus(Failed);
        return;
    }
    
    setBytesTransferred(bytesTransferred() + bytes);
    setSpeed(int(bytes) * 1000 / qMax(1, m_speedTime.restart()));
    
    if (size() > 0) {
        setProgress(int(bytesTransferred() * 100 / size()));
    }
}

void EnclosureDownload::onReplyFinished() {
    setSpeed(0);
    const QString redirect = QString::fromUtf8(m_reply->rawHeader("Location"));

    if (!redirect.isEmpty()) {
	m_file.close();
        m_reply->deleteLater();
        m_reply = 0;
        
        if (m_redirects < MAX_REDIRECTS) {
            followRedirect(redirect);
        }
        else {
            setErrorString(tr("Maximum redirects reached"));
            setStatus(Failed);
        }
        
        return;
    }

    const QNetworkReply::NetworkError error = m_reply->error();
    const QString errorString = m_reply->errorString();

    if ((m_reply->isOpen()) && (error == QNetworkReply::NoError) && (m_file.isOpen())) {
        const qint64 bytes = m_reply->bytesAvailable();
        
        if ((bytes > 0) && (m_metadataSet)) {
            m_file.write(m_reply->read(bytes));
            setBytesTransferred(bytesTransferred() + bytes);
            
            if (size() > 0) {
                setProgress(bytesTransferred() * 100 / size());
            }
        }
    }

    m_file.close();
    m_reply->deleteLater();
    m_reply = 0;
        
    switch (error) {
    case QNetworkReply::NoError:
        break;
    case QNetworkReply::OperationCanceledError:
        setErrorString(QString());
        
        if (m_canceled) {
            m_file.remove();
            QDir().rmdir(downloadPath());
            setStatus(Canceled);
        }
        else {
            setStatus(Paused);
        }
        
        return;
    default:
        setErrorString(errorString);
        setStatus(Failed);
        return;
    }
    
    if (!executeCustomCommands()) {
        moveDownloadedFiles();
    }
}

void EnclosureDownload::onCustomCommandFinished(int exitCode) {
    if (exitCode != 0) {
        Logger::log("EnclosureDownload::onCustomCommandFinished(): Error: " + m_process->readAllStandardError());
    }
    
    if (!m_commands.isEmpty()) {
        executeCustomCommand(m_commands.takeFirst());
    }
    else {
        moveDownloadedFiles();
    }
}

void EnclosureDownload::onCustomCommandError() {
    Logger::log("EnclosureDownload::onCustomCommandError(): " + m_process->errorString());

    if (!m_commands.isEmpty()) {
        executeCustomCommand(m_commands.takeFirst());
    }
    else {
        moveDownloadedFiles();
    }
}
