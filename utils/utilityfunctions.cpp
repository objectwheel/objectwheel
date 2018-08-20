#include <utilityfunctions.h>
#include <saveutils.h>

#include <QQmlEngine>

namespace {

QString projectDirectory;
}

void UtilityFunctions::registerGlobalPath(const QString& projectDir)
{
    projectDirectory = projectDir;
    qmlRegisterSingletonType("Global", 1, 0, "Global",
                             [] (QQmlEngine* engine, QJSEngine* scriptEngine) -> QJSValue {
        Q_UNUSED(engine)
        QJSValue globalPath = scriptEngine->newObject();
        globalPath.setProperty("path", SaveUtils::toGlobalDir(projectDirectory));
        globalPath.setProperty("url", scriptEngine->toScriptValue(
                                   QUrl::fromLocalFile(SaveUtils::toGlobalDir(projectDirectory))));
        return globalPath;
    });
}
