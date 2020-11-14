/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <wil/result.h>

#include <Windows.Foundation.h>
#include <Windows.h>

#include <BasicLogFile.h>

namespace Microsoft::Reunion::Sidecar
{
    HRESULT FileExists(_In_ PCWSTR filePath, _Out_ bool* exists)
    {
        const DWORD attributes = ::GetFileAttributesW(filePath);
        if (attributes == INVALID_FILE_ATTRIBUTES)
        {
            const DWORD lastError = GetLastError();
            RETURN_HR_IF_MSG(HRESULT_FROM_WIN32(lastError), (lastError != ERROR_FILE_NOT_FOUND) &&
                (lastError != ERROR_PATH_NOT_FOUND) && (lastError != ERROR_INVALID_DRIVE) &&
                (lastError != ERROR_BAD_NETPATH) && (lastError != ERROR_BAD_NET_NAME) &&
                (lastError != ERROR_NO_NET_OR_BAD_PATH), "GetFileAttributesW %ws", filePath);

            *exists = false;
            return S_OK;
        }

        RETURN_HR_IF_MSG(HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS), (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
            "GetFileAttributesW %ws", filePath);

        *exists = true;
        return S_OK;
    }

    HRESULT BasicLogFile::WriteLine(_In_ PCWSTR logLine)
    {
        const DWORD bytesToWrite = (DWORD)(sizeof(WCHAR) * wcslen(logLine));
        DWORD bytesWritten = 0;
        RETURN_IF_WIN32_BOOL_FALSE_MSG(WriteFile(
            this->logFileHandle.get(),
            logLine,
            bytesToWrite,
            &bytesWritten,
            nullptr),
            "LogLine %ws of len %u.", logLine, bytesToWrite);
        LOG_HR_IF_MSG(E_UNEXPECTED, bytesWritten < 1, "LogLine %ws of len %u written %u.",
            logLine, bytesToWrite, bytesWritten);
        return S_OK;
    }

    HRESULT BasicLogFile::WriteMsg(__in __format_string PCWSTR formatString, ...)
    {
        RETURN_HR_IF_NULL(E_INVALIDARG, formatString);
        RETURN_HR_IF(E_INVALIDARG, this->logFileHandle.get() == INVALID_HANDLE_VALUE);

        va_list args;
        va_start(args, formatString);
        WCHAR szMsg[MAX_PATH];
        const HRESULT hrVPrint = LOG_IF_FAILED_MSG(StringCchVPrintf(szMsg, ARRAYSIZE(szMsg), formatString, args),
            "Format string %ws.", formatString);
        va_end(args);
        RETURN_IF_FAILED_MSG(hrVPrint, "StringCchVPrintf %ws", formatString);

        SYSTEMTIME st;
        GetLocalTime(&st);

        WCHAR dataTimeBuf[MAX_PATH];
        RETURN_IF_FAILED_MSG(StringCchPrintf(
            dataTimeBuf,
            ARRAYSIZE(dataTimeBuf),
            L"%04u/%02u/%02u %02u:%02u:%02u.%03u", 
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds),
            "%u %u %u %u %u %u %u.", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        WCHAR msgBuf[2 * MAX_PATH];
        RETURN_IF_FAILED_MSG(StringCchPrintf(
            msgBuf,
            ARRAYSIZE(msgBuf),
            L"%ws %ws\r\n",
            dataTimeBuf,
            szMsg),
            "Date %ws msg %ws.", dataTimeBuf, szMsg);
        RETURN_IF_FAILED_MSG(WriteLine(msgBuf), "Msg %ws.", msgBuf);
        return S_OK;
    }

    HRESULT BasicLogFile::OpenLogFileAlways(_In_ PCWSTR logDirectory, _In_ PCWSTR instanceString)
    {
        RETURN_HR_IF_NULL(E_INVALIDARG, logDirectory);

        // Build full path to the log file.
        if (instanceString != nullptr)
        {
            RETURN_IF_FAILED_MSG(StringCchPrintf(this->logFileFullPath, ARRAYSIZE(this->logFileFullPath),
                L"%ws\\%ws-%ws.%ws", logDirectory, LOG_FILE_NAME, instanceString, LOG_FILE_EXTENSION),
                "StringCchPrintf %ws %ws %ws", logDirectory, LOG_FILE_NAME, instanceString);
        }
        else
        {
            RETURN_IF_FAILED_MSG(StringCchPrintf(this->logFileFullPath, ARRAYSIZE(this->logFileFullPath),
                L"%ws\\%ws.%ws", logDirectory, LOG_FILE_NAME, LOG_FILE_EXTENSION),
                "StringCchPrintf %ws %ws", logDirectory, LOG_FILE_NAME);
        }

        // If an old log file exists, copy it to a "backup" file name, overwriting the previous backup.
        bool exists = false;
        RETURN_IF_FAILED_MSG(FileExists(this->logFileFullPath, &exists), "FileExists %ws", this->logFileFullPath);

        if (exists)
        {
            WCHAR backupLogFilePath[MAX_PATH];
            if (instanceString != nullptr)
            {
                RETURN_IF_FAILED_MSG(StringCchPrintf(backupLogFilePath, ARRAYSIZE(backupLogFilePath),
                    L"%ws\\%ws-%ws.%ws", logDirectory, LOG_FILE_BACKUP_NAME, instanceString, LOG_FILE_EXTENSION),
                    "StringCchPrintf %ws %ws %ws", logDirectory, LOG_FILE_BACKUP_NAME, instanceString);
            }
            else
            {
                RETURN_IF_FAILED_MSG(StringCchPrintf(backupLogFilePath, ARRAYSIZE(backupLogFilePath),
                    L"%ws\\%ws.%ws", logDirectory, LOG_FILE_BACKUP_NAME, LOG_FILE_EXTENSION),
                    "StringCchPrintf %ws %ws", logDirectory, LOG_FILE_BACKUP_NAME);
            }

            LOG_IF_WIN32_BOOL_FALSE_MSG(MoveFileEx(this->logFileFullPath, backupLogFilePath,
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH),
                "MoveFileEx %ws %ws", this->logFileFullPath, backupLogFilePath);
        }

        // Create a new log file and write header into it.
        this->logFileHandle.reset(CreateFileW(
            this->logFileFullPath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr));
        if (this->logFileHandle.get() == INVALID_HANDLE_VALUE)
        {
            RETURN_HR_MSG(HRESULT_FROM_WIN32(GetLastError()), "CreateFileW %ws.", this->logFileFullPath);
        }

        WORD header = 0xFEFF;  // UTF-16 Byte Order Mark (big endian)
        DWORD bytesWritten = 0;
        RETURN_IF_WIN32_BOOL_FALSE(WriteFile(
            this->logFileHandle.get(),
            &header,
            sizeof(header),
            &bytesWritten,
            nullptr));
        LOG_HR_IF_MSG(E_UNEXPECTED, bytesWritten < 1, "Written %u.", bytesWritten);

        const DWORD myPid = GetCurrentProcessId();
        RETURN_IF_FAILED_MSG(WriteMsg(L"Begin logging for pid %d, %ws.", myPid, this->logFileFullPath),
            "%ws %d.", this->logFileFullPath, myPid);
        return S_OK;
    }
} // namespace Microsoft::Reunion::Sidecar
