/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.   *
*                                                       *
********************************************************/

#pragma once

#define FIND_COMMAND_PATH                L"%windir%\\System32\\find.exe"
#define DEFAULT_LIFETIME_MANAGER_CLSID   L"32E7CF70-038C-429a-BD49-88850F1B4A11"
#define DUMMY_LIFETIME_MANAGER_CLSID     L"185EE6F9-9C99-403B-A776-125272BC3804"
#define TEMP_FOLDER_PATH_PREFIX          L"%LocalAppData%\\Packages\\"
#define CENTENNIAL_LOG_FILE_TEMPLATE     L"PIP-Centennial-*.log"
#define WINDOWS_APPS_PATH_PREFIX         L"%ProgramFiles%\\WindowsApps\\"

#define PIP_FAMILY_NAME                  L"SidecarMainApp_8wekyb3d8bbwe"
#define PIP_FULL_NAME                    L"SidecarMainApp_1.0.0.1_neutral_en-us_8wekyb3d8bbwe"
#define FRAMEWORK_FULL_NAME              L"ReunionFrameworkPlaceholder_1.0.0.0_neutral_en-us_8wekyb3d8bbwe"
#define CENTENNIAL_APP_AUMID             L"SidecarMainApp_8wekyb3d8bbwe!SidecarCentennialApp"
#define DUMMY_CENTENNIAL_APP_AUMID       L"WrongName_8wekyb3d8bbwe!wrongAUMID"

// TODO: fix paths
#define PIP_APPX_PATH                    L"c:\\pr2\\BuildOutput\\Debug\\x64\\PIPMain.Msix\\PIPMain.Msix"
#define FRAMEWORK_APPX_PATH              L"C:\\pr2\\BuildOutput\\Debug\\x64\\PIPFramework.Msix\\PIPFramework.Msix"

namespace Microsoft::Reunion::Sidecar
{
    namespace FunctionalTests
    {
        // TODO: fix path
        static PCWSTR PipLauncherPath = L"C:\\PR2\\BuildOutput\\Debug\\x64\\PIPLauncher\\PIPLauncher.exe";
        static WCHAR FindCommandPath[MAX_PATH];
        static WCHAR TempFolderPath[MAX_PATH];
        static WCHAR PackageRootPath[MAX_PATH];

        BEGIN_MODULE()
            MODULE_PROPERTY(L"Feature", L"ReunionPIPTool")
        END_MODULE()

        MODULE_SETUP(ModuleSetup);
        MODULE_CLEANUP(ModuleCleanup);

        class PIPActivationTest : public WEX::TestClass <PIPActivationTest>
        {
        public:
            PIPActivationTest()
            {}
            ~PIPActivationTest()
            {}

            BEGIN_TEST_CLASS(PIPActivationTest)
                TEST_CLASS_PROPERTY(L"Description", L"Test for Reunion PIP activation")
                TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
                TEST_CLASS_PROPERTY(L"RunAs", L"User")
                TEST_CLASS_PROPERTY(L"BinaryUnderTest", L"PIPLauncher.exe")
            END_TEST_CLASS()

            TEST_METHOD(BasicLaunch);
            TEST_METHOD(LaunchByAUMID);
            TEST_METHOD(DynDepLifetimeManagerLaunch);

            static HRESULT AddTestPackage(_In_ PCWSTR packagePath);
            static HRESULT RemoveTestPackage(_In_ PCWSTR packageFullName);

        private:
            static HRESULT RunWithArgumentsAndWait(_In_ PCWSTR cmdString, _In_opt_ PCWSTR arguments, _In_ DWORD expectedResult);
            static HRESULT CleanupTempFolder();
            static HRESULT VerifyPIPActivatedByAumid(_In_ bool checkExecutablePath);
            static HRESULT VerifyDDLMIsActivated();

            static HRESULT CheckStringValueByNameInCollection(
                _In_ PCWSTR valueName,
                _In_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> settingsCollection,
                _In_ PCWSTR expectedName);
        };
    }
}
