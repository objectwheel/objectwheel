#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QString>

class CommandlineParser final
{
    Q_DISABLE_COPY(CommandlineParser)

public:
    static void init(int argc, char* argv[]);
    static bool highDpiEnabled();
    static QString projectDirectory();
    static QString serverName();

private:
    CommandlineParser() {}

private:
    static bool s_highDpiEnabled;
    static QString s_projectDirectory;
    static QString s_serverName;
};

#endif // COMMANDLINEPARSER_H