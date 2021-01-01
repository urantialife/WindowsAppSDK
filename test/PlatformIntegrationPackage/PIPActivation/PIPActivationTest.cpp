/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.   *
*                                                       *
********************************************************/

#include <pch.h>

#include <stdio.h>
#include <stdlib.h>
#include <ShellAPI.h>

#include <PIPActivationTest.hpp>

using namespace WEX::TestExecution;
using namespace WEX::Logging;
using namespace WEX::Common;

namespace Microsoft::Reunion::Sidecar::FunctionalTests
{
bool ModuleSetup()
{
    winrt::init_apartment();

    // Build paths
    VERIFY_IS_GREATER_THAN(ExpandEnvironmentStrings(FIND_COMMAND_PATH, FindCommandPath, ARRAYSIZE(FindCommandPath)), (DWORD)0);
    VERIFY_IS_GREATER_THAN(ExpandEnvironmentStrings(TEMP_FOLDER_PATH_PREFIX, TempFolderPath, ARRAYSIZE(TempFolderPath)),
        (DWORD)0);

    VERIFY_SUCCEEDED(StringCchCat(TempFolderPath, ARRAYSIZE(TempFolderPath), PIP_FAMILY_NAME));
    VERIFY_SUCCEEDED(StringCchCat(TempFolderPath, ARRAYSIZE(TempFolderPath), L"\\TempState"));

    VERIFY_IS_GREATER_THAN(ExpandEnvironmentStrings(WINDOWS_APPS_PATH_PREFIX, PackageRootPath, ARRAYSIZE(PackageRootPath)),
        (DWORD)0);

    VERIFY_SUCCEEDED(StringCchCat(PackageRootPath, ARRAYSIZE(PackageRootPath), PIP_FULL_NAME));

    // Install framework package before main package
    // Comment out them to manually install the packages before testing.
#if 1
    VERIFY_SUCCEEDED(PIPActivationTest::AddTestPackage(FRAMEWORK_APPX_PATH));
    VERIFY_SUCCEEDED(PIPActivationTest::AddTestPackage(PIP_APPX_PATH));
#endif
    return true;
}

bool ModuleCleanup()
{
    // Remove main package before framework package
    // Comment out them to manually uninstall the packages after testing.
#if 1
    VERIFY_SUCCEEDED_RETURN(PIPActivationTest::RemoveTestPackage(PIP_FULL_NAME));
    VERIFY_SUCCEEDED_RETURN(PIPActivationTest::RemoveTestPackage(FRAMEWORK_FULL_NAME));
#endif
    winrt::uninit_apartment();
    return true;
}

// Delete all log files under the PIP's TempState folder
HRESULT PIPActivationTest::CleanupTempFolder()
{
    WCHAR searchString[MAX_PATH];
    VERIFY_SUCCEEDED(StringCchPrintf(searchString, ARRAYSIZE(searchString), L"%ls\\%ls",
        TempFolderPath, CENTENNIAL_LOG_FILE_TEMPLATE));
    Log::Comment(WEX::Common::String().Format(L"SearchString %ws", searchString));

    WIN32_FIND_DATA subDirectoryData;
    wil::unique_hfile subDirectoryHandle(FindFirstFile(searchString, &subDirectoryData));
    Log::Comment(WEX::Common::String().Format(L"SearchString %ws %p", searchString, (PVOID)subDirectoryHandle.get()));
    if (subDirectoryHandle.get() == INVALID_HANDLE_VALUE)
    {
        const DWORD err = GetLastError();
        RETURN_HR_IF_EXPECTED(S_OK, (err == ERROR_FILE_NOT_FOUND) || (err == ERROR_NOT_FOUND));
        VERIFY_IS_TRUE(err == ERROR_SUCCESS);
    }

    do
    {
        if (!(subDirectoryData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            WCHAR logFilePath[MAX_PATH];
            VERIFY_SUCCEEDED(StringCchPrintf(logFilePath, ARRAYSIZE(logFilePath), L"%ls\\%ls",
                TempFolderPath, subDirectoryData.cFileName));
            Log::Comment(WEX::Common::String().Format(L"DeleteFile %ws", logFilePath));
            VERIFY_IS_TRUE(DeleteFile(logFilePath));
        }
    } while (FindNextFile(subDirectoryHandle.get(), &subDirectoryData));
    return S_OK;
}

// Run command with optional argument using ShellExecute and then wait for process termination
HRESULT PIPActivationTest::RunWithArgumentsAndWait(
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

    // In case we get back a "not found" error, wait a bit, it appears that flushing the log file to disk takes time.
    DWORD procExitCode = 0;
    DWORD retryCount = 10;
    do
    {
        Sleep(250);
        VERIFY_IS_TRUE(ShellExecuteEx(&sei));
        wil::unique_handle processHandle(sei.hProcess);
        const DWORD retValue = WaitForSingleObject(processHandle.get(), INFINITE);
        VERIFY_ARE_EQUAL(retValue, (DWORD)WAIT_OBJECT_0);

        VERIFY_IS_TRUE(GetExitCodeProcess(processHandle.get(), &procExitCode));
    } while ((procExitCode == 2) && (retryCount-- > 0));
    VERIFY_ARE_EQUAL(expectedResult, procExitCode);

    // Mitigate race condition.
    Sleep(200);
    return S_OK;
}

void PIPActivationTest::BasicLaunch()
{
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, nullptr, 1));
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, L"--Help", 1));
    return;
}

