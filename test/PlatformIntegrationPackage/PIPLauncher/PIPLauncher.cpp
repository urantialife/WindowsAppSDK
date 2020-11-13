// Copyright (C) Microsoft. All rights reserved.

#include <pch.h>

// TODO: enable telemetry
#define ENABLE_TELEMETRY 0

#include <stdio.h>
#include <stdlib.h>
#include <Shobjidl.h>

#include <wil/result_macros.h>
#include <wil/com.h>

#include <LifetimeManagerClsid.h>
#include <IDynamicDependencyLifetimeManager.h>

#include <guiddef.h>
#include <TraceLoggingProvider.h>
#if ENABLE_TELEMETRY
#ifndef DOWNLEVEL_PRIOR_TO_WIN8
#include <telemetry/MicrosoftTelemetry.h>
#include <telemetry/MicrosoftTelemetryPrivacy.h>
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME "Microsoft.Windows.ApplicationModel.pip"
#ifdef __cplusplus
}
#endif

using namespace Microsoft::WRL;
using namespace Windows::Foundation;

TRACELOGGING_DECLARE_PROVIDER(g_pipTraceLoggingProvider);

#if ENABLE_TELEMETRY
// GUID for PIP trace logging provider: {65c056f3-458d-4df1-9a03-3303733f671f}
TRACELOGGING_DEFINE_PROVIDER(
    g_pipTraceLoggingProvider,
    TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME,
    (0x65c056f3, 0x458d, 0x4df1, 0x9a, 0x03, 0x33, 0x03, 0x73, 0x3f, 0x67, 0x1f),
    TraceLoggingOptionMicrosoftTelemetry());

class PipProvider : public wil::TraceLoggingProvider
{
    //65c056f3-458d-4df1-9a03-3303733f671f
    IMPLEMENT_TRACELOGGING_CLASS(PipProvider, TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME,
        (0x65c056f3, 0x458d, 0x4df1, 0x9a, 0x03, 0x33, 0x03, 0x73, 0x3f, 0x67, 0x1f));
};
#endif

int ShowUsage()
{
    wprintf(L"Usage: PIPLauncher <AUMID> [ args...]\n");
    wprintf(L"Or:    PIPLauncher --Invoke [ CLSID ]\n");
    wprintf(L"Or:    PIPLauncher --Help\n");
    wprintf(L"Flags:\n");
    wprintf(L"--Help\t\t\t  Show this help page\n");
    wprintf(L"--Init \"<ExecutablePath>\" Trigger initializtion of executable path for win32 app to <ExecutablePath>\n");
    wprintf(L"--Invoke [CLSID]\t  Activate the OOP package COM server hosted by the PIP, using the prescribed CLSID if present\n");
    wprintf(L"Examples:\n");
    wprintf(L"\t PIPLauncher SidecarMainApp_8wekyb3d8bbwe!SidecarMainApp\n");
    wprintf(L"\t PIPLauncher SidecarMainApp_8wekyb3d8bbwe!SidecarMainApp --Init \"C:\\DirA\\App B\\bin\"\n");
    wprintf(L"\t PIPLauncher --Invoke\n");
    wprintf(L"\t PIPLauncher --Invoke {65c056f3-458d-4df1-9a03-3303733f671f}\n");
    return 1;
}

int LaunchApp(_In_ PCWSTR AUMID, _In_ PCWSTR args)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        try
        {
            ComPtr<IApplicationActivationManager> AppActivationMgr;
            hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&AppActivationMgr));
            if (SUCCEEDED(hr))
            {
                DWORD pid = 0;
                hr = AppActivationMgr->ActivateApplication(AUMID, args, AO_NONE, &pid);
                if (SUCCEEDED(hr))
                {
                    wprintf(L"INFO: Done %ls %ls %u.\n", AUMID, args, pid);
                }
                else
                {
                    wprintf(L"ERROR: LaunchApp %s: Failed to Activate App. hr = 0x%08lx \n", AUMID, hr);
                }
            }
            else
            {
                wprintf(L"ERROR: LaunchApp %s: Failed to create Application Activation Manager. hr = 0x%08lx \n", AUMID, hr);
            }
        }
        catch (...)
        {
            const HRESULT hrException = wil::ResultFromCaughtException();
            wprintf(L"ERROR: exception 0x%0x\n", hrException);
        }
        
        CoUninitialize();
    }
    else
    {
        wprintf(L"ERROR %s: CoInitializeEx 0x%0x.\n", AUMID, hr);
    }

    return SUCCEEDED(hr) ? 0 : 1;
}

bool containsSpace(_In_ PCWSTR inputString)
{
    for (UINT i = 0; i < wcslen(inputString); i++)
    {
        if (inputString[i] == L' ')
        {
            return true;
        }
    }
    return false;
}

