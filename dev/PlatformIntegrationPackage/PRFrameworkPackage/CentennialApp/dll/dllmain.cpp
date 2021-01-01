// Copyright (c) Microsoft Corporation.  All rights reserved.

#include <windows.h>
#include <wrl\module.h>

// TODO: enable telemetry
#define ENABLE_TELEMETRY 0

#include <PipTraceLogging.h>

namespace Microsoft
{
    namespace Windows
    {
        namespace ApplicationModel
        {
            WrlCreatorMapIncludePragma(CentennialBackendImpl);
        }
    }
}

/// <summary>
/// DLL Entry point, called by the system when the DLL is loaded/unloaded.
/// </summary>
/// <param name="instance">DLL instance.</param>
/// <param name="reason">Reason for entry.</param>
/// <param name="pReserved">Reserved.</param>
BOOL WINAPI DllMain(HANDLE instance, DWORD reason, _In_opt_ PVOID pReserved)
{
    UNREFERENCED_PARAMETER(pReserved);

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls((HMODULE)instance);
#if ENABLE_TELEMETRY
        wil::SetResultTelemetryFallback(PipProvider::FallbackTelemetryCallback);
        TraceLoggingRegister(g_pipTraceLoggingProvider);
#endif
        break;

    case DLL_PROCESS_DETACH:
#if ENABLE_TELEMETRY
        TraceLoggingUnregister(g_pipTraceLoggingProvider);
#endif
        break;
    }

    return TRUE;
}

// <summary>
// RPC runtime will call this routine to allocate memory.
// </summary>
// <param name="allocBytes"> Size of the allocation in bytes </param>
// <returns> A valid pointer on success, nullptr on failure. </returns>
_Must_inspect_result_
_Ret_maybenull_ _Post_writable_byte_size_(size)
extern "C" void* __RPC_USER  MIDL_user_allocate(_In_  size_t allocBytes)
{
    // This function uses LocalAlloc so that any data that is LocalAlloc'd is
    // automatically able to be marshalled over RPC (rather than needing to
    // duplicate it again with MIDL_user_allocate).
    return LocalAlloc(LPTR, allocBytes);
}

// <summary>
// RPC runtime will call this routine to free memory.
// </summary>
// <param name="pBuffer"> Pointer to the buffer that will be freed. </param>
// <returns> None. </returns>
extern "C" void __RPC_USER  MIDL_user_free(_Pre_maybenull_ _Post_invalid_ void* pBuffer)
{
    LocalFree(pBuffer);
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllGetActivationFactory(
    __in HSTRING activatibleClassId,
    __deref_out IActivationFactory * *factory)
{
    return Microsoft::WRL::Module<Microsoft::WRL::InProc>::GetModule().GetActivationFactory(activatibleClassId, factory);
}
