#include <commandlineparser.h>

QString CommandlineParser::s_projectDirectory;
QString CommandlineParser::s_serverName;

void CommandlineParser::init(int argc, char* argv[])
{
#if defined(RENDERER_DEBUG)
    s_serverName = "serverName";
#else
    Q_ASSERT_X(argc == 3, "init", "Number of total commandline arguments must be equal to 3");
    s_serverName = argv[2];
#endif

    s_projectDirectory = argv[1];
}

QString CommandlineParser::projectDirectory()
{
    return s_projectDirectory;
}

QString CommandlineParser::serverName()
{
    return s_serverName;
}
