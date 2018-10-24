/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "helpmanager.h"
#include <helpviewer.h>
#include <bootsettings.h>

//#include <coreplugin/icore.h>
//#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/algorithm.h>
#include <utils/filesystemwatcher.h>
#include <utils/qtcassert.h>
#include <utils/runextensions.h>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QStringList>
#include <QUrl>
#include <QSettings>
#include <QApplication>
#include <QHelpEngine>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

//static const char kUserDocumentationKey[] = "Help/UserDocumentation";
//static const char kUpdateDocumentationTask[] = "UpdateDocumentationTask";

namespace {

QByteArray loadErrorMessage(const QUrl &url, const QString &errorString)
{
    const char g_htmlPage[] =
        "<html>"
        "<head>"
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">"
        "<title>%1</title>"
        "<style>"
        "body {padding: 3em 0em; background: #eeeeee;}"
        "hr {color: lightgray; width: 100%;}"
        "img {float: left; opacity: .8;}"
        "#box {background: white; border: 1px solid lightgray; width: 600px; padding: 60px; margin: auto;}"
        "h1 {font-size: 130%; font-weight: bold; border-bottom: 1px solid lightgray; margin-left: 48px;}"
        "h2 {font-size: 100%; font-weight: normal; border-bottom: 1px solid lightgray; margin-left: 48px;}"
        "p {font-size: 90%; margin-left: 48px;}"
        "</style>"
        "</head>"
        "<body>"
        "<div id=\"box\">"
        "<img "
            "src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACA"
            "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\""
            "width=\"32\" height=\"32\"/>"
        "<h1>%2</h1>"
        "<h2>%3</h2>"
        "%4"
        "</div>"
        "</body>"
        "</html>";

    // some of the values we will replace %1...6 inside the former html
    const QString g_percent1 = QCoreApplication::translate("Help", "Error loading page");
    // percent2 will be the error details
    // percent3 will be the url of the page we got the error from
    const QString g_percent4 = QCoreApplication::translate("Help", "<p>Check that you have the corresponding "
        "documentation set installed.</p>");

    return QString::fromLatin1(g_htmlPage).arg(g_percent1, errorString,
                QCoreApplication::translate("Help", "Error loading: %1").arg(url.toString()),
                g_percent4).toUtf8();
}
}

struct HelpManagerPrivate
{
    HelpManagerPrivate() :
       m_needsSetup(true), m_helpEngine(nullptr), m_collectionWatcher(nullptr)
    {}

    ~HelpManagerPrivate();

    const QStringList documentationFromInstaller();
    void readSettings();
    void writeSettings();
    void cleanUpDocumentation();

    bool m_needsSetup;
    QHelpEngine *m_helpEngine;
    Utils::FileSystemWatcher *m_collectionWatcher;

    // data for delayed initialization
    QSet<QString> m_filesToRegister;
    QSet<QString> m_nameSpacesToUnregister;
    QHash<QString, QVariant> m_customValues;

    QSet<QString> m_userRegisteredFiles;

    QMutex m_helpengineMutex;
    QFuture<bool> m_registerFuture;
};

static HelpManager *m_instance = nullptr;
static HelpManagerPrivate *d = nullptr;

static const char linksForKeyQuery[] = "SELECT d.Title, f.Name, e.Name, "
    "d.Name, a.Anchor FROM IndexTable a, FileNameTable d, FolderTable e, "
    "NamespaceTable f WHERE a.FileId=d.FileId AND d.FolderId=e.Id AND "
    "a.NamespaceId=f.Id AND a.Name='%1'";

// -- DbCleaner

struct DbCleaner
{
    DbCleaner(const QString &dbName) : name(dbName) {}
    ~DbCleaner() { QSqlDatabase::removeDatabase(name); }
    QString name;
};

// -- HelpManager

HelpManager::HelpManager(QObject *parent) : QObject(parent)
{
    QTC_CHECK(!m_instance);
    m_instance = this;
    d = new HelpManagerPrivate;
}

HelpManager::~HelpManager()
{
    delete d;
    m_instance = nullptr;
}

HelpManager *HelpManager::instance()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

QHelpEngine* HelpManager::helpEngine()
{
    return d->m_helpEngine;
}

QString HelpManager::collectionFilePath()
{
    return QApplication::applicationDirPath() + "/docs/docs.qhc";
}

void HelpManager::registerDocumentation(const QStringList &files)
{
    if (d->m_needsSetup) {
        for (const QString &filePath : files)
            d->m_filesToRegister.insert(filePath);
        return;
    }

    QFuture<bool> future = Utils::runAsync(&HelpManager::registerDocumentationNow, files);
    Utils::onResultReady(future, m_instance, [](bool docsChanged){
        if (docsChanged) {
            d->m_helpEngine->setupData();
            emit m_instance->documentationChanged();
        }
    });
    // BUG
//    ProgressManager::addTask(future, tr("Update Documentation"),
//                             kUpdateDocumentationTask);
}

void HelpManager::registerDocumentationNow(QFutureInterface<bool> &futureInterface,
                                           const QStringList &files)
{
    QMutexLocker locker(&d->m_helpengineMutex);

    futureInterface.setProgressRange(0, files.count());
    futureInterface.setProgressValue(0);

    QHelpEngineCore helpEngine(collectionFilePath());
    helpEngine.setupData();
    bool docsChanged = false;
    QStringList nameSpaces = helpEngine.registeredDocumentations();
    for (const QString &file : files) {
        if (futureInterface.isCanceled())
            break;
        futureInterface.setProgressValue(futureInterface.progressValue() + 1);
        const QString &nameSpace = helpEngine.namespaceName(file);
        if (nameSpace.isEmpty())
            continue;
        if (!nameSpaces.contains(nameSpace)) {
            if (helpEngine.registerDocumentation(file)) {
                nameSpaces.append(nameSpace);
                docsChanged = true;
            } else {
                qWarning() << "Error registering namespace '" << nameSpace
                    << "' from file '" << file << "':" << helpEngine.error();
            }
        } else {
            const QLatin1String key("CreationDate");
            const QString &newDate = helpEngine.metaData(file, key).toString();
            const QString &oldDate = helpEngine.metaData(
                helpEngine.documentationFileName(nameSpace), key).toString();
            if (QDateTime::fromString(newDate, Qt::ISODate)
                > QDateTime::fromString(oldDate, Qt::ISODate)) {
                if (helpEngine.unregisterDocumentation(nameSpace)) {
                    docsChanged = true;
                    helpEngine.registerDocumentation(file);
                }
            }
        }
    }
    futureInterface.reportResult(docsChanged);
}

void HelpManager::unregisterDocumentation(const QStringList &nameSpaces)
{
    if (d->m_needsSetup) {
        for (const QString &name : nameSpaces)
            d->m_nameSpacesToUnregister.insert(name);
        return;
    }

    QMutexLocker locker(&d->m_helpengineMutex);
    bool docsChanged = false;
    for (const QString &nameSpace : nameSpaces) {
        const QString filePath = d->m_helpEngine->documentationFileName(nameSpace);
        if (d->m_helpEngine->unregisterDocumentation(nameSpace)) {
            docsChanged = true;
            d->m_userRegisteredFiles.remove(filePath);
        } else {
            qWarning() << "Error unregistering namespace '" << nameSpace
                << "' from file '" << filePath
                << "': " << d->m_helpEngine->error();
        }
    }
    locker.unlock();
    if (docsChanged)
        emit m_instance->documentationChanged();
}

void HelpManager::registerUserDocumentation(const QStringList &filePaths)
{
    for (const QString &filePath : filePaths)
        d->m_userRegisteredFiles.insert(filePath);
    registerDocumentation(filePaths);
}

QSet<QString> HelpManager::userDocumentationPaths()
{
    return d->m_userRegisteredFiles;
}

static QUrl buildQUrl(const QString &ns, const QString &folder,
    const QString &relFileName, const QString &anchor)
{
    QUrl url;
    url.setScheme(QLatin1String("qthelp"));
    url.setAuthority(ns);
    url.setPath(QLatin1Char('/') + folder + QLatin1Char('/') + relFileName);
    url.setFragment(anchor);
    return url;
}

// This should go into Qt 4.8 once we start using it for Qt Creator
QMap<QString, QUrl> HelpManager::linksForKeyword(const QString &key)
{
    QMap<QString, QUrl> links;
    QTC_ASSERT(!d->m_needsSetup, return links);

    const QLatin1String sqlite("QSQLITE");
    const QLatin1String name("HelpManager::linksForKeyword");

    DbCleaner cleaner(name);
    QSqlDatabase db = QSqlDatabase::addDatabase(sqlite, name);
    if (db.driver() && db.driver()->lastError().type() == QSqlError::NoError) {
        const QStringList &registeredDocs = d->m_helpEngine->registeredDocumentations();
        for (const QString &nameSpace : registeredDocs) {
            db.setDatabaseName(d->m_helpEngine->documentationFileName(nameSpace));
            if (db.open()) {
                QSqlQuery query = QSqlQuery(db);
                query.setForwardOnly(true);
                query.exec(QString::fromLatin1(linksForKeyQuery).arg(key));
                while (query.next()) {
                    QString title = query.value(0).toString();
                    if (title.isEmpty()) // generate a title + corresponding path
                        title = key + QLatin1String(" : ") + query.value(3).toString();
                    links.insertMulti(title, buildQUrl(query.value(1).toString(),
                        query.value(2).toString(), query.value(3).toString(),
                        query.value(4).toString()));
                }
            }
        }
    }
    return links;
}

