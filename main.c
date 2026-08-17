/*
 * Xbox Game runtime Library
 *
 * Written by Weather
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <initguid.h>
#include <wchar.h>
#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(xgameruntime);

DWORD tlsIndex;

/* Dummy WinRT Activation Factory COM implementation */
static HRESULT WINAPI dummy_inspectable_QueryInterface(IUnknown *iface, REFIID iid, void **out)
{
    TRACE("dummy_inspectable_QueryInterface iface %p, iid %s, out %p.\n", iface, debugstr_guid(iid), out);
    if (!out) return E_POINTER;
    *out = iface;
    return S_OK;
}

static ULONG WINAPI dummy_inspectable_AddRef(IUnknown *iface)
{
    return 2;
}

static ULONG WINAPI dummy_inspectable_Release(IUnknown *iface)
{
    return 1;
}

static HRESULT WINAPI dummy_inspectable_GetIids(IUnknown *iface, ULONG *count, GUID **iids)
{
    if (count) *count = 0;
    if (iids) *iids = NULL;
    return S_OK;
}

static HRESULT WINAPI dummy_inspectable_GetRuntimeClassName(IUnknown *iface, void **className)
{
    if (className) *className = NULL;
    return S_OK;
}

static HRESULT WINAPI dummy_inspectable_GetTrustLevel(IUnknown *iface, DWORD *trustLevel)
{
    if (trustLevel) *trustLevel = 0;
    return S_OK;
}

#define STUB_METHOD(idx) \
static HRESULT WINAPI dummy_stub_method_##idx(void *iface, void *a1, void *a2, void *a3, void *a4, void *a5) { \
    void *caller = __builtin_return_address(0); \
    fprintf(stderr, "[XGDK VTABLE] dummy_inspectable method[%d] called on %p by %p (args: %p, %p, %p, %p, %p)\n", idx, iface, caller, a1, a2, a3, a4, a5); \
    return S_OK; \
}

STUB_METHOD(6)
STUB_METHOD(7)
STUB_METHOD(8)
STUB_METHOD(9)
STUB_METHOD(10)
STUB_METHOD(11)
STUB_METHOD(12)
STUB_METHOD(13)
STUB_METHOD(14)
STUB_METHOD(15)

static const void *dummy_inspectable_vtbl[] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    dummy_stub_method_6,
    dummy_stub_method_7,
    dummy_stub_method_8,
    dummy_stub_method_9,
    dummy_stub_method_10,
    dummy_stub_method_11,
    dummy_stub_method_12,
    dummy_stub_method_13,
    dummy_stub_method_14,
    dummy_stub_method_15
};

static struct {
    const void **vtbl;
} dummy_inspectable = { dummy_inspectable_vtbl };


static HRESULT WINAPI dummy_factory_QueryInterface(IUnknown *iface, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = iface;
    return S_OK;
}

static ULONG WINAPI dummy_factory_AddRef(IUnknown *iface)
{
    return 2;
}

static ULONG WINAPI dummy_factory_Release(IUnknown *iface)
{
    return 1;
}

static HRESULT WINAPI dummy_factory_GetIids(IUnknown *iface, ULONG *count, GUID **iids)
{
    if (count) *count = 0;
    if (iids) *iids = NULL;
    return S_OK;
}

static HRESULT WINAPI dummy_factory_GetRuntimeClassName(IUnknown *iface, void **className)
{
    if (className) *className = NULL;
    return S_OK;
}

static HRESULT WINAPI dummy_factory_GetTrustLevel(IUnknown *iface, DWORD *trustLevel)
{
    if (trustLevel) *trustLevel = 0;
    return S_OK;
}

static HRESULT WINAPI dummy_factory_ActivateInstance(IUnknown *iface, void **instance)
{
    if (!instance) return E_POINTER;
    *instance = &dummy_inspectable;
    return S_OK;
}

static const void *dummy_factory_vtbl[] = {
    dummy_factory_QueryInterface,
    dummy_factory_AddRef,
    dummy_factory_Release,
    dummy_factory_GetIids,
    dummy_factory_GetRuntimeClassName,
    dummy_factory_GetTrustLevel,
    dummy_factory_ActivateInstance
};

static struct {
    const void **vtbl;
} dummy_activation_factory_obj = { dummy_factory_vtbl };

static const WCHAR *get_hstring_buffer(void *hstring)
{
    if (!hstring) return L"<null>";
    typedef const WCHAR* (WINAPI *pfn_WindowsGetStringRawBuffer)(void *string, UINT32 *length);
    static pfn_WindowsGetStringRawBuffer pWindowsGetStringRawBuffer = NULL;
    if (!pWindowsGetStringRawBuffer)
    {
        HMODULE hCombase = GetModuleHandleA("combase.dll");
        if (!hCombase) hCombase = LoadLibraryA("combase.dll");
        if (hCombase)
            pWindowsGetStringRawBuffer = (pfn_WindowsGetStringRawBuffer)GetProcAddress(hCombase, "WindowsGetStringRawBuffer");
    }
    if (pWindowsGetStringRawBuffer)
    {
        UINT32 len = 0;
        const WCHAR *wstr = pWindowsGetStringRawBuffer(hstring, &len);
        if (wstr) return wstr;
    }
    return L"<unknown>";
}

