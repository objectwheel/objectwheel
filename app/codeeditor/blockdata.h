#ifndef BLOCKDATA_H
#define BLOCKDATA_H

#include <QTextBlockUserData>

struct CodeFormatterData {};

struct Parenthesis;
typedef QVector<Parenthesis> Parentheses;

struct Parenthesis
{
    enum Type : char { Opened, Closed };

    inline Parenthesis() : pos(-1), type(Opened)  {}
    inline Parenthesis(Type t, QChar c, int position)
        : pos(position), chr(c), type(t) {}
    int pos;
    QChar chr;
    Type type;
};

struct Mark
{
    using RemovalCallBack = std::function<void(Mark*)>;

    enum Type {
        Bug,
        CodeModelError,
        CodeModelWarning,
        Error,
        FixMe,
        Note,
        NoMark,
        Todo,
        Warning
    };

    ~Mark()
    {
        if (removalCallback)
            removalCallback(this);
    }

    QString message;
    Type type = NoMark;
    RemovalCallBack removalCallback;
};

struct BlockData : public QTextBlockUserData
{
    enum BlockState {
        StartOn,
        StartOff,
        Line,
        MiddleEnd,
        End,
        NoBlock
    };

    Mark mark;
    int lexerState = 0;
    Parentheses parentheses;
    BlockState state = NoBlock;
    bool hasBreakpointSet = false;
    CodeFormatterData* codeFormatterData = nullptr;
};

#endif // BLOCKDATA_H