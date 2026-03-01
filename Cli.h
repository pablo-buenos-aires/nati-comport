#ifndef CLI_H
#define CLI_H

#include <string>
#include <windows.h>

struct AppConfig
{
    bool hasPid;
    DWORD pid;
    std::string processName;

    std::string comPort;
    DWORD baud;
    std::string writeText;
    std::string expectText;
    DWORD timeoutMs;

    bool once;
    bool dryRun;

    AppConfig();
};

bool ParseCommandLine(int argc, char *argv[], AppConfig &config, std::string &error);
void PrintUsage();

#endif
