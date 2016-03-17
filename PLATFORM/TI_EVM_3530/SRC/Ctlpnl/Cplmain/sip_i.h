//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#ifndef __IIMCallback_FWD_DEFINED__
#define __IIMCallback_FWD_DEFINED__
typedef interface IIMCallback IIMCallback;
#endif	/* __IIMCallback_FWD_DEFINED__ */

#ifndef __IInputMethod_FWD_DEFINED__
#define __IInputMethod_FWD_DEFINED__
typedef interface IInputMethod IInputMethod;
#endif	/* __IInputMethod_FWD_DEFINED__ */

#ifndef __IIMCallback_INTERFACE_DEFINED__
#define __IIMCallback_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IIMCallback
 * at Tue Nov 25 14:31:51 1997
 * using MIDL 3.02.88
 ****************************************/
/* [object][helpstring][uuid] */ 



EXTERN_C const IID IID_IIMCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("42429669-ae04-11d0-a4f8-00aa00a749b9")
    IIMCallback : public IUnknown
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetImInfo( 
            IMINFO __RPC_FAR *pimi) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SendVirtualKey( 
            BYTE bVK,
            DWORD dwFlags) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SendCharEvents( 
            UINT uVK,
            UINT uKeyFlags,
            UINT uChars,
            UINT __RPC_FAR *puShift,
            UINT __RPC_FAR *puChars) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SendString( 
            BSTR ptszStr,
            DWORD dwChars) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IIMCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IIMCallback __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IIMCallback __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IIMCallback __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetImInfo )( 
            IIMCallback __RPC_FAR * This,
            IMINFO __RPC_FAR *pimi);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendVirtualKey )( 
            IIMCallback __RPC_FAR * This,
            BYTE bVK,
            DWORD dwFlags);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendCharEvents )( 
            IIMCallback __RPC_FAR * This,
            UINT uVK,
            UINT uKeyFlags,
            UINT uChars,
            UINT __RPC_FAR *puShift,
            UINT __RPC_FAR *puChars);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendString )( 
            IIMCallback __RPC_FAR * This,
            BSTR ptszStr,
            DWORD dwChars);
        
        END_INTERFACE
    } IIMCallbackVtbl;

    interface IIMCallback
    {
        CONST_VTBL struct IIMCallbackVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIMCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIMCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIMCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIMCallback_SetImInfo(This,pimi)	\
    (This)->lpVtbl -> SetImInfo(This,pimi)

#define IIMCallback_SendVirtualKey(This,bVK,dwFlags)	\
    (This)->lpVtbl -> SendVirtualKey(This,bVK,dwFlags)

#define IIMCallback_SendCharEvents(This,uVK,uKeyFlags,uChars,puShift,puChars)	\
    (This)->lpVtbl -> SendCharEvents(This,uVK,uKeyFlags,uChars,puShift,puChars)

#define IIMCallback_SendString(This,ptszStr,dwChars)	\
    (This)->lpVtbl -> SendString(This,ptszStr,dwChars)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IIMCallback_SetImInfo_Proxy( 
    IIMCallback __RPC_FAR * This,
    IMINFO __RPC_FAR *pimi);


void __RPC_STUB IIMCallback_SetImInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IIMCallback_SendVirtualKey_Proxy( 
    IIMCallback __RPC_FAR * This,
    BYTE bVK,
    DWORD dwFlags);


void __RPC_STUB IIMCallback_SendVirtualKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IIMCallback_SendCharEvents_Proxy( 
    IIMCallback __RPC_FAR * This,
    UINT uVK,
    UINT uKeyFlags,
    UINT uChars,
    UINT __RPC_FAR *puShift,
    UINT __RPC_FAR *puChars);


void __RPC_STUB IIMCallback_SendCharEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IIMCallback_SendString_Proxy( 
    IIMCallback __RPC_FAR * This,
    BSTR ptszStr,
    DWORD dwChars);


void __RPC_STUB IIMCallback_SendString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IIMCallback_INTERFACE_DEFINED__ */

#ifndef __IInputMethod_INTERFACE_DEFINED__
#define __IInputMethod_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IInputMethod
 * at Tue Nov 25 14:31:51 1997
 * using MIDL 3.02.88
 ****************************************/
/* [object][helpstring][uuid] */ 



EXTERN_C const IID IID_IInputMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("42429666-ae04-11d0-a4f8-00aa00a749b9")
    IInputMethod : public IUnknown
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Select( 
            /* [in] */ HWND hwndSip) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Deselect( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Showing( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Hiding( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetInfo( 
            /* [out] */ IMINFO __RPC_FAR *pimi) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ReceiveSipInfo( 
            /* [in] */ SIPINFO __RPC_FAR *psi) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RegisterCallback( 
            /* [in] */ IIMCallback __RPC_FAR *lpIMCallback) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetImData( 
            /* [in] */ DWORD dwSize,
            /* [out] */ LPVOID pvImData) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetImData( 
            /* [in] */ DWORD dwSize,
            /* [in] */ LPVOID pvImData) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE UserOptionsDlg( 
            /* [in] */ HWND hwndParent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IInputMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInputMethod __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInputMethod __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Select )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ HWND hwndSip);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Deselect )( 
            IInputMethod __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Showing )( 
            IInputMethod __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hiding )( 
            IInputMethod __RPC_FAR * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInfo )( 
            IInputMethod __RPC_FAR * This,
            /* [out] */ IMINFO __RPC_FAR *pimi);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReceiveSipInfo )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ SIPINFO __RPC_FAR *psi);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterCallback )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ IIMCallback __RPC_FAR *lpIMCallback);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetImData )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [out] */ LPVOID pvImData);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetImData )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [in] */ LPVOID pvImData);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UserOptionsDlg )( 
            IInputMethod __RPC_FAR * This,
            /* [in] */ HWND hwndParent);
        
        END_INTERFACE
    } IInputMethodVtbl;

    interface IInputMethod
    {
        CONST_VTBL struct IInputMethodVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInputMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInputMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInputMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInputMethod_Select(This,hwndSip)	\
    (This)->lpVtbl -> Select(This,hwndSip)

#define IInputMethod_Deselect(This)	\
    (This)->lpVtbl -> Deselect(This)

#define IInputMethod_Showing(This)	\
    (This)->lpVtbl -> Showing(This)

#define IInputMethod_Hiding(This)	\
    (This)->lpVtbl -> Hiding(This)

#define IInputMethod_GetInfo(This,pimi)	\
    (This)->lpVtbl -> GetInfo(This,pimi)

#define IInputMethod_ReceiveSipInfo(This,psi)	\
    (This)->lpVtbl -> ReceiveSipInfo(This,psi)

#define IInputMethod_RegisterCallback(This,lpIMCallback)	\
    (This)->lpVtbl -> RegisterCallback(This,lpIMCallback)

#define IInputMethod_GetImData(This,dwSize,pvImData)	\
    (This)->lpVtbl -> GetImData(This,dwSize,pvImData)

#define IInputMethod_SetImData(This,dwSize,pvImData)	\
    (This)->lpVtbl -> SetImData(This,dwSize,pvImData)

#define IInputMethod_UserOptionsDlg(This,hwndParent)	\
    (This)->lpVtbl -> UserOptionsDlg(This,hwndParent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_Select_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [in] */ HWND hwndSip);


void __RPC_STUB IInputMethod_Select_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_Deselect_Proxy( 
    IInputMethod __RPC_FAR * This);


void __RPC_STUB IInputMethod_Deselect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_Showing_Proxy( 
    IInputMethod __RPC_FAR * This);


void __RPC_STUB IInputMethod_Showing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_Hiding_Proxy( 
    IInputMethod __RPC_FAR * This);


void __RPC_STUB IInputMethod_Hiding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_GetInfo_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [out] */ IMINFO __RPC_FAR *pimi);


void __RPC_STUB IInputMethod_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_ReceiveSipInfo_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [in] */ SIPINFO __RPC_FAR *psi);


void __RPC_STUB IInputMethod_ReceiveSipInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_RegisterCallback_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [in] */ IIMCallback __RPC_FAR *lpIMCallback);


void __RPC_STUB IInputMethod_RegisterCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_GetImData_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [out] */ LPVOID pvImData);


void __RPC_STUB IInputMethod_GetImData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_SetImData_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [in] */ LPVOID pvImData);


void __RPC_STUB IInputMethod_SetImData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IInputMethod_UserOptionsDlg_Proxy( 
    IInputMethod __RPC_FAR * This,
    /* [in] */ HWND hwndParent);


void __RPC_STUB IInputMethod_UserOptionsDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IInputMethod_INTERFACE_DEFINED__ */
