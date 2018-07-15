#ifndef PREVIEWERCOMMANDS_H
#define PREVIEWERCOMMANDS_H

#include <QDataStream>

enum PreviewerCommands {
    ConnectionAlive,
    IdUpdate,
    Init,
    InitializationProgress,
    PreviewDone,
    Refresh,
    Remove,
    ParentUpdate,
    Terminate,
    PropertyUpdate,
    ControlCreation,
    ControlDeletion,
    FormDeletion
};
Q_DECLARE_METATYPE(PreviewerCommands)

inline QDataStream& operator>>(QDataStream& in, PreviewerCommands& e)
{ return in >> (quint32&) e; }

inline QDataStream& operator<<(QDataStream& out, const PreviewerCommands& e)
{ return out << quint32(e); }

#endif // PREVIEWERCOMMANDS_H