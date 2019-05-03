#ifndef PREVIEWERCOMMANDS_H
#define PREVIEWERCOMMANDS_H

enum PreviewerCommands {
    ConnectionAlive = 0x1000,
    ControlCodeUpdate,
    ControlCreation,
    ControlDeletion,
    FormCodeUpdate,
    FormCreation,
    FormDeletion,
    IdUpdate,
    Init,
    InitializationProgress,
    ParentUpdate,
    PreviewDone,
    PropertyUpdate,
    Refresh,
    Terminate
};

#endif // PREVIEWERCOMMANDS_H