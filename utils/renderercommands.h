#ifndef RENDERERCOMMANDS_H
#define RENDERERCOMMANDS_H

#include <QMetaType>

enum RendererCommands {
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
Q_DECLARE_METATYPE(RendererCommands)

#endif // RENDERERCOMMANDS_H