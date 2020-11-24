/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include <wil\resource.h>

#define LOG_FILE_NAME                   L"PIP"
#define LOG_FILE_EXTENSION              L"log"
#define LOG_FILE_BACKUP_NAME            L"PIP.bak"

namespace Microsoft::Reunion::Sidecar
{
    class BasicLogFile
    {
    public:

        HRESULT WriteMsg(_In_ __format_string PCWSTR formatString, ...);

        BasicLogFile() : logFileHandle(INVALID_HANDLE_VALUE) {}
        ~BasicLogFile()
        {
            LOG_IF_FAILED(WriteMsg(L"The End."));
        }

        HRESULT OpenLogFileAlways(_In_ PCWSTR logDirectory, _In_ PCWSTR instanceString);

    private:
        HRESULT WriteLine(_In_ PCWSTR logLine);

        WCHAR logFileFullPath[MAX_PATH] = {};
        wil::unique_handle logFileHandle{};
    };
} // namespace Microsoft::Reunion::Sidecar
