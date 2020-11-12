/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.   *
*                                                       *
********************************************************/

#include <pch.h>

#include <stdio.h>
#include <stdlib.h>
#include <ShellAPI.h>

#include <PIPToolTest.hpp>

using namespace WEX::TestExecution;
using namespace WEX::Logging;
using namespace WEX::Common;

namespace Microsoft::Reunion::Sidecar::FunctionalTests
{
    bool ModuleSetup()
    {
        VERIFY_IS_GREATER_THAN(ExpandEnvironmentStrings(TEST_FOLDER_PATH, TestFolderPath, ARRAYSIZE(TestFolderPath)), (DWORD)0);
        VERIFY_IS_GREATER_THAN(ExpandEnvironmentStrings(FIND_COMMAND_PATH, FindCommandPath, ARRAYSIZE(FindCommandPath)), (DWORD)0);

        VERIFY_SUCCEEDED(StringCchPrintf(SourceManifestPath, ARRAYSIZE(SourceManifestPath), L"%ws\\%ws", DATA_SOURCE_FOLDER,
            SOURCE_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(StringCchPrintf(TargetManifestPath, ARRAYSIZE(TargetManifestPath), L"%ws\\%ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(StringCchPrintf(OutputManifestPath, ARRAYSIZE(OutputManifestPath), L"%ws\\%ws", TestFolderPath,
            OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(StringCchPrintf(DefaultOutputManifestPath, ARRAYSIZE(DefaultOutputManifestPath), L"%ws\\%ws", TestFolderPath,
            DEFAULT_OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(StringCchPrintf(SourceCodeFilePath, ARRAYSIZE(SourceCodeFilePath), L"%ws\\%ws", DATA_SOURCE_FOLDER,
            SOURCE_CODE_FILENAME));
        VERIFY_SUCCEEDED(StringCchPrintf(TargetCodeFilePath, ARRAYSIZE(TargetCodeFilePath), L"%ws\\%ws", TestFolderPath,
            SOURCE_CODE_FILENAME));

        VERIFY_SUCCEEDED(PIPToolTest::CleanupTestFolder());

        if (!CreateDirectory(TestFolderPath, nullptr))
        {
            Log::Comment(WEX::Common::String().Format(L"CreateDirectory %ws, 0x%0x", TestFolderPath, GetLastError()));
        }

        return true;
    }

    bool ModuleCleanup()
    {
        VERIFY_SUCCEEDED(PIPToolTest::CleanupTestFolder());
        return true;
    }

    HRESULT PIPToolTest::CleanupTestFolder()
    {
        DeleteFile(TargetManifestPath);
        DeleteFile(OutputManifestPath);
        DeleteFile(DefaultOutputManifestPath);
        DeleteFile(TargetCodeFilePath);

        if (!RemoveDirectory(TestFolderPath))
        {
            Log::Comment(WEX::Common::String().Format(L"RemoveDirectory %ws, 0x%0x", TestFolderPath, GetLastError()));
        }
        return S_OK;
    }

    HRESULT PIPToolTest::FileExists(_In_ PCWSTR filePath, _Out_ bool* exists)
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

    HRESULT PIPToolTest::GetLastWriteTime(_In_ PCWSTR parentFolderPath, _In_ PCWSTR fileName, _Out_ PFILETIME lastWriteTime)
    {
        WCHAR filePathBuffer[MAX_PATH];
        VERIFY_SUCCEEDED(StringCchPrintf(filePathBuffer, ARRAYSIZE(filePathBuffer), L"%ws\\%ws", parentFolderPath, fileName));

        wil::unique_hfile fileHandle(CreateFile(filePathBuffer,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr));
        VERIFY_IS_FALSE(!fileHandle);

        VERIFY_IS_TRUE(GetFileTime(fileHandle.get(), nullptr, nullptr, lastWriteTime));
        return S_OK;
    }

    HRESULT PIPToolTest::GetFileSize(_In_ PCWSTR parentFolderPath, _In_ PCWSTR fileName, _Out_ PLARGE_INTEGER fileSize)
    {
        WCHAR filePathBuffer[MAX_PATH];
        VERIFY_SUCCEEDED(StringCchPrintf(filePathBuffer, ARRAYSIZE(filePathBuffer), L"%ws\\%ws", parentFolderPath, fileName));

        wil::unique_hfile fileHandle(CreateFile(filePathBuffer,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr));
        VERIFY_IS_FALSE(!fileHandle);

        VERIFY_IS_TRUE(GetFileSizeEx(fileHandle.get(), fileSize));
        return S_OK;
    }

    HRESULT PIPToolTest::RunWithArgumentsAndWait(
        _In_ PCWSTR cmdString,
        _In_opt_ PCWSTR arguments,
        _In_ DWORD expectedResult)
    {
        SHELLEXECUTEINFO sei{};
        sei.cbSize = sizeof(sei);
        sei.nShow = SW_SHOW;
        sei.fMask |= (SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOZONECHECKS);
        sei.lpFile = cmdString;
        sei.lpParameters = arguments;
        if (sei.lpParameters != nullptr)
        {
            Log::Comment((WEX::Common::String().Format(L"ShellExecute %ws %ws", sei.lpFile, sei.lpParameters)));
        }
        else
        {
            Log::Comment((WEX::Common::String().Format(L"ShellExecute sei.lpFile: %ws", sei.lpFile)));
        }
        VERIFY_IS_TRUE(ShellExecuteEx(&sei));

        wil::unique_process_information procInfo;
        procInfo.hProcess = sei.hProcess;
        const DWORD retValue = WaitForSingleObject(procInfo.hProcess, INFINITE);
        VERIFY_ARE_EQUAL(retValue, (DWORD)WAIT_OBJECT_0);

        DWORD procExitCode;
        VERIFY_IS_TRUE(GetExitCodeProcess(procInfo.hProcess, &procExitCode));
        VERIFY_ARE_EQUAL(expectedResult, procExitCode);
        return S_OK;
    }

    HRESULT PIPToolTest::VerifyPropertiesElementIsNotInFile(_In_ PCWSTR filePath)
    {
        WCHAR argumentString[MAX_PATH];
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"/i \"CheeseCake\" %ws", filePath));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(FindCommandPath, argumentString, 1),
            L"Properties element should _not_ be in the output manifest.");
        return S_OK;
    }

    HRESULT PIPToolTest::CheckIfDefaultClsidIsInFile(_In_ PCWSTR filePath, _In_ bool presenceExpected)
    {
        WCHAR argumentString[MAX_PATH];
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"/i \"%ws\" %ws",
            DEFAULT_LIFETIME_MANAGER_CLSID, filePath));

        if (presenceExpected)
        {
            VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(FindCommandPath, argumentString, 0),
                L"Default CLSID should still be in the target file.");
        }
        else
        {
            VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(FindCommandPath, argumentString, 1),
                L"Default CLSID should _not_ be in the target file.");
        }
        return S_OK;
    }

    void PIPToolTest::BasicLaunch()
    {
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, nullptr, 1));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, L"-h", 1));
        return;
    }

    void PIPToolTest::RemovePropertiesFromManifestOnly()
    {
        // 1. Copy test manifest to a temp location
        if (!CopyFile(SourceManifestPath, TargetManifestPath, FALSE))
        {
            Log::Comment(WEX::Common::String().Format(L"CopyFile %ws %ws, 0x%0x", SourceManifestPath, TargetManifestPath,
                GetLastError()));
        }
        DeleteFile(DefaultOutputManifestPath);
        DeleteFile(OutputManifestPath);

        // 2. Exercise the -n flag
        WCHAR argumentString[MAX_PATH];
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws %ws -k -n", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        bool fileExists = false;
        VERIFY_SUCCEEDED(FileExists(OutputManifestPath, &fileExists));
        VERIFY_IS_FALSE(fileExists, L"With the -n flag, no output manifest should be created.");

        // 3. Exercise the -k flag, PIPAppxManifest.xml -> AppxManifest.out.xml
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws %ws -k", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        VERIFY_SUCCEEDED(FileExists(OutputManifestPath, &fileExists));
        VERIFY_IS_TRUE(fileExists, L"Without the -n flag, output manifest should be created.");

        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(OutputManifestPath));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(OutputManifestPath, true));

        FILETIME outputFiletime1;
        VERIFY_SUCCEEDED(GetLastWriteTime(TestFolderPath, OUTPUT_MANIFEST_FILENAME, &outputFiletime1));

        // 4. Verify that the output manifest is _not_ overwritten if the input manifest hasn't changed.
        //    PIPAppxManifest.xml -> AppxManifest.out.xml
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws %ws -k", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        FILETIME outputFiletime2;
        VERIFY_SUCCEEDED(GetLastWriteTime(TestFolderPath, OUTPUT_MANIFEST_FILENAME, &outputFiletime2));
        VERIFY_ARE_EQUAL(CompareFileTime(&outputFiletime1, &outputFiletime2), (LONG)0);

        // 5. Verify that the output manifest is overwritten even if the input manifest hasn't changed if the -o flag is present.
        //    PIPAppxManifest.xml -> AppxManifest.out.xml
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws %ws -k -o", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        VERIFY_SUCCEEDED(GetLastWriteTime(TestFolderPath, OUTPUT_MANIFEST_FILENAME, &outputFiletime2));
        VERIFY_IS_LESS_THAN(CompareFileTime(&outputFiletime1, &outputFiletime2), (LONG)0);

        // 6. Verify that the output manifest is equal to the input if the input has already processed.
        //    AppxManifest.out.xml -> AppxManifest.xml
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -k -v", TestFolderPath,
            OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        LARGE_INTEGER sourceFileSize;
        LARGE_INTEGER targetFileSize;
        VERIFY_SUCCEEDED(GetFileSize(TestFolderPath, OUTPUT_MANIFEST_FILENAME, &sourceFileSize));
        VERIFY_SUCCEEDED(GetFileSize(TestFolderPath, DEFAULT_OUTPUT_MANIFEST_FILENAME, &targetFileSize));
        VERIFY_IS_TRUE(sourceFileSize.QuadPart == targetFileSize.QuadPart);

        // 7. Verify that without the -k flag, the default CLSID in the manifest is being replaced.
        //    AppxManifest.out.xml -> AppxManifest.xml
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -o -v", TestFolderPath,
            OUTPUT_MANIFEST_FILENAME));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        // Without the -k flag, the Default CLSID should _not_ be found in the output manifest.
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(DefaultOutputManifestPath, false));
        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(DefaultOutputManifestPath));
        return;
    }

    void PIPToolTest::RemovePropertiesAndReplaceClsid()
    {
        // 1. Copy test source code and manifest files to a temp location
        if (!CopyFile(SourceCodeFilePath, TargetCodeFilePath, FALSE))
        {
            Log::Comment(WEX::Common::String().Format(L"CopyFile %ws %ws, 0x%0x", SourceCodeFilePath, TargetCodeFilePath,
                GetLastError()));
        }
        if (!CopyFile(SourceManifestPath, TargetManifestPath, FALSE))
        {
            Log::Comment(WEX::Common::String().Format(L"CopyFile %ws %ws, 0x%0x", SourceManifestPath, TargetManifestPath,
                GetLastError()));
        }

        LARGE_INTEGER sourceCodeFileSize1;
        VERIFY_SUCCEEDED(GetFileSize(TestFolderPath, SOURCE_CODE_FILENAME, &sourceCodeFileSize1));

        DeleteFile(DefaultOutputManifestPath);
        DeleteFile(OutputManifestPath);

        // 2. Verify that, with the -n flag, no changes are being written.
        FILETIME outputFiletime1;
        VERIFY_SUCCEEDED(GetLastWriteTime(TestFolderPath, SOURCE_CODE_FILENAME, &outputFiletime1));

        WCHAR argumentString[MAX_PATH];
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -k -n -fp %ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, TargetCodeFilePath));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        bool fileExists = false;
        VERIFY_SUCCEEDED(FileExists(DefaultOutputManifestPath, &fileExists));
        VERIFY_IS_FALSE(fileExists, L"With the -n flag, no output manifest should be created.");

        FILETIME outputFiletime2;
        VERIFY_SUCCEEDED(GetLastWriteTime(TestFolderPath, SOURCE_CODE_FILENAME, &outputFiletime2));
        VERIFY_ARE_EQUAL(CompareFileTime(&outputFiletime1, &outputFiletime2), (LONG)0,
            L"With the -n flag, the target source code file shouldn't be updated.");

        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(TargetCodeFilePath, true));

        // 3. Verify that, with the -k flag but not the -n flag, properties elements are removed from the manifest, but
        //    no changes are done to the source code file.
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -o -k -fp %ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, TargetCodeFilePath));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(DefaultOutputManifestPath));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(DefaultOutputManifestPath, true));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(TargetCodeFilePath, true));

        VERIFY_SUCCEEDED(GetLastWriteTime(TestFolderPath, SOURCE_CODE_FILENAME, &outputFiletime2));
        VERIFY_ARE_EQUAL(CompareFileTime(&outputFiletime1, &outputFiletime2), (LONG)0,
            L"With the -k flag, the target source code file shouldn't be updated.");

        // 4. Verify that, without the -n and -k flag, but with the -c flag specifying a random CLSID, properties elements
        // are removed from the manifest, but the default CLSID is _not_ being replaced in both the manifest file and in the
        // source code file.
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -o -fp %ws -c %ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, TargetCodeFilePath, DUMMY_LIFETIME_MANAGER_CLSID));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(DefaultOutputManifestPath));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(DefaultOutputManifestPath, true));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(TargetCodeFilePath, true));

        // 5. Verify that, without the -n and -k flag, properties elements are removed from the manifest, the default
        //    CLSID is being replaced in both the manifest file and in the source code file.
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -o -fp %ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, TargetCodeFilePath));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(DefaultOutputManifestPath));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(DefaultOutputManifestPath, false));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(TargetCodeFilePath, false));

        // 6. Verify that, with the -c flag specifying a random GUID, the default CLSID in the manifest should _not_ be replaced.
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -o -fp %ws -c %ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, TargetCodeFilePath, DUMMY_LIFETIME_MANAGER_CLSID));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(DefaultOutputManifestPath));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(DefaultOutputManifestPath, true));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(TargetCodeFilePath, false));

        // 7. Verify that, with the -c flag specifying the default CLSID, its instance in the manifest should be replaced.
        VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ws %ws -o -fp %ws -c %ws", TestFolderPath,
            SOURCE_MANIFEST_FILENAME, TargetCodeFilePath, DEFAULT_LIFETIME_MANAGER_CLSID));
        VERIFY_SUCCEEDED(PIPToolTest::RunWithArgumentsAndWait(PipToolPath, argumentString, 0));

        // Expect to get the same outcome as case #5 above.
        VERIFY_SUCCEEDED(VerifyPropertiesElementIsNotInFile(DefaultOutputManifestPath));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(DefaultOutputManifestPath, false));
        VERIFY_SUCCEEDED(CheckIfDefaultClsidIsInFile(TargetCodeFilePath, false));

        LARGE_INTEGER sourceCodeFileSize2;
        VERIFY_SUCCEEDED(GetFileSize(TestFolderPath, SOURCE_CODE_FILENAME, &sourceCodeFileSize2));
        VERIFY_IS_TRUE(sourceCodeFileSize1.QuadPart == sourceCodeFileSize2.QuadPart,
            L"After all these steps, the source code file size shouldn't have changed.");
        return;
    }
}
