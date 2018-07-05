#ifndef PREVIEWERCOMMANDS_H
#define PREVIEWERCOMMANDS_H

#include <QDataStream>

enum PreviewerCommands {
    ConnectionAlive,
    Init,
    InitializationProgress,
    PreviewDone,
    Remove,
    Reparent,
    Terminate,
    PropertyUpdate,
    ControlCreation
};
Q_DECLARE_METATYPE(PreviewerCommands)

inline QDataStream& operator>>(QDataStream& in, PreviewerCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const PreviewerCommands& e)
{ return out << int(e); }

#endif // PREVIEWERCOMMANDS_H