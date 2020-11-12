

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0623 */
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
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

#ifndef __idynamicdependencylifetimemanager_h__
#define __idynamicdependencylifetimemanager_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if _CONTROL_FLOW_GUARD_XFG
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IDynamicDependencyLifetimeManager_FWD_DEFINED__
#define __IDynamicDependencyLifetimeManager_FWD_DEFINED__
typedef interface IDynamicDependencyLifetimeManager IDynamicDependencyLifetimeManager;

#endif 	/* __IDynamicDependencyLifetimeManager_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IDynamicDependencyLifetimeManager_INTERFACE_DEFINED__
#define __IDynamicDependencyLifetimeManager_INTERFACE_DEFINED__

/* interface IDynamicDependencyLifetimeManager */
/* [unique][uuid][helpstring][object] */ 


EXTERN_C const IID IID_IDynamicDependencyLifetimeManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("06F1BAD0-DD14-11d0-AB8F-0000C0148FDB")
    IDynamicDependencyLifetimeManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPackageFullName( 
            /* [retval][out] */ __RPC__deref_out_opt LPWSTR *packageFullName) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IDynamicDependencyLifetimeManagerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IDynamicDependencyLifetimeManager * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IDynamicDependencyLifetimeManager * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IDynamicDependencyLifetimeManager * This);
        
        DECLSPEC_XFGVIRT(IDynamicDependencyLifetimeManager, Initialize)
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            __RPC__in IDynamicDependencyLifetimeManager * This);
        
        DECLSPEC_XFGVIRT(IDynamicDependencyLifetimeManager, Shutdown)
        HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            __RPC__in IDynamicDependencyLifetimeManager * This);
        
        DECLSPEC_XFGVIRT(IDynamicDependencyLifetimeManager, GetPackageFullName)
        HRESULT ( STDMETHODCALLTYPE *GetPackageFullName )( 
            __RPC__in IDynamicDependencyLifetimeManager * This,
            /* [retval][out] */ __RPC__deref_out_opt LPWSTR *packageFullName);
        
        END_INTERFACE
    } IDynamicDependencyLifetimeManagerVtbl;

    interface IDynamicDependencyLifetimeManager
    {
        CONST_VTBL struct IDynamicDependencyLifetimeManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDynamicDependencyLifetimeManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDynamicDependencyLifetimeManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDynamicDependencyLifetimeManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDynamicDependencyLifetimeManager_Initialize(This)	\
    ( (This)->lpVtbl -> Initialize(This) ) 

#define IDynamicDependencyLifetimeManager_Shutdown(This)	\
    ( (This)->lpVtbl -> Shutdown(This) ) 

#define IDynamicDependencyLifetimeManager_GetPackageFullName(This,packageFullName)	\
    ( (This)->lpVtbl -> GetPackageFullName(This,packageFullName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDynamicDependencyLifetimeManager_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


