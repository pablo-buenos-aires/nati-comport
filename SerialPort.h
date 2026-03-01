#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <string>
#include <windows.h>

class SerialPort
{
public:
    SerialPort();
    ~SerialPort();

    bool Open(const std::string &portName, DWORD baudRate, DWORD timeoutMs);
    void Close();

    bool Handshake(const std::string &toWrite,
                   const std::string &expect,
                   DWORD timeoutMs,
                   std::string &received,
                   std::string &errorText);

private:
    HANDLE m_handle;

    bool Configure(DWORD baudRate, DWORD timeoutMs);
    bool WriteAll(const std::string &data);
    bool ReadLineWithTimeout(DWORD timeoutMs, std::string &line);
    std::string BuildPortPath(const std::string &portName) const;
};

#endif
