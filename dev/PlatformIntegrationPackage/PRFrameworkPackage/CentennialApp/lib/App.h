//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

namespace Microsoft::Reunion::Sidecar
{
    HRESULT CentenialInitialize();
    HRESULT CentenialUnnitialize();

    HRESULT OnCentenialLaunch(
        _Out_ BOOL* activatedByAumid);

    HRESULT LogCentenialErrorMsg(
        _In_ PCWSTR msg,
        _In_ HRESULT hr);
}
