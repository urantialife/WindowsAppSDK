/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#pragma once

#include <Microsoft.Windows.ApplicationModel.CentennialBackend_h.h>

#include <wrl/implements.h>
#include <wrl/module.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>

namespace Microsoft
{
    namespace Windows
    {
        namespace ApplicationModel
        {
            class CentennialBackendImpl : public Microsoft::WRL::RuntimeClass<Microsoft::Windows::ApplicationModel::ICentennialBackend, Microsoft::WRL::FtmBase>
            {
                InspectableClass(RuntimeClass_Microsoft_Windows_ApplicationModel_CentennialBackend, BaseTrust);

            public: // constructor/destructor declarations

                CentennialBackendImpl(){};
                ~CentennialBackendImpl(){};

            public: // activation factory integration method declarations

                IFACEMETHOD(Initialize)();

                // Microsoft.Windows.ApplicationModel.CentennialBackend.Uninitialize
                IFACEMETHOD(Uninitialize)();

                // Microsoft.Windows.ApplicationModel.CentennialBackend.OnLaunch
                IFACEMETHOD(OnLaunch)(__RPC__out BOOL* activatedByAumid);

                // Microsoft.Windows.ApplicationModel.CentennialBackend.LogErrorMsg
                IFACEMETHOD(LogErrorMsg)(_In_ HSTRING  msg, _In_ HRESULT hr);
            };
        }
    }
}
