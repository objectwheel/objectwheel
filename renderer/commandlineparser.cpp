#include <commandlineparser.h>

bool CommandlineParser::s_highDpiEnabled;
QString CommandlineParser::s_projectDirectory;
QString CommandlineParser::s_serverName;

void CommandlineParser::init(int argc, char* argv[])
{
#if defined(RENDERER_DEBUG)
    s_serverName = "serverName";
#else
    Q_ASSERT_X(argc == 4, "init", "Number of total commandline arguments must be equal to 4");
    s_serverName = argv[3];
#endif
    s_projectDirectory = argv[2];
    s_highDpiEnabled = QString::fromUtf8(argv[1]).toInt();
}

bool CommandlineParser::highDpiEnabled()
{
    return s_highDpiEnabled;
}

QString CommandlineParser::projectDirectory()
{
    return s_projectDirectory;
}

QString CommandlineParser::serverName()
{
    return s_serverName;
}
