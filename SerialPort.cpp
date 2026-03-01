#include "SerialPort.h"

SerialPort::SerialPort()
    : m_handle(INVALID_HANDLE_VALUE)
{
}

SerialPort::~SerialPort()
{
    Close();
}

bool SerialPort::Open(const std::string &portName, DWORD baudRate, DWORD timeoutMs)
{
    Close();

    const std::string path = BuildPortPath(portName);
    m_handle = CreateFileA(path.c_str(),
                           GENERIC_READ | GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (m_handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    if (!Configure(baudRate, timeoutMs))
    {
        Close();
        return false;
    }

    PurgeComm(m_handle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    return true;
}

void SerialPort::Close()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort::Handshake(const std::string &toWrite,
                           const std::string &expect,
                           DWORD timeoutMs,
                           std::string &received,
                           std::string &errorText)
{
    if (m_handle == INVALID_HANDLE_VALUE)
    {
        errorText = "Port is not open";
        return false;
    }

    if (!WriteAll(toWrite))
    {
        errorText = "WriteFile failed";
        return false;
    }

    FlushFileBuffers(m_handle);

    if (!ReadLineWithTimeout(timeoutMs, received))
    {
        errorText = "Read timeout or ReadFile failed";
        return false;
    }

    if (!expect.empty())
    {
        if (received.find(expect) == std::string::npos)
        {
            errorText = "Expected response fragment not found";
            return false;
        }
    }

    return true;
}

bool SerialPort::Configure(DWORD baudRate, DWORD timeoutMs)
{
    SetupComm(m_handle, 1024, 1024);

    DCB dcb;
    ZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(m_handle, &dcb))
    {
        return false;
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError = FALSE;

    if (!SetCommState(m_handle, &dcb))
    {
        return false;
    }

    COMMTIMEOUTS timeouts;
    ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = timeoutMs;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = timeoutMs;

    if (!SetCommTimeouts(m_handle, &timeouts))
    {
        return false;
    }

    return true;
}

bool SerialPort::WriteAll(const std::string &data)
{
    const char *buffer = data.c_str();
    DWORD left = static_cast<DWORD>(data.size());
    DWORD offset = 0;

    while (left > 0)
    {
        DWORD written = 0;
        if (!WriteFile(m_handle, buffer + offset, left, &written, NULL))
        {
            return false;
        }
        if (written == 0)
        {
            return false;
        }

        offset += written;
        left -= written;
    }

    return true;
}

bool SerialPort::ReadLineWithTimeout(DWORD timeoutMs, std::string &line)
{
    line.clear();
    const DWORD startTick = GetTickCount();

    for (;;)
    {
        char ch = 0;
        DWORD readBytes = 0;
        if (!ReadFile(m_handle, &ch, 1, &readBytes, NULL))
        {
            return false;
        }

        if (readBytes == 1)
        {
            line.push_back(ch);
            if (ch == '\n')
            {
                return true;
            }
        }

        const DWORD elapsed = GetTickCount() - startTick;
        if (elapsed >= timeoutMs)
        {
            return !line.empty();
        }
    }
}

std::string SerialPort::BuildPortPath(const std::string &portName) const
{
    if (portName.size() >= 4 && portName.substr(0, 4) == "\\\\.\\")
    {
        return portName;
    }
    return std::string("\\\\.\\") + portName;
}
