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
    BindingUpdate,
    PropertyUpdate,
    DevicePixelRatioUpdate,
    Refresh
};

#endif // RENDERERCOMMANDS_H