void PIPActivationTest::LaunchByAUMID()
{
    // 1. Specifying the wrong AUMID should _not_ activate the PIP.
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, DUMMY_CENTENNIAL_APP_AUMID, 1));

    // 2. Specifying the right AUMID should activate the PIP.
    VERIFY_SUCCEEDED(CleanupTempFolder());
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, CENTENNIAL_APP_AUMID, 0));

    // Give time for log file to flush.
    Sleep(100);

    VERIFY_SUCCEEDED(VerifyPIPActivatedByAumid(false));

    // 3. Specifying the --Init flag should put the path into the PIP's Settings.
    VERIFY_SUCCEEDED(CleanupTempFolder());

    WCHAR argumentString[MAX_PATH];
    VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"%ls --Init \"%ls\"",
        CENTENNIAL_APP_AUMID, PipLauncherPath));
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, argumentString, 0));
    VERIFY_SUCCEEDED(VerifyPIPActivatedByAumid(true));
    return;
}

void PIPActivationTest::DynDepLifetimeManagerLaunch()
{
    VERIFY_SUCCEEDED(CleanupTempFolder());

    // 1. Specifying a _wrong_ CLSID should _not_ activate the DDLM.
    WCHAR argumentString[MAX_PATH];
    VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"--Invoke {%ls}",
        DUMMY_LIFETIME_MANAGER_CLSID));
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, argumentString, 1));

    // 2. Specifying the right default CLSID should activate the DDLM.
    VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString), L"--Invoke {%ls}",
        DEFAULT_LIFETIME_MANAGER_CLSID));
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, argumentString, 0));
    VerifyDDLMIsActivated();

    // 3. Using the built-in default CLSID should also activate the DDLM.
    VERIFY_SUCCEEDED(CleanupTempFolder());
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(PipLauncherPath, L"--Invoke", 0));
    VerifyDDLMIsActivated();
    return;
}

HRESULT PIPActivationTest::VerifyDDLMIsActivated()
{
    WCHAR argumentString[MAX_PATH];
    VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString),
        L"/i \"MyLifetimeManagerImpl::GetCurrentPackageFullName\" %ls\\%ls", TempFolderPath, CENTENNIAL_LOG_FILE_TEMPLATE));
    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(FindCommandPath, argumentString, 0),
        L"The log should show that a DDLM method has been invoked.");
    return S_OK;
}

HRESULT PIPActivationTest::AddTestPackage(_In_ PCWSTR packagePath)
{
     const winrt::hstring packagePathStr = packagePath;
     auto uriFactory = winrt::get_activation_factory<winrt::Windows::Foundation::Uri, winrt::Windows::Foundation::IUriRuntimeClassFactory>();
     auto packagePathUri = uriFactory.CreateUri(packagePathStr);

     winrt::Windows::Management::Deployment::PackageManager packageManager;
     auto result = packageManager.AddPackageAsync(packagePathUri, nullptr, winrt::Windows::Management::Deployment::DeploymentOptions::None);

     const auto deploymentResult = result.get();
     VERIFY_IS_TRUE(deploymentResult.IsRegistered() || (deploymentResult.ExtendedErrorCode() == HRESULT_FROM_WIN32(ERROR_PACKAGE_ALREADY_EXISTS)));
     return S_OK;
}

HRESULT PIPActivationTest::RemoveTestPackage(_In_ PCWSTR packageFullName)
{
    const winrt::hstring packageFullNameString = packageFullName;
    winrt::Windows::Management::Deployment::PackageManager packageManager;
    auto result = packageManager.RemovePackageAsync(packageFullNameString);

    const auto deploymentResult = result.get();
    VERIFY_SUCCEEDED(deploymentResult.ExtendedErrorCode());
    return S_OK;
}

HRESULT PIPActivationTest::VerifyPIPActivatedByAumid(_In_ bool checkExecutablePath)
{
    // 1. Verify that log file says the PIP was activated by AUMID.
    WCHAR argumentString[MAX_PATH];
    VERIFY_SUCCEEDED(StringCchPrintf(argumentString, ARRAYSIZE(argumentString),
        L"/i \"wmain::activatedByAumid, 0x1\" %ls\\%ls", TempFolderPath, CENTENNIAL_LOG_FILE_TEMPLATE));

    VERIFY_SUCCEEDED(RunWithArgumentsAndWait(FindCommandPath, argumentString, 0));

    // Setup access to the PIP's Local Settings
    const winrt::hstring familyName = PIP_FAMILY_NAME;
    auto appData = winrt::Windows::Management::Core::ApplicationDataManager::CreateForPackageFamily(familyName);

    // Sample layout under the PIP's Local Settings:
    // LOCAL
    //    LifetimeManagerClsid    String    32E7CF70-038C-429a-BD49-88850F1B4A11
    //    PackageRoot    String    C:\Program Files\WindowsApps\SidecarMainApp_1.0.0.1_neutral_en-us_8wekyb3d8bbwe
    //    Initialized    Boolean    True
    // LOCAL\Extensions
    // LOCAL\Extensions\SidecarCentennialApp
    //    ExecutablePath    String    c:\os2\obj\amd64fre\onecore\admin\appmodel\tools\prstaging\test\PlatformIntegrationPackage\PIPLauncher\objfre\amd64\PIPtool.exe
    // LOCAL\Extensions\SidecarCentennialApp\windows.comInterface
    // LOCAL\Extensions\SidecarCentennialApp\windows.protocol
    //    AnotherProperty    String    CheeseCake
    // LOCAL\Extensions\SidecarCentennialApp\windows.appExecutionAlias
    // LOCAL\Extensions\SidecarCentennialApp\windows.comServer

    // 2. Verify that the initialized marker is present
    auto localSettingsContainer = appData.LocalSettings();
    auto propertySet = localSettingsContainer.Values();
    const winrt::hstring initializedValueNameString = L"Initialized";
    auto settingsCollection{ propertySet.as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable>>() };
    VERIFY_IS_TRUE(settingsCollection.HasKey(initializedValueNameString), L"Initialized marker should be present");

    // 3. Verify that the default CLSID has been extracted from the manifest.
    VERIFY_SUCCEEDED(CheckStringValueByNameInCollection(L"LifetimeManagerClsid", settingsCollection,
        DEFAULT_LIFETIME_MANAGER_CLSID));

    // 4. Verify that the PackageRoot has been populated.
    VERIFY_SUCCEEDED(CheckStringValueByNameInCollection(L"PackageRoot", settingsCollection, PackageRootPath));

    // Reach down to the SidecarCentennialApp container.
    auto subcontainers = localSettingsContainer.Containers();

    auto extensionsContainer = subcontainers.Lookup(winrt::hstring(L"Extensions"));
    VERIFY_IS_NOT_NULL(extensionsContainer);

    subcontainers = extensionsContainer.Containers();
    VERIFY_IS_NOT_NULL(subcontainers);

    auto centennialAppContainer = subcontainers.Lookup(winrt::hstring(L"SidecarCentennialApp"));
    VERIFY_IS_NOT_NULL(centennialAppContainer);

    if (checkExecutablePath)
    {
        // 5. Verify that the ExecutablePath has been correctly populated (via the "--Init" flag during activation by AUMID).
        propertySet = centennialAppContainer.Values();
        settingsCollection = propertySet.as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring,
            winrt::Windows::Foundation::IInspectable>>();
        VERIFY_SUCCEEDED(CheckStringValueByNameInCollection(L"ExecutablePath", settingsCollection, PipLauncherPath));
    }

    // 6. Verify that the Property element parsed out from the PIP manifest exists in Local Settings.
    subcontainers = centennialAppContainer.Containers();
    VERIFY_IS_NOT_NULL(subcontainers);

    auto protocolContainer = subcontainers.Lookup(winrt::hstring(L"windows.protocol"));
    VERIFY_IS_NOT_NULL(protocolContainer);

    propertySet = protocolContainer.Values();
    VERIFY_IS_NOT_NULL(propertySet);

    settingsCollection = propertySet.as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring,
        winrt::Windows::Foundation::IInspectable>>();

    VERIFY_SUCCEEDED(CheckStringValueByNameInCollection(L"AnotherProperty", settingsCollection, L"CheeseCake"));
    return S_OK;
}

HRESULT PIPActivationTest::CheckStringValueByNameInCollection(
    _In_ PCWSTR valueName,
    _In_ winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> settingsCollection,
    _In_ PCWSTR expectedName)
{
    const winrt::hstring valueNameString = valueName;
    winrt::Windows::Foundation::IPropertyValue stringPropertyValue = settingsCollection.Lookup(
        valueNameString).as<winrt::Windows::Foundation::IPropertyValue>();
    VERIFY_IS_NOT_NULL(stringPropertyValue);

    auto propertyType = stringPropertyValue.Type();
    VERIFY_IS_TRUE(propertyType == winrt::Windows::Foundation::PropertyType::String);

    const winrt::hstring localValue = stringPropertyValue.GetString();
    Log::Comment(WEX::Common::String().Format(L"Expected %ls, got %ls", expectedName, localValue.c_str()));
    VERIFY_IS_TRUE(CompareStringOrdinal(localValue.c_str(), -1, expectedName, -1, TRUE) == CSTR_EQUAL);
    return S_OK;
}

}
