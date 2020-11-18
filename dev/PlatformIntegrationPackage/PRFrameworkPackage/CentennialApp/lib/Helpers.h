//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <pch.h>

#define LOG_MESSAGE(Format, ...)    Microsoft::Reunion::Sidecar::LogDebugMessage((Format), __VA_ARGS__)

#define LOG_AND_RETURN_IF_FAILED(op, msg) \
{ \
    const HRESULT localHr = op; \
    if (FAILED(localHr)) \
    { \
        LOG_MESSAGE(L"%s, 0x%0x", msg, localHr); \
        RETURN_HR(localHr); \
    } \
}

namespace Microsoft::Reunion::Sidecar
{
    void LogDebugMessage(PCWSTR format, ...);
    HRESULT UninitializeLogFile();
    HRESULT EnsureAppDataOpened();
    HRESULT EnsureAppDataPopulated();
    HRESULT EnsureLogFileInitialized(PCWSTR instanceString);
    HRESULT ProcessInitializationFlag(std::wstring args);
}
