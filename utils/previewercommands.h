#ifndef PREVIEWERCOMMANDS_H
#define PREVIEWERCOMMANDS_H

#include <QDataStream>

enum PreviewerCommands {
    ConnectionAlive,
    Init,
    Preview,
    Done,
    Repreview,
    Terminate,
    Restart,
    Remove,
    Update,
    Reparent,
    InitializationProgress
};

inline QDataStream& operator>>(QDataStream& in, PreviewerCommands& e)
{ return in >> (int&) e; }

inline QDataStream& operator<<(QDataStream& out, const PreviewerCommands& e)
{ return out << int(e); }

#endif // PREVIEWERCOMMANDS_H