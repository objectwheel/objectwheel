#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QJsonValue>

#define SIGN_OWDB        "T3dkYl92Mi4w"    // FIXME: There is no function of this
#define SIGN_OWCTRL      "T3djdHJsX3YyLjA"
#define SIGN_OWPRJ       "T3dwcmpfdjIuMA"

#define DIR_THIS         "t"
#define DIR_CHILDREN     "c"
#define DIR_MAINFORM     "1"
#define DIR_OWDB         "owdb"
#define DIR_IMPORTS      "imports"
#define DIR_OW           "Objectwheel"
#define DIR_GLOBAL       "GlobalResources"

#define FILE_PROJECT     "project.json"
#define FILE_OWDB        "owdb.json"       // FIXME: There is no function of this
#define FILE_CONTROL     "control.json"    // TODO: Embed icon.png data into control.json
#define FILE_ICON        "icon.png"        // TODO: Apply everywhere
#define FILE_MAIN        "main.qml"        // TODO: Apply everywhere

#define TAG_ID           "id"
#define TAG_UID          "uid"
#define TAG_NAME         "name"
#define TAG_CATEGORY     "category"
#define TAG_OWDB_SIGN    "owdbsign"        // FIXME: There is no function of this
#define TAG_OWCTRL_SIGN  "owctrlsign"

#define PTAG_NAME        "name"
#define PTAG_DESCRIPTION "description"
#define PTAG_OWNER       "owner"
#define PTAG_CRDATE      "crdate"
#define PTAG_MFDATE      "mfdate"
#define PTAG_SIZE        "size"
#define PTAG_HASH        "hash"
#define PTAG_THEME       "theme"
#define PTAG_SCALING     "scaling"
#define PTAG_OWPRJ_SIGN  "owprj"

namespace SaveUtils
{
bool isForm(const QString& rootPath);
bool isMain(const QString& rootPath);
bool isOwctrl(const QString& rootPath);
bool isOwprj(const QString& projectDir);

int biggestDir(const QString& basePath);
int childrenCount(const QString& rootPath);

QString toUrl(const QString& rootPath);
QString toIcon(const QString& rootPath);
QString toThisDir(const QString& rootPath);
QString toParentDir(const QString& topPath);
QString toChildrenDir(const QString& rootPath);
QString toOwdbDir(const QString& projectDir);
QString toProjectFile(const QString& projectDir);
QString toImportsDir(const QString& projectDir);
QString toOwDir(const QString& projectDir);
QString toGlobalDir(const QString& projectDir);
QString toControlFile(const QString& rootPath);

QStringList formPaths(const QString& projectDir);
QStringList controlPaths(const QString& topPath);
QStringList childrenPaths(const QString& rootPath);

QString id(const QString& rootPath);
QString uid(const QString& rootPath);
QString name(const QString& rootPath);
QString category(const QString& rootPath);

QString projectHash(const QString& projectDir);
QString projectName(const QString& projectDir);
QString projectSize(const QString& projectDir);
QString projectOwner(const QString& projectDir);
QString projectCrDate(const QString& projectDir);
QString projectMfDate(const QString& projectDir);
QString projectScaling(const QString& projectDir);
QString projectDescription(const QString& projectDir);
QJsonValue projectTheme(const QString& projectDir);

QJsonValue property(const QString& rootPath, const QString& property);
QJsonValue projectProperty(const QString& projectDir, const QString& property);

void setProperty(const QString& rootPath,  const QString& property, const QJsonValue& value);
void setProjectProperty(const QString& projectDir, const QString& property, const QJsonValue& value);
void regenerateUids(const QString& topPath);
}

#endif // SAVEUTILS_H