HRESULT WINAPI DllGetActivationFactory(void *classid, void **factory)
{
    const WCHAR *str = get_hstring_buffer(classid);
    fprintf(stderr, "[XGDK DllGetActivationFactory] classid=%ls (%p), factory=%p\n", str ? str : L"(null)", classid, factory);
    if (!factory) return E_POINTER;

    if (str && (wcsstr(str, L"Windows.ApplicationModel.") ||
                wcsstr(str, L"Windows.Gaming.Preview.") ||
                wcsstr(str, L"Windows.Gaming.XboxLive.") ||
                wcsstr(str, L"Windows.Internal.System.") ||
                wcsstr(str, L"Windows.UI.ViewManagement.") ||
                wcsstr(str, L"Windows.UI.Core.")))
    {
        *factory = get_winrt_package_factory();
        return S_OK;
    }

    *factory = NULL;
    return REGDB_E_CLASSNOTREG;
}


BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, void *reserved )

{
    TRACE( "hinst %p, reason %lu, reserved %p.\n", hinst, reason, reserved );

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            if ((tlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) return FALSE;
            break;


        case DLL_THREAD_ATTACH:
            TlsSetValue( tlsIndex, FALSE );
            break;
        case DLL_PROCESS_DETACH:
            TlsFree( tlsIndex );
            break;
    }
    return TRUE;
}

struct initialize_options
{
    UINT32 unk;
    BOOL isInline;
    const char *gameConfig;
};

HRESULT WINAPI InitializeApiImplEx2( ULONG gdkVer, ULONG gsVer, char mode, const struct initialize_options *options )
{
    TRACE( "gdkVer %ld, gsVer %ld, mode %d, options %p.\n", gdkVer, gsVer, mode, options );
    return S_OK;
}

HRESULT WINAPI InitializeApiImplEx( ULONG gdkVer, ULONG gsVer, char mode )
{
    return InitializeApiImplEx2( gdkVer, gsVer, mode, NULL );
}

HRESULT WINAPI InitializeApiImpl( ULONG gdkVer, ULONG gsVer )
{
    return InitializeApiImplEx2( gdkVer, gsVer, 0, NULL );
}

/**
 * Entrypoint for Microsoft Gaming Desktop Runtime (GDK) COM interface acquisition.
 * 
 * When a GDK title initializes or requests an API interface (e.g. XUser, XGameSave, XStore, XThreading),
 * the Microsoft GDK flat export thunks invoke `QueryApiImpl` with the corresponding CLSID and IID.
 * 
 * @param clsid Class Identifier for the requested GDK subsystem.
 * @param iid Interface Identifier for the specific interface version.
 * @param out Pointer to store the acquired interface vtable pointer.
 * @return S_OK on success, or E_NOINTERFACE / E_POINTER on failure.
 */
HRESULT WINAPI QueryApiImpl( REFCLSID clsid, REFIID iid, void **out )
{
    FIXME( "QueryApiImpl clsid %s, iid %s, out %p.\n", debugstr_guid( clsid ), debugstr_guid( iid ), out );

    if (!out) return E_POINTER;

    if (IsEqualGUID( clsid, &CLSID_XAccessibilityImpl ))
        return IXAccessibilityImpl_QueryInterface( x_accessibility_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XAppCaptureImpl ))
        return IXAppCaptureImpl_QueryInterface( x_app_capture_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XAppCaptureMetadataImpl ))
        return IXAppCaptureMetadataImpl_QueryInterface( x_app_capture_metadata_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XDisplayImpl ))
        return IXDisplayImpl_QueryInterface( x_display_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XErrorImpl ))
        return IXErrorImpl_QueryInterface( x_error_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameImpl ))
        return IXGameImpl_QueryInterface( x_game_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameActivationImpl ))
        return IXGameActivationImpl_QueryInterface( x_game_activation_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameEventImpl ))
        return IXGameEventImpl_QueryInterface( x_game_event_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameInviteImpl ))
        return IXGameInviteImpl_QueryInterface( x_game_invite_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameProtocolImpl ))
        return IXGameProtocolImpl_QueryInterface( x_game_protocol_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameRuntimeFeatureImpl ))
        return IXGameRuntimeFeatureImpl_QueryInterface( x_game_runtime_feature_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameSaveImpl ))
        return IXGameSaveImpl_QueryInterface( x_game_save_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameStreamingImpl ))
        return IXGameStreamingImpl_QueryInterface( x_game_streaming_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XGameUiImpl ))
        return IXGameUiImpl_QueryInterface( x_game_ui_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XLauncherImpl ))
        return IXLauncherImpl_QueryInterface( x_launcher_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XNetworkingImpl ))
        return IXNetworkingImpl_QueryInterface( x_networking_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XPackageImpl ))
        return IXPackageImpl_QueryInterface( x_package_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XPersistentLocalStorageImpl ))
        return IXPersistentLocalStorageImpl_QueryInterface( x_persistent_local_storage_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XStoreImpl ))
        return IXStoreImpl_QueryInterface( x_store_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XSystemImpl ))
        return IXSystemImpl_QueryInterface( x_system_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XSystemAnalyticsImpl ))
        return IXSystemAnalyticsImpl_QueryInterface( x_system_analytics_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XThreadingImpl ))
        return IXThreadingImpl_QueryInterface( x_threading_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XUserImpl ))
        return IXUserImpl_QueryInterface( x_user_impl, iid, out );
    if (IsEqualGUID( clsid, &CLSID_XUserDeviceImpl ))
        return IXUserDeviceImpl_QueryInterface( x_user_device_impl, iid, out );

    FIXME( "Unknown clsid %s, iid %s\n", debugstr_guid( clsid ), debugstr_guid( iid ) );
    *out = &dummy_activation_factory_obj;
    return S_OK;
}


HRESULT WINAPI UninitializeApiImpl(void)
{
    return S_OK;
}
