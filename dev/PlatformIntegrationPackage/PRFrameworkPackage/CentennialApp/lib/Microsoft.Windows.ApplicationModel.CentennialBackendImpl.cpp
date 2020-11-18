/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include <pch.h>

#include <App.h>

#include <Microsoft.Windows.ApplicationModel.CentennialBackendImpl.hpp>

using namespace Microsoft::WRL;
// using namespace Windows::Internal;

namespace Microsoft
{
    namespace Windows
    {
        namespace ApplicationModel
        {
            ActivatableClass(CentennialBackendImpl);

            // --------------------------------------------------------------------------------------------------------------------------------
            IFACEMETHODIMP
            CentennialBackendImpl::Initialize()
            {
                RETURN_IF_FAILED_MSG(Microsoft::Reunion::Sidecar::CentenialInitialize(), "CentenialInitialize");
                return S_OK;
            }

            IFACEMETHODIMP
            CentennialBackendImpl::OnLaunch(__RPC__out BOOL* activatedByAumid)
            {
                RETURN_IF_FAILED_MSG(Microsoft::Reunion::Sidecar::OnCentenialLaunch(activatedByAumid),
                    "OnCentenialLaunch");
                return S_OK;
            }

            IFACEMETHODIMP
            CentennialBackendImpl::Uninitialize()
            {
                RETURN_IF_FAILED_MSG(Microsoft::Reunion::Sidecar::CentenialUnnitialize(), "CentenialUnnitialize");
                return S_OK;
            }

            IFACEMETHODIMP
            CentennialBackendImpl::LogErrorMsg(_In_ HSTRING  msg, _In_ HRESULT hr)
            {
                RETURN_IF_FAILED_MSG(Microsoft::Reunion::Sidecar::LogCentenialErrorMsg(WindowsGetStringRawBuffer(msg, nullptr), hr),
                    "LogCentenialErrorMsg %ws 0x%0x", WindowsGetStringRawBuffer(msg, nullptr), hr);
                return S_OK;
            }
        }
    }
}