QMap<QString, QUrl> HelpManager::linksForIdentifier(const QString &id)
{
    QMap<QString, QUrl> empty;
    QTC_ASSERT(!d->m_needsSetup, return empty);
    return d->m_helpEngine->linksForIdentifier(id);
}

QUrl HelpManager::findFile(const QUrl &url)
{
    QTC_ASSERT(!d->m_needsSetup, return QUrl());
    return d->m_helpEngine->findFile(url);
}

QByteArray HelpManager::fileData(const QUrl &url)
{
    QTC_ASSERT(!d->m_needsSetup, return QByteArray());
    return d->m_helpEngine->fileData(url);
}

void HelpManager::handleHelpRequest(const QUrl &url, HelpManager::HelpViewerLocation location)
{
    emit m_instance->helpRequested(url, location);
}

void HelpManager::handleHelpRequest(const QString &url, HelpViewerLocation location)
{
    handleHelpRequest(QUrl(url), location);
}

QStringList HelpManager::registeredNamespaces()
{
    QTC_ASSERT(!d->m_needsSetup, return QStringList());
    return d->m_helpEngine->registeredDocumentations();
}

QString HelpManager::namespaceFromFile(const QString &file)
{
    QTC_ASSERT(!d->m_needsSetup, return QString());
    return d->m_helpEngine->namespaceName(file);
}

QString HelpManager::fileFromNamespace(const QString &nameSpace)
{
    QTC_ASSERT(!d->m_needsSetup, return QString());
    return d->m_helpEngine->documentationFileName(nameSpace);
}

HelpManager::HelpData HelpManager::helpData(const QUrl& url)
{
    HelpData data;
    const QHelpEngineCore *engine = helpEngine();

    data.resolvedUrl = engine->findFile(url);
    if (data.resolvedUrl.isValid()) {
        data.data = engine->fileData(data.resolvedUrl);
        data.mimeType = HelpViewer::mimeFromUrl(data.resolvedUrl);
        if (data.mimeType.isEmpty())
            data.mimeType = "application/octet-stream";
    } else {
        data.data = loadErrorMessage(url, QCoreApplication::translate(
                                         "Help", "The page could not be found"));
        data.mimeType = "text/html";
    }
    return data;
}

void HelpManager::setCustomValue(const QString &key, const QVariant &value)
{
    if (d->m_needsSetup) {
        d->m_customValues.insert(key, value);
        return;
    }
    if (d->m_helpEngine->setCustomValue(key, value))
        emit m_instance->collectionFileChanged();
}

QVariant HelpManager::customValue(const QString &key, const QVariant &value)
{
    QTC_ASSERT(!d->m_needsSetup, return QVariant());
    return d->m_helpEngine->customValue(key, value);
}

HelpManager::Filters HelpManager::filters()
{
    QTC_ASSERT(!d->m_needsSetup, return Filters());

    Filters filters;
    const QStringList &customFilters = d->m_helpEngine->customFilters();
    for (const QString &filter : customFilters)
        filters.insert(filter, d->m_helpEngine->filterAttributes(filter));
    return filters;
}

HelpManager::Filters HelpManager::fixedFilters()
{
    Filters fixedFilters;
    QTC_ASSERT(!d->m_needsSetup, return fixedFilters);

    const QLatin1String sqlite("QSQLITE");
    const QLatin1String name("HelpManager::fixedCustomFilters");

    DbCleaner cleaner(name);
    QSqlDatabase db = QSqlDatabase::addDatabase(sqlite, name);
    if (db.driver() && db.driver()->lastError().type() == QSqlError::NoError) {
        const QStringList &registeredDocs = d->m_helpEngine->registeredDocumentations();
        for (const QString &nameSpace : registeredDocs) {
            db.setDatabaseName(d->m_helpEngine->documentationFileName(nameSpace));
            if (db.open()) {
                QSqlQuery query = QSqlQuery(db);
                query.setForwardOnly(true);
                query.exec(QLatin1String("SELECT Name FROM FilterNameTable"));
                while (query.next()) {
                    const QString &filter = query.value(0).toString();
                    fixedFilters.insert(filter, d->m_helpEngine->filterAttributes(filter));
                }
            }
        }
    }
    return fixedFilters;
}

