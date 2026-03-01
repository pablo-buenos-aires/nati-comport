#include "Logger.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>

namespace
{
    void Log(const char *level, const std::string &message)
    {
        std::cout << Logger::Timestamp() << " [" << level << "] " << message << std::endl;
    }
}

namespace Logger
{
    void Info(const std::string &message)
    {
        Log("INFO", message);
    }

    void Warn(const std::string &message)
    {
        Log("WARN", message);
    }

    void Error(const std::string &message)
    {
        Log("ERROR", message);
    }

    std::string ToStringUnsignedLong(unsigned long value)
    {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    std::string Timestamp()
    {
        SYSTEMTIME st;
        GetLocalTime(&st);

        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << st.wYear << "-"
            << std::setw(2) << st.wMonth << "-"
            << std::setw(2) << st.wDay << " "
            << std::setw(2) << st.wHour << ":"
            << std::setw(2) << st.wMinute << ":"
            << std::setw(2) << st.wSecond << "."
            << std::setw(3) << st.wMilliseconds;
        return oss.str();
    }

    std::string MakePrintable(const std::string &input)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < input.size(); ++i)
        {
            const unsigned char c = static_cast<unsigned char>(input[i]);
            if (c == '\r')
            {
                oss << "\\r";
            }
            else if (c == '\n')
            {
                oss << "\\n";
            }
            else if (c == '\t')
            {
                oss << "\\t";
            }
            else if (c >= 32 && c <= 126)
            {
                oss << input[i];
            }
            else
            {
                oss << "\\x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                    << static_cast<int>(c) << std::dec;
            }
        }
        return oss.str();
    }
}
