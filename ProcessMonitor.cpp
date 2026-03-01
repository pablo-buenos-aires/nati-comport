#include "ProcessMonitor.h"

#include <tlhelp32.h>

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#endif

ProcessMonitor::ProcessMonitor()
{
}

bool ProcessMonitor::WaitForProcessByPid(DWORD pid, ProcessExitInfo &info)
{
    HANDLE processHandle = OpenProcessForWait(pid);
    if (processHandle == NULL)
    {
        return false;
    }

    const bool result = WaitForExit(processHandle, pid, info);
    CloseHandle(processHandle);
    return result;
}

bool ProcessMonitor::WaitForNewestProcessByName(const std::string &exeName, ProcessExitInfo &info, DWORD pollIntervalMs)
{
    for (;;)
    {
        const DWORD pid = FindNewestProcessIdByName(exeName);
        if (pid != 0)
        {
            HANDLE processHandle = OpenProcessForWait(pid);
            if (processHandle != NULL)
            {
                const bool result = WaitForExit(processHandle, pid, info);
                CloseHandle(processHandle);
                return result;
            }
        }
        Sleep(pollIntervalMs);
    }
}

DWORD ProcessMonitor::FindNewestProcessIdByName(const std::string &exeName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32A pe;
    pe.dwSize = sizeof(PROCESSENTRY32A);

    DWORD newestPid = 0;
    FILETIME newestCreateTime;
    newestCreateTime.dwHighDateTime = 0;
    newestCreateTime.dwLowDateTime = 0;

    if (Process32FirstA(snapshot, &pe))
    {
        do
        {
            if (lstrcmpiA(pe.szExeFile, exeName.c_str()) == 0)
            {
                HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProc != NULL)
                {
                    FILETIME createTime;
                    FILETIME exitTime;
                    FILETIME kernelTime;
                    FILETIME userTime;

                    if (GetProcessTimes(hProc, &createTime, &exitTime, &kernelTime, &userTime))
                    {
                        if (CompareFileTime(&createTime, &newestCreateTime) > 0)
                        {
                            newestCreateTime = createTime;
                            newestPid = pe.th32ProcessID;
                        }
                    }
                    CloseHandle(hProc);
                }
                else
                {
                    newestPid = pe.th32ProcessID;
                }
            }
        } while (Process32NextA(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return newestPid;
}

HANDLE ProcessMonitor::OpenProcessForWait(DWORD pid)
{
    return OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
}

bool ProcessMonitor::WaitForExit(HANDLE processHandle, DWORD pid, ProcessExitInfo &info)
{
    const DWORD waitResult = WaitForSingleObject(processHandle, INFINITE);
    if (waitResult != WAIT_OBJECT_0)
    {
        return false;
    }

    DWORD exitCode = 0;
    const BOOL exitOk = GetExitCodeProcess(processHandle, &exitCode);

    info.pid = pid;
    info.exitCodeAvailable = (exitOk == TRUE);
    info.exitCode = exitCode;
    info.classification = ClassifyExitCode(info.exitCodeAvailable, exitCode);

    return true;
}

ExitClassification ProcessMonitor::ClassifyExitCode(bool available, DWORD exitCode)
{
    if (!available)
    {
        return ExitUnknown;
    }
    if (exitCode == 0)
    {
        return ExitNormal;
    }
    return ExitCrashOrError;
}
