#ifndef PREVIEWERCOMMANDS_H
#define PREVIEWERCOMMANDS_H

#include <QDataStream>

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

inline QDataStream& operator>> (QDataStream& in, PreviewerCommands& e)
{
    return in >> (int&) e;
}

inline QDataStream& operator<< (QDataStream& out, PreviewerCommands e)
{
    return out << (int) e;
}

#endif // PREVIEWERCOMMANDS_H