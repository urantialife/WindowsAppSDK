

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 19:14:07 2038
 */
/* Compiler settings for C:\Users\alexlam\AppData\Local\Temp\Microsoft.Windows.ApplicationModel.CentennialBackend.idl-9fd4e2df:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __Microsoft2EWindows2EApplicationModel2ECentennialBackend_h_p_h__
#define __Microsoft2EWindows2EApplicationModel2ECentennialBackend_h_p_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(__cplusplus)
#if defined(__MIDL_USE_C_ENUM)
#define MIDL_ENUM enum
#else
#define MIDL_ENUM enum class
#endif
#endif


/* Forward Declarations */ 

#ifndef ____x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_FWD_DEFINED__
#define ____x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_FWD_DEFINED__
typedef interface __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend;

#endif 	/* ____x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Inspectable.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_Microsoft2EWindows2EApplicationModel2ECentennialBackend_0000_0000 */
/* [local] */ 





extern RPC_IF_HANDLE __MIDL_itf_Microsoft2EWindows2EApplicationModel2ECentennialBackend_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_Microsoft2EWindows2EApplicationModel2ECentennialBackend_0000_0000_v0_0_s_ifspec;

#ifndef ____x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_INTERFACE_DEFINED__
#define ____x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_INTERFACE_DEFINED__

/* interface __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend */
/* [uuid][object] */ 


EXTERN_C const IID IID___x_Microsoft_CWindows_CApplicationModel_CICentennialBackend;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("46f8b72a-f777-4d65-81f0-7efa644ff6a7")
    __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend : public IInspectable
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Uninitialize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnLaunch( 
            /* [out] */ BOOL *activatedByAumid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogErrorMsg( 
            /* [in] */ HSTRING msg,
            /* [in] */ HRESULT hr) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct __x_Microsoft_CWindows_CApplicationModel_CICentennialBackendVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This,
            /* [out] */ ULONG *iidCount,
            /* [size_is][size_is][out] */ IID **iids);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This,
            /* [out] */ HSTRING *className);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This,
            /* [out] */ TrustLevel *trustLevel);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This);
        
        HRESULT ( STDMETHODCALLTYPE *Uninitialize )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnLaunch )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This,
            /* [out] */ BOOL *activatedByAumid);
        
        HRESULT ( STDMETHODCALLTYPE *LogErrorMsg )( 
            __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend * This,
            /* [in] */ HSTRING msg,
            /* [in] */ HRESULT hr);
        
        END_INTERFACE
    } __x_Microsoft_CWindows_CApplicationModel_CICentennialBackendVtbl;

    interface __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend
    {
        CONST_VTBL struct __x_Microsoft_CWindows_CApplicationModel_CICentennialBackendVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_GetIids(This,iidCount,iids)	\
    ( (This)->lpVtbl -> GetIids(This,iidCount,iids) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_GetRuntimeClassName(This,className)	\
    ( (This)->lpVtbl -> GetRuntimeClassName(This,className) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_GetTrustLevel(This,trustLevel)	\
    ( (This)->lpVtbl -> GetTrustLevel(This,trustLevel) ) 


#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_Initialize(This)	\
    ( (This)->lpVtbl -> Initialize(This) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_Uninitialize(This)	\
    ( (This)->lpVtbl -> Uninitialize(This) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_OnLaunch(This,activatedByAumid)	\
    ( (This)->lpVtbl -> OnLaunch(This,activatedByAumid) ) 

#define __x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_LogErrorMsg(This,msg,hr)	\
    ( (This)->lpVtbl -> LogErrorMsg(This,msg,hr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_Microsoft_CWindows_CApplicationModel_CICentennialBackend_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HSTRING_UserSize(     unsigned long *, unsigned long            , HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserMarshal(  unsigned long *, unsigned char *, HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserUnmarshal(unsigned long *, unsigned char *, HSTRING * ); 
void                      __RPC_USER  HSTRING_UserFree(     unsigned long *, HSTRING * ); 

unsigned long             __RPC_USER  HSTRING_UserSize64(     unsigned long *, unsigned long            , HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserMarshal64(  unsigned long *, unsigned char *, HSTRING * ); 
unsigned char * __RPC_USER  HSTRING_UserUnmarshal64(unsigned long *, unsigned char *, HSTRING * ); 
void                      __RPC_USER  HSTRING_UserFree64(     unsigned long *, HSTRING * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


