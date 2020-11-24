//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.

#include <pch.h>

#include <wil/result_macros.h>

#include <wrl.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <wrl/module.h>
#if 0
#include <Windows.Foundation.h>
#endif
#include <Windows.h>
#include <winrt/Windows.Storage.h>

#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <App.h>

#include <Helpers.h>
#include <BasicLogFile.h>

// using namespace winrt;

#define LOG_AND_RETURN_IF_FAILED(op, msg) \
{ \
    const HRESULT localHr = op; \
    if (FAILED(localHr)) \
    { \
        LOG_MESSAGE(L"%s, 0x%0x", msg, localHr); \
        RETURN_HR(localHr); \
    } \
}

// CoreApplication does not work for Centennial
namespace Microsoft::Reunion::Sidecar
{
    static winrt::Windows::Storage::IApplicationData applicationData{};
    static const PCWSTR CENTENNIAL_LOG_MARKER = L"Centennial";
    static wil::unique_rpc_wstr tempGuidRpcString;

    // TODO: Consider using CoInitializeSecurity() to lock down access to the OOP COM server.
    HRESULT CentenialLocalEntry(_Out_ BOOL* activatedByAumid /* , _Out_ long* maxNumTrials */)
    {
        LOG_AND_RETURN_IF_FAILED(Microsoft::Reunion::Sidecar::EnsureAppDataPopulated(/* localMaxNumTrials */), L"EnsureAppDataPopulated.");

        // The following returns an error if this app was activated via execution alias
        auto eventArgs = winrt::Windows::ApplicationModel::AppInstance::GetActivatedEventArgs();
        if (eventArgs != nullptr)
        {
            *activatedByAumid = TRUE;
            auto activatedKind = eventArgs.Kind();
            switch (activatedKind)
            {
            case winrt::Windows::ApplicationModel::Activation::ActivationKind::Launch:
            {
                auto launchArgs = eventArgs.try_as<winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs>();
                LOG_AND_RETURN_IF_FAILED(ProcessInitializationFlag(launchArgs.Arguments().c_str()), L"ProcessInitializationFlag.");
            }
            break;

            default:
                LOG_MESSAGE(L"Unsupported ActivatedKind %u.", static_cast<UINT>(activatedKind));
                break;
            }
        }
        else
        {
            *activatedByAumid = FALSE;
            LOG_MESSAGE(L"OnCentenialLaunch done, this launch was not via AUMID?");
        }
        return S_OK;
    }

    HRESULT OnCentenialLaunch(_Out_ BOOL* activatedByAumid)
    {
        *activatedByAumid = TRUE;
        LOG_AND_RETURN_IF_FAILED(CentenialLocalEntry(activatedByAumid), L"CentenialLocalEntry.");
        return S_OK;
    }

    HRESULT LogCentenialErrorMsg(_In_ PCWSTR msg, _In_ HRESULT hr)
    {
        LOG_MESSAGE(L"%s:%s, 0x%0x.", FAILED(hr) ? L"ERROR" : L"INFO", msg, hr);
        return S_OK;
    }

    HRESULT CentenialUninitialize()
    {
        LOG_AND_RETURN_IF_FAILED(UninitializeLogFile(), "CentenialUnnitialize::UninitializeLogFile");
        return S_OK;
    }

    HRESULT CentenialInitialize()
    {
        RETURN_IF_FAILED_MSG(EnsureAppDataOpened(), "CentenialInitialize::EnsureAppDataOpened");

        UUID uniqueId;
        RETURN_IF_FAILED_MSG(CoCreateGuid(&uniqueId), "CentenialInitialize::CoCreateGuid");

        if (tempGuidRpcString.get() == nullptr)
        {
            RETURN_HR_IF_MSG(HRESULT_FROM_WIN32(E_UNEXPECTED), UuidToString(&uniqueId, &tempGuidRpcString) != RPC_S_OK,
                "CentenialInitialize::UuidToString");
        }

        WCHAR instanceString[MAX_PATH];
        RETURN_IF_FAILED_MSG(StringCchPrintf(instanceString, ARRAYSIZE(instanceString), L"%ls-%ls", CENTENNIAL_LOG_MARKER,
            tempGuidRpcString.get()), "StringCchPrintf %ws", tempGuidRpcString.get());

        RETURN_IF_FAILED_MSG(EnsureLogFileInitialized(instanceString), "EnsureLogFileInitialized %ws", instanceString);
        return S_OK;
    }
} // namespace Microsoft::Reunion::Sidecar
