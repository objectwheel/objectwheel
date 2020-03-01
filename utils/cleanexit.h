#ifndef CLEANEXIT_H
#define CLEANEXIT_H

#include <csetjmp>

class QCoreApplication;

using SignalHandler = bool (*) (int);

class CleanExit
{
public:
    static int exec(QCoreApplication& app, SignalHandler signalHandler = defaultSignalHandler);

private:
    static void signalHandler(int signal);
    static bool defaultSignalHandler(int signal);

private:
    CleanExit();
    CleanExit(const CleanExit&) = delete;
    CleanExit &operator=(const CleanExit&) = delete;

private:
    static std::jmp_buf s_jumpBuffer;
    static SignalHandler s_signalHandler;
};

#endif // CLEANEXIT_H
