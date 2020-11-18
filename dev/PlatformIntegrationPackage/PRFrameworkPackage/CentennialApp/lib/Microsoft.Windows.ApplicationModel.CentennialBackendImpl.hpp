/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#pragma once

#include <Microsoft.Windows.ApplicationModel.CentennialBackend.h>

#include <wrl/implements.h>
#include <wrl/module.h>
#include <wrl/event.h>
#include <wrl/wrappers/corewrappers.h>
// #include <wrl/wrappers.h>

namespace Microsoft
{
    namespace Windows
    {
        namespace ApplicationModel
        {
            class CentennialBackendImpl : public Microsoft::WRL::RuntimeClass<ICentennialBackend, Microsoft::WRL::FtmBase>
            {
                InspectableClass(RuntimeClass_Microsoft_Windows_ApplicationModel_CentennialBackend, BaseTrust);

            public: // constructor/destructor declarations

                CentennialBackendImpl(){};
                ~CentennialBackendImpl(){};

            public: // activation factory integration method declarations

                IFACEMETHOD(Initialize)();

                // Microsoft.Windows.ApplicationModel.CentennialBackend.Uninitialize
                IFACEMETHOD(Uninitialize)() override;

                // Microsoft.Windows.ApplicationModel.CentennialBackend.OnLaunch
                IFACEMETHOD(OnLaunch)(__RPC__out BOOL* activatedByAumid) override;

                // Microsoft.Windows.ApplicationModel.CentennialBackend.LogErrorMsg
                IFACEMETHOD(LogErrorMsg)(_In_ HSTRING  msg, _In_ HRESULT hr) override;
            };
        }
    }
}
