#include "Cli.h"
#include "Logger.h"
#include "ProcessMonitor.h"
#include "SerialPort.h"

#include <vector>

namespace
{
    std::string ExitClassToString(ExitClassification cls)
    {
        switch (cls)
        {
        case ExitNormal:
            return "NormalExit";
        case ExitCrashOrError:
            return "CrashOrError";
        case ExitUnknown:
        default:
            return "UnknownExit";
        }
    }

    bool PerformSerialAction(const AppConfig &cfg)
    {
        if (cfg.dryRun)
        {
            Logger::Info("Dry-run: would open " + cfg.comPort + " at " + Logger::ToStringUnsignedLong(cfg.baud));
            Logger::Info("Dry-run: would write '" + Logger::MakePrintable(cfg.writeText) + "' and expect '" + Logger::MakePrintable(cfg.expectText) + "'");
            return true;
        }

        static const DWORD retries[] = {200, 400, 800, 1600, 2000};
        SerialPort port;
        for (size_t i = 0; i < sizeof(retries) / sizeof(retries[0]); ++i)
        {
            if (port.Open(cfg.comPort, cfg.baud, cfg.timeoutMs))
            {
                Logger::Info("COM port opened: " + cfg.comPort);

                std::string received;
                std::string error;
                Logger::Info("Writing to COM: '" + Logger::MakePrintable(cfg.writeText) + "'");

                const bool ok = port.Handshake(cfg.writeText, cfg.expectText, cfg.timeoutMs, received, error);
                Logger::Info("Received from COM: '" + Logger::MakePrintable(received) + "'");
                port.Close();
                Logger::Info("COM port closed: " + cfg.comPort);

                if (!ok)
                {
                    Logger::Error("Handshake failed: " + error);
                }
                return ok;
            }

            const DWORD err = GetLastError();
            Logger::Warn("Failed to open COM port, attempt " + Logger::ToStringUnsignedLong(static_cast<unsigned long>(i + 1)) +
                         ", error=" + Logger::ToStringUnsignedLong(err));
            Sleep(retries[i]);
        }

        Logger::Error("Failed to open COM port after retries");
        return false;
    }
}

int RunApp(int argc, char *argv[])
{
    AppConfig config;
    std::string parseError;

    if (!ParseCommandLine(argc, argv, config, parseError))
    {
        if (parseError != "help")
        {
            Logger::Error(parseError);
        }
        PrintUsage();
        return 1;
    }

    Logger::Info("Watcher started");

    ProcessMonitor monitor;

    for (;;)
    {
        ProcessExitInfo info;
        bool waitOk = false;

        if (config.hasPid)
        {
            Logger::Info("Waiting for PID=" + Logger::ToStringUnsignedLong(config.pid));
            waitOk = monitor.WaitForProcessByPid(config.pid, info);
            if (!waitOk)
            {
                Logger::Error("Cannot monitor PID=" + Logger::ToStringUnsignedLong(config.pid));
                return 2;
            }
        }
        else
        {
            Logger::Info("Searching and waiting for newest process: " + config.processName);
            waitOk = monitor.WaitForNewestProcessByName(config.processName, info, 500);
            if (!waitOk)
            {
                Logger::Warn("Failed waiting for process by name, retrying");
                Sleep(500);
                continue;
            }
        }

        std::string line = "TargetApp exited. PID=" + Logger::ToStringUnsignedLong(info.pid) +
                           ", class=" + ExitClassToString(info.classification);
        if (info.exitCodeAvailable)
        {
            line += ", exitCode=" + Logger::ToStringUnsignedLong(info.exitCode);
        }
        else
        {
            line += ", exitCode=unknown";
        }
        Logger::Info(line);

        PerformSerialAction(config);

        if (config.once)
        {
            Logger::Info("--once mode: exiting");
            break;
        }

        if (config.hasPid)
        {
            Logger::Info("PID mode without --once handled one lifecycle; exiting.");
            break;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    return RunApp(argc, argv);
}

int wmain(int argc, wchar_t *argv[])
{
    std::vector<std::string> utf8Args;
    std::vector<char *> narrowArgv;
    utf8Args.reserve(static_cast<size_t>(argc));
    narrowArgv.reserve(static_cast<size_t>(argc));

    for (int i = 0; i < argc; ++i)
    {
        const wchar_t *w = argv[i];

        int bytes = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
        UINT cp = CP_UTF8;
        if (bytes <= 0)
        {
            bytes = WideCharToMultiByte(CP_ACP, 0, w, -1, NULL, 0, NULL, NULL);
            cp = CP_ACP;
        }

        if (bytes <= 0)
        {
            utf8Args.push_back("");
            continue;
        }

        std::string converted(static_cast<size_t>(bytes - 1), '\0');
        WideCharToMultiByte(cp, 0, w, -1, &converted[0], bytes, NULL, NULL);
        utf8Args.push_back(converted);
    }

    for (size_t i = 0; i < utf8Args.size(); ++i)
    {
        narrowArgv.push_back(const_cast<char *>(utf8Args[i].c_str()));
    }

    return RunApp(argc, narrowArgv.empty() ? NULL : &narrowArgv[0]);
}
