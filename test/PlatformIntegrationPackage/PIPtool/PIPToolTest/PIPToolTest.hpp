/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.   *
*                                                       *
********************************************************/

#pragma once
#define TEST_FOLDER_PATH                 L"%temp%\\PIPToolTestFolder"
#define SOURCE_MANIFEST_FILENAME         L"PIPAppxManifest.xml"
#define OUTPUT_MANIFEST_FILENAME         L"AppxManifest.out.xml"
#define DEFAULT_OUTPUT_MANIFEST_FILENAME L"AppxManifest.xml"
#define FIND_COMMAND_PATH                L"%windir%\\System32\\find.exe"
#define DEFAULT_LIFETIME_MANAGER_CLSID   L"32E7CF70-038C-429a-BD49-88850F1B4A11"
#define DUMMY_LIFETIME_MANAGER_CLSID     L"185EE6F9-9C99-403B-A776-125272BC3804"
#define SOURCE_CODE_FILENAME             L"LifetimeManagerClsid.h"

// TODO: fix path
#define DATA_SOURCE_FOLDER               L"c:\\PR\\test\\PlatformIntegrationPackage\\PIPTool\\Data"

namespace Microsoft::Reunion::Sidecar
{
    namespace FunctionalTests
    {
         // TODO: fix path
        static PCWSTR PipToolPath = L"C:\\PR\\BuildOutput\\Debug\\x64\\PIPTool\\PIPTool.exe";
        static WCHAR TestFolderPath[MAX_PATH];
        static WCHAR SourceManifestPath[MAX_PATH];
        static WCHAR SourceCodeFilePath[MAX_PATH];
        static WCHAR TargetCodeFilePath[MAX_PATH];
        static WCHAR TargetManifestPath[MAX_PATH];
        static WCHAR OutputManifestPath[MAX_PATH];
        static WCHAR DefaultOutputManifestPath[MAX_PATH];
        static WCHAR FindCommandPath[MAX_PATH];

        BEGIN_MODULE()
            MODULE_PROPERTY(L"Feature", L"ReunionPIPTool")
        END_MODULE()

        MODULE_SETUP(ModuleSetup);
        MODULE_CLEANUP(ModuleCleanup);

        class PIPToolTest : public WEX::TestClass <PIPToolTest>
        {
        public:
            PIPToolTest()
            {}
            ~PIPToolTest()
            {}

            BEGIN_TEST_CLASS(PIPToolTest)
                TEST_CLASS_PROPERTY(L"Description", L"Test for Reunion PIPTool")
                TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
                TEST_CLASS_PROPERTY(L"RunAs", L"User")
                TEST_CLASS_PROPERTY(L"BinaryUnderTest", L"PIPTool.exe")
            END_TEST_CLASS()

            TEST_METHOD(BasicLaunch);
            TEST_METHOD(RemovePropertiesFromManifestOnly);
            TEST_METHOD(RemovePropertiesAndReplaceClsid);

            static HRESULT RunWithArgumentsAndWait(_In_ PCWSTR cmdString, _In_opt_ PCWSTR arguments, _In_ DWORD expectedResult);
            static HRESULT CleanupTestFolder();

        private:
            static HRESULT FileExists(_In_ PCWSTR filePath, _Out_ bool* exists);
            static HRESULT GetLastWriteTime(_In_ PCWSTR parentFolderPath, _In_ PCWSTR fileName, _Out_ PFILETIME lastWriteTime);
            static HRESULT GetFileSize(_In_ PCWSTR parentFolderPath, _In_ PCWSTR fileName, _Out_ PLARGE_INTEGER fileSize);
            static HRESULT VerifyPropertiesElementIsNotInFile(_In_ PCWSTR filePath);
            static HRESULT CheckIfDefaultClsidIsInFile(_In_ PCWSTR filePath, _In_ bool presenceExpected);
        };
    }
}