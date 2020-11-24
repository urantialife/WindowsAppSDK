//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.

// alam
// [ ] Adapter -> Backend
#if 0
#include <pch.h>
#else
#include <string>
#include <vector>
#if 1
#include <wil\cppwinrt.h>
#endif
#include <wil\result.h>
#include <wil\resource.h>

// WRL dependencies.
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <wrl\wrappers\corewrappers.h>
#endif

// TODO: enable telemetry
#define ENABLE_TELEMETRY 0

#include <Windows.Foundation.h>
#include <appmodel.h>                    // for PACKAGE_FULL_NAME_MAX_LENGTH

// alam
#if 0
#include <Microsoft.Windows.ApplicationModel.CentennialBackend.h>
#else
#include <Microsoft.Windows.ApplicationModel.CentennialBackend_h.h>
#endif

#include <IDynamicDependencyLifetimeManager_h.h>
#include <PipTraceLogging.h>

// This header file contains a CLSID that needs to be replaced with a newly generated per-app version.
#include <LifetimeManagerClsid.h>

#define MAX_RETRY_COUNT                  12
#define RETRY_PERIOD_IN_MS               1000 // 1 sec

// alam
#if 1
#define LOG_TO_FILE_MSG_AND_HR(msg, hr) \
    (void)LOG_IF_FAILED(Microsoft::Reunion::Sidecar::centennialBackend->LogErrorMsg( \
        Microsoft::WRL::Wrappers::HStringReference(msg).Get(), hr))
#else
#define LOG_TO_FILE_MSG_AND_HR(msg, hr) \
    (void)LOG_IF_FAILED(::centennialBackend->LogErrorMsg( \
        Microsoft::WRL::Wrappers::HStringReference(msg).Get(), hr))
#endif

using namespace std;

// alam
#if 1
static Microsoft::WRL::ComPtr<Microsoft::Windows::ApplicationModel::ICentennialBackend> centennialBackend;
#endif

namespace Microsoft::Reunion::Sidecar
{
    // alam
#if 1
    static Microsoft::WRL::ComPtr<Microsoft::Windows::ApplicationModel::ICentennialBackend> centennialBackend;
#endif
    static wil::unique_event endOfTheLine;
    static bool isOopCOMActivation = false;

    // Implement the LifetimeManager as a classic COM Out-of-Proc server, via WRL
    // See https://docs.microsoft.com/en-us/cpp/cppcx/wrl/how-to-create-a-classic-com-component-using-wrl?redirectedfrom=MSDN&view=vs-2019 for more details

    struct __declspec(uuid(DEFAULT_LIFETIME_MANAGER_CLSID_A)) MyLifetimeManagerImpl WrlFinal :
    Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IDynamicDependencyLifetimeManager>
    {
        HRESULT __stdcall Initialize()
        {
            LOG_TO_FILE_MSG_AND_HR(L"MyLifetimeManagerImpl::Initialize", S_OK);
            isOopCOMActivation = true;
            return S_OK;
        }

        HRESULT __stdcall Shutdown()
        {
            LOG_TO_FILE_MSG_AND_HR(L"MyLifetimeManagerImpl::Shutdown", S_OK);
            return S_OK;
        }

        HRESULT __stdcall GetPackageFullName(/*[out, retval]*/ PWSTR * packageFullName)
        {
            *packageFullName = nullptr;
            isOopCOMActivation = true;

            WCHAR fullName[PACKAGE_FULL_NAME_MAX_LENGTH + 1]{};
            UINT32 fullNameLength = ARRAYSIZE(fullName);
            const HRESULT hrGet = GetCurrentPackageFullName(&fullNameLength, fullName);
            LOG_TO_FILE_MSG_AND_HR(L"MyLifetimeManagerImpl::GetCurrentPackageFullName", hrGet);
            RETURN_IF_FAILED_MSG(hrGet, "GetCurrentPackageFullName %u", fullNameLength);

            auto fullNameCoTaskMem = wil::make_cotaskmem_string_nothrow(fullName);
            RETURN_IF_NULL_ALLOC_MSG(fullNameCoTaskMem, "make_cotaskmem_string_nothrow %ws", fullName);

            *packageFullName = fullNameCoTaskMem.release();
            return S_OK;
        }
    };

    CoCreatableClass(MyLifetimeManagerImpl);

    void EndOfTheLine()
    {
        LOG_TO_FILE_MSG_AND_HR(L"EndOfTheLine", S_OK);
        endOfTheLine.SetEvent();
    }
}

// Application entry point
int __cdecl wmain()
{
#if ENABLE_TELEMETRY
    wil::SetResultTelemetryFallback(PipProvider::FallbackTelemetryCallback);
    TraceLoggingRegister(g_pipTraceLoggingProvider);
#endif

    winrt::init_apartment();
    {
        // Initialize the backend DLL in the framework package
        THROW_IF_FAILED_MSG(Windows::Foundation::ActivateInstance(
            Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Microsoft_Windows_ApplicationModel_CentennialBackend).Get(),
            &Microsoft::Reunion::Sidecar::centennialBackend), "ActivateInstance %ws",
            RuntimeClass_Microsoft_Windows_ApplicationModel_CentennialBackend);

        THROW_IF_NULL_ALLOC_MSG(Microsoft::Reunion::Sidecar::centennialBackend, "ActivateInstance %ws",
            RuntimeClass_Microsoft_Windows_ApplicationModel_CentennialBackend);

        // We minimize the work we do here in this exe. Log file support is in the backend DLL. So for the few things we do here,
        // sent it back to the backend for logging.
        (void)LOG_IF_FAILED_MSG(Microsoft::Reunion::Sidecar::centennialBackend->Initialize(), "wmain->Initialize");

        wil::unique_event localEndOfTheLine(::CreateEventW(nullptr, TRUE, FALSE, nullptr));
        const HRESULT hrCreate = (localEndOfTheLine != nullptr) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
        LOG_TO_FILE_MSG_AND_HR(L"wmain::CreateEventW", hrCreate);
        THROW_IF_FAILED_MSG(hrCreate, "wmain::CreateEventW");

        Microsoft::Reunion::Sidecar::endOfTheLine = std::move(localEndOfTheLine);

        // Try to track termination of the caller. Need this in case this is an OOP COM activation.
        auto& module = Microsoft::WRL::Module<Microsoft::WRL::OutOfProc>::Create(Microsoft::Reunion::Sidecar::EndOfTheLine);
        const HRESULT hrReg = module.RegisterObjects();
        LOG_TO_FILE_MSG_AND_HR(L"wmain::RegisterObjects", hrReg);
        THROW_IF_FAILED_MSG(hrReg, "wmain::RegisterObjects");

        BOOL activatedByAumid = FALSE; 
        if (!Microsoft::Reunion::Sidecar::isOopCOMActivation)
        {
            // If no clear sign that we were activated via OOP COM activation, then try to see if we were activated by AUMID.
            const HRESULT hrLaunch = Microsoft::Reunion::Sidecar::centennialBackend->OnLaunch(&activatedByAumid);
            LOG_TO_FILE_MSG_AND_HR(L"wmain::OnLaunch", hrLaunch);
            THROW_IF_FAILED_MSG(hrLaunch, "wmain::OnLaunch");
        }

        // Diagnostic info. Abusing the HR parameter.
        LOG_TO_FILE_MSG_AND_HR(L"wmain::activatedByAumid", static_cast<UINT>(activatedByAumid));
        LOG_TO_FILE_MSG_AND_HR(L"wmain::isOopCOMActivation", static_cast<UINT>(Microsoft::Reunion::Sidecar::isOopCOMActivation));

        if (activatedByAumid == FALSE)
        {
            // We know it was not an AUMID activation. It could be either an executable alias activation or an OOP COM 
            // activation. The latter should show a sign if we wait a bit.
            UINT retryCount = MAX_RETRY_COUNT;
            while ((activatedByAumid == FALSE) && !Microsoft::Reunion::Sidecar::isOopCOMActivation && (retryCount > 0))
            {
                // If no clear sign that we were activated via OOP COM activation, give it time for the .
                Sleep(RETRY_PERIOD_IN_MS);
                retryCount -= 1;
            }
            LOG_TO_FILE_MSG_AND_HR(L"wmain::retryCount", retryCount);
        }

        // Block on caller termination if this is an OOP COM activation.
        if (Microsoft::Reunion::Sidecar::isOopCOMActivation)
        {
            LOG_TO_FILE_MSG_AND_HR(L"wmain::Microsoft::Reunion::Sidecar::endOfTheLine.wait", S_OK);
            Microsoft::Reunion::Sidecar::endOfTheLine.wait();
        }

        const HRESULT hrUnreg = module.UnregisterObjects();
        LOG_TO_FILE_MSG_AND_HR(L"wmain::UnregisterObjects", hrUnreg);
        THROW_IF_FAILED_MSG(hrUnreg, "wmain::UnregisterObjects");

        const HRESULT hrUninit = Microsoft::Reunion::Sidecar::centennialBackend->Uninitialize();
        LOG_TO_FILE_MSG_AND_HR(L"wmain::Uninitialize", hrUninit);
        (void)LOG_IF_FAILED_MSG(hrUninit, "wmain::Uninitialize");
        module.Terminate();
    }
    winrt::uninit_apartment();
#if ENABLE_TELEMETRY
    TraceLoggingUnregister(g_pipTraceLoggingProvider);
#endif
    return 0;
}
