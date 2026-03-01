#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace Logger
{
    void Info(const std::string &message);
    void Warn(const std::string &message);
    void Error(const std::string &message);

    std::string ToStringUnsignedLong(unsigned long value);
    std::string Timestamp();
    std::string MakePrintable(const std::string &input);
}

#endif