HelpManager::Filters HelpManager::userDefinedFilters()
{
    QTC_ASSERT(!d->m_needsSetup, return Filters());

    Filters all = filters();
    const Filters &fixed = fixedFilters();
    for (Filters::const_iterator it = fixed.constBegin(); it != fixed.constEnd(); ++it)
        all.remove(it.key());
    return all;
}

void HelpManager::removeUserDefinedFilter(const QString &filter)
{
    QTC_ASSERT(!d->m_needsSetup, return);

    if (d->m_helpEngine->removeCustomFilter(filter))
        emit m_instance->collectionFileChanged();
}

void HelpManager::addUserDefinedFilter(const QString &filter, const QStringList &attr)
{
    QTC_ASSERT(!d->m_needsSetup, return);

    if (d->m_helpEngine->addCustomFilter(filter, attr))
        emit m_instance->collectionFileChanged();
}

void HelpManager::aboutToShutdown()
{
    if (d && d->m_registerFuture.isRunning()) {
        d->m_registerFuture.cancel();
        d->m_registerFuture.waitForFinished();
    }
}

// -- private

void HelpManager::setupHelpManager()
{
    if (!d->m_needsSetup)
        return;
    d->m_needsSetup = false;

    d->readSettings();

    // create the help engine
    d->m_helpEngine = new QHelpEngine(collectionFilePath(), m_instance);
    d->m_helpEngine->setAutoSaveFilter(false);
    d->m_helpEngine->setupData();

//    qDebug() << d->m_helpEngine->registeredDocumentations();

//    for (const QString &filePath : d->documentationFromInstaller())
//        d->m_filesToRegister.insert(filePath);

//    d->cleanUpDocumentation();

//    if (!d->m_nameSpacesToUnregister.isEmpty()) {
//        unregisterDocumentation(d->m_nameSpacesToUnregister.toList());
//        d->m_nameSpacesToUnregister.clear();
//    }

//    if (!d->m_filesToRegister.isEmpty()) {
//        registerDocumentation(d->m_filesToRegister.toList());
//        d->m_filesToRegister.clear();
//    }

//    QHash<QString, QVariant>::const_iterator it;
//    for (it = d->m_customValues.constBegin(); it != d->m_customValues.constEnd(); ++it)
//        setCustomValue(it.key(), it.value());

    emit m_instance->setupFinished();
}

void HelpManagerPrivate::cleanUpDocumentation()
{
    // mark documentation for removal for which there is no documentation file anymore
    // mark documentation for removal that is neither user registered, nor marked for registration
    const QStringList &registeredDocs = m_helpEngine->registeredDocumentations();
    for (const QString &nameSpace : registeredDocs) {
        const QString filePath = m_helpEngine->documentationFileName(nameSpace);
        if (!QFileInfo::exists(filePath)
                || (!m_filesToRegister.contains(filePath)
                    && !m_userRegisteredFiles.contains(filePath))) {
            m_nameSpacesToUnregister.insert(nameSpace);
        }
    }
}

HelpManagerPrivate::~HelpManagerPrivate()
{
    writeSettings();
    delete m_helpEngine;
    m_helpEngine = nullptr;
}

const QStringList HelpManagerPrivate::documentationFromInstaller()
{
    // WARNING
//    QSettings *installSettings = BootSettings::settings();
//    const QStringList documentationPaths = installSettings->value(QLatin1String("Help/InstalledDocumentation"))
//            .toStringList();
    QStringList documentationFiles;
//    for (const QString &path : documentationPaths) {
//        QFileInfo pathInfo(path);
//        if (pathInfo.isFile() && pathInfo.isReadable()) {
//            documentationFiles << pathInfo.absoluteFilePath();
//        } else if (pathInfo.isDir()) {
//            const QFileInfoList files(QDir(path).entryInfoList(QStringList(QLatin1String("*.qch")),
//                                                               QDir::Files | QDir::Readable));
//            for (const QFileInfo &fileInfo : files)
//                documentationFiles << fileInfo.absoluteFilePath();
//        }
//    }
    return documentationFiles;
}

void HelpManagerPrivate::readSettings()
{// WARNING
//    m_userRegisteredFiles = BootSettings::settings()->value(QLatin1String(kUserDocumentationKey))
//            .toStringList().toSet();
}

void HelpManagerPrivate::writeSettings()
{// WARNING
//    const QStringList list = m_userRegisteredFiles.toList();
//    BootSettings::settings()->setValue(QLatin1String(kUserDocumentationKey), list);
}