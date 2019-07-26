#ifndef RENDERERCOMMANDS_H
#define RENDERERCOMMANDS_H

#include <QtGlobal>

enum RendererCommands : quint32 {
    ConnectionAlive = 0x1000,
    ControlCodeUpdate,
    ControlCreation,
    ControlDeletion,
    FormCodeUpdate,
    FormCreation,
    FormDeletion,
    IdUpdate,
    IndexUpdate,
    Init,
    InitializationProgress,
    ParentUpdate,
    RenderDone,
    Preview,
    PreviewDone,
    PropertyUpdate,
    DevicePixelRatioUpdate,
    Refresh,
    Terminate,
    GeometryLock
};

#endif // RENDERERCOMMANDS_H