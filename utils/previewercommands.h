#ifndef PREVIEWERCOMMANDS_H
#define PREVIEWERCOMMANDS_H

#include <QtGlobal>

enum PreviewerCommands : quint32 {
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
    PreviewDone,
    IndividualPreview,
    IndividualPreviewDone,
    PropertyUpdate,
    DevicePixelRatioUpdate,
    Refresh,
    Terminate
};

#endif // PREVIEWERCOMMANDS_H