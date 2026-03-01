#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <string>
#include <windows.h>

enum ExitClassification
{
    ExitNormal,
    ExitCrashOrError,
    ExitUnknown
};

struct ProcessExitInfo
{
    DWORD pid;
    DWORD exitCode;
    bool exitCodeAvailable;
    ExitClassification classification;
};

class ProcessMonitor
{
public:
    ProcessMonitor();

    bool WaitForProcessByPid(DWORD pid, ProcessExitInfo &info);
    bool WaitForNewestProcessByName(const std::string &exeName, ProcessExitInfo &info, DWORD pollIntervalMs);

private:
    DWORD FindNewestProcessIdByName(const std::string &exeName);
    HANDLE OpenProcessForWait(DWORD pid);
    bool WaitForExit(HANDLE processHandle, DWORD pid, ProcessExitInfo &info);
    ExitClassification ClassifyExitCode(bool available, DWORD exitCode);
};

#endif
