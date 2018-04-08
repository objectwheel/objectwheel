#include <fm.h>
#include <filemanager.h>
#include <QQmlEngine>

#define COMPONENT_URI "com.objectwheel.components"
#define COMPONENT_NAME "FileManager"
#define COMPONENT_VERSION_MAJOR 1
#define COMPONENT_VERSION_MINOR 0

void FM::registerQmlType()
{
    qmlRegisterType<FileManager>(
        COMPONENT_URI,
        COMPONENT_VERSION_MAJOR,
        COMPONENT_VERSION_MINOR,
        COMPONENT_NAME
    );
}
