#include "Cli.h"

#include <cstdlib>
#include <iostream>

AppConfig::AppConfig()
    : hasPid(false),
      pid(0),
      processName(""),
      comPort(""),
      baud(9600),
      writeText("PING\r\n"),
      expectText("PONG"),
      timeoutMs(2000),
      once(false),
      dryRun(false)
{
}

namespace
{
    bool ParseUnsigned(const std::string &value, DWORD &result)
    {
        if (value.empty())
        {
            return false;
        }

        char *endPtr = NULL;
        unsigned long parsed = std::strtoul(value.c_str(), &endPtr, 10);
        if (endPtr == NULL || *endPtr != '\0')
        {
            return false;
        }

        result = static_cast<DWORD>(parsed);
        return true;
    }
}

bool ParseCommandLine(int argc, char *argv[], AppConfig &config, std::string &error)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);

        if (arg == "--pid")
        {
            if (i + 1 >= argc)
            {
                error = "--pid requires a value";
                return false;
            }
            DWORD pidValue = 0;
            if (!ParseUnsigned(argv[++i], pidValue))
            {
                error = "Invalid value for --pid";
                return false;
            }
            config.hasPid = true;
            config.pid = pidValue;
        }
        else if (arg == "--process")
        {
            if (i + 1 >= argc)
            {
                error = "--process requires a value";
                return false;
            }
            config.processName = argv[++i];
        }
        else if (arg == "--com")
        {
            if (i + 1 >= argc)
            {
                error = "--com requires a value";
                return false;
            }
            config.comPort = argv[++i];
        }
        else if (arg == "--baud")
        {
            if (i + 1 >= argc)
            {
                error = "--baud requires a value";
                return false;
            }
            DWORD baudValue = 0;
            if (!ParseUnsigned(argv[++i], baudValue))
            {
                error = "Invalid value for --baud";
                return false;
            }
            config.baud = baudValue;
        }
        else if (arg == "--write")
        {
            if (i + 1 >= argc)
            {
                error = "--write requires a value";
                return false;
            }
            config.writeText = argv[++i];
        }
        else if (arg == "--expect")
        {
            if (i + 1 >= argc)
            {
                error = "--expect requires a value";
                return false;
            }
            config.expectText = argv[++i];
        }
        else if (arg == "--timeout")
        {
            if (i + 1 >= argc)
            {
                error = "--timeout requires a value";
                return false;
            }
            DWORD timeoutValue = 0;
            if (!ParseUnsigned(argv[++i], timeoutValue))
            {
                error = "Invalid value for --timeout";
                return false;
            }
            config.timeoutMs = timeoutValue;
        }
        else if (arg == "--once")
        {
            config.once = true;
        }
        else if (arg == "--dry-run")
        {
            config.dryRun = true;
        }
        else if (arg == "--help" || arg == "-h")
        {
            error = "help";
            return false;
        }
        else
        {
            error = "Unknown argument: " + arg;
            return false;
        }
    }

    if (!config.hasPid && config.processName.empty())
    {
        error = "Specify one of --pid or --process";
        return false;
    }

    if (config.hasPid && !config.processName.empty())
    {
        error = "Use either --pid or --process, not both";
        return false;
    }

    if (config.comPort.empty())
    {
        error = "--com is required";
        return false;
    }

    return true;
}

void PrintUsage()
{
    std::cout << "Watcher for TargetApp exit and COM handshake\n"
              << "Usage:\n"
              << "  watcher.exe --pid <num> --com COM3 [options]\n"
              << "  watcher.exe --process <name.exe> --com COM3 [options]\n\n"
              << "Options:\n"
              << "  --baud <num>      Baud rate (default 9600)\n"
              << "  --write <text>    Text to send (default PING\\r\\n)\n"
              << "  --expect <text>   Expected response fragment (default PONG)\n"
              << "  --timeout <ms>    Timeout for COM I/O (default 2000)\n"
              << "  --once            Handle one exit event and quit\n"
              << "  --dry-run         Do not touch COM; only log actions\n"
              << "  --help            Show this help\n";
}