int InvokeClass(_In_opt_ PCWSTR clsidString)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: CoInitializeEx FAILED, 0x%0x\n", hr);
        return 1;
    }

    PCWSTR localClsidString = (clsidString != nullptr) ? clsidString : Microsoft::Reunion::Sidecar::DefaultLifetimeManager_clsid;
    CLSID appDynamicDependencyLifetimeManager;
    hr = CLSIDFromString(localClsidString, &appDynamicDependencyLifetimeManager);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: CLSIDFromString %ls FAILED, 0x%0x\n", localClsidString, hr);
        return 1;
    }

    wprintf(L"INFO: CoCreateInstance %ls\n", localClsidString);

    try
    {
        wil::com_ptr_t<IDynamicDependencyLifetimeManager> lifetimeManager(wil::CoCreateInstance<IDynamicDependencyLifetimeManager>(
            appDynamicDependencyLifetimeManager, CLSCTX_LOCAL_SERVER));

        if (lifetimeManager == nullptr)
        {
            wprintf(L"ERROR: lifetimeManager is null\n");
            return 1;
        }

        hr = lifetimeManager->Initialize();
        if (FAILED(hr))
        {
            wprintf(L"ERROR: lifetimeManager->Initialize FAILED, 0x%0x\n", hr);
            return 1;
        }
        wprintf(L"INFO: lifetimeManager->Initialize Done.\n");

        wil::unique_cotaskmem_string packageFullName;
        hr = lifetimeManager->GetPackageFullName(&packageFullName);
        if (FAILED(hr))
        {
            wprintf(L"ERROR: lifetimeManager->GetPackageFullName FAILED, 0x%0x\n", hr);
            return 1;
        }

        wprintf(L"INFO: GetPackageFullName %s, sleeping 2 secs...\n", packageFullName.get());
        Sleep(2000);

        hr = lifetimeManager->Shutdown();
        if (FAILED(hr))
        {
            wprintf(L"ERROR: lifetimeManager->Shutdown FAILED, 0x%0x\n", hr);
            return 1;
        }
    }
    catch (...)
    {
        const HRESULT hr = wil::ResultFromCaughtException();
        wprintf(L"ERROR: CoCreateInstance 0x%0x\n", hr);
#if ENABLE_TELEMETRY
        TraceLoggingWrite(g_pipTraceLoggingProvider, "lifetimeManagerActivationFailed",
            TraceLoggingHResult(hr),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_TELEMETRY),
            TraceLoggingLevel(WINEVENT_LEVEL_VERBOSE),
            TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
#endif
        return 1;
    }

#if ENABLE_TELEMETRY
    TraceLoggingWrite(g_pipTraceLoggingProvider, "lifetimeManagerActivated",
        TraceLoggingKeyword(MICROSOFT_KEYWORD_TELEMETRY),
        TraceLoggingLevel(WINEVENT_LEVEL_VERBOSE),
        TelemetryPrivacyDataTag(PDT_ProductAndServicePerformance));
#endif
    wprintf(L"INFO: lifetimeManager->Shutdown Done.\n");
    return 0;
}

// Argument processing, and then hand over to the main loop.
int __cdecl wmain(int argc, _In_reads_(argc) WCHAR * argv[])
{
#if ENABLE_TELEMETRY
    wil::SetResultTelemetryFallback(PipProvider::FallbackTelemetryCallback);

    TraceLoggingRegister(g_pipTraceLoggingProvider);
#endif

    wprintf(L"== PIPLauncher ==\n");
    if (argc < 2)
    {
        wprintf(L"ERROR: Too few args, %u\n\n", argc);
        return ShowUsage();
    }

    const PCWSTR helpFlag = L"--Help";
    if (CompareStringOrdinal(argv[1], -1, helpFlag, -1, TRUE) == CSTR_EQUAL)
    {
        return ShowUsage();
    }

    const PCWSTR invokeFlag = L"--Invoke";
    if (CompareStringOrdinal(argv[1], -1, invokeFlag, -1, TRUE) == CSTR_EQUAL)
    {
        PCWSTR clsidString = (argc > 2) ? argv[2] : nullptr;
        const int InvokeResult = InvokeClass(clsidString);
#if ENABLE_TELEMETRY
        TraceLoggingUnregister(g_pipTraceLoggingProvider);
#endif
        return InvokeResult;
    }

    PCWSTR aumid = argv[1];
    WCHAR args[MAX_PATH * 2] = { L"\0" };
    for (int i = 2; i < argc; i++)
    {
        wprintf(L"Arg[%u]=%s\n", i, argv[i]);
        if (i > 2)
        {
            StringCchCat(args, ARRAYSIZE(args), L" ");
        }

        // If argv[i] contains space, then surround it with double quotes.
        if (containsSpace(argv[i]))
        {
            StringCchCat(args, ARRAYSIZE(args), L"\"");
        }
        StringCchCat(args, ARRAYSIZE(args), argv[i]);
        if (containsSpace(argv[i]))
        {
            StringCchCat(args, ARRAYSIZE(args), L"\"");
        }
    }

    const int launchResult = LaunchApp(aumid, args);
#if ENABLE_TELEMETRY
    TraceLoggingUnregister(g_pipTraceLoggingProvider);
#endif
    return launchResult;
}
