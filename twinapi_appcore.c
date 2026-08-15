#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(twinapi);

typedef void (__stdcall *PAPPSTATE_CHANGE_ROUTINE)(BOOLEAN quarantined, void *context);
typedef void (__stdcall *PAPPCONSTRAINED_CHANGE_ROUTINE)(BOOLEAN constrained, void *context);
typedef void *PAPPSTATE_REGISTRATION;

HRESULT WINAPI RegisterAppStateChangeNotification(PAPPSTATE_CHANGE_ROUTINE routine, void *context, PAPPSTATE_REGISTRATION *reg)
{
    TRACE("routine %p, context %p, reg %p\n", routine, context, reg);
    if (!reg) return E_POINTER;
    *reg = (PAPPSTATE_REGISTRATION)(UINT_PTR)1;
    if (routine)
    {
        routine(FALSE, context);
    }
    return S_OK;
}

HRESULT WINAPI RegisterAppConstrainedChangeNotification(PAPPCONSTRAINED_CHANGE_ROUTINE routine, void *context, PAPPSTATE_REGISTRATION *reg)
{
    TRACE("routine %p, context %p, reg %p\n", routine, context, reg);
    if (!reg) return E_POINTER;
    *reg = (PAPPSTATE_REGISTRATION)(UINT_PTR)1;
    if (routine)
    {
        routine(FALSE, context);
    }
    return S_OK;
}


void WINAPI UnregisterAppStateChangeNotification(PAPPSTATE_REGISTRATION reg)
{
    TRACE("reg %p\n", reg);
}

void WINAPI UnregisterAppConstrainedChangeNotification(PAPPSTATE_REGISTRATION reg)
{
    TRACE("reg %p\n", reg);
}

/* Dummy WinRT Inspectable and Factory */
static HRESULT WINAPI dummy_inspectable_QueryInterface(IInspectable *iface, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = iface;
    return S_OK;
}

static ULONG WINAPI dummy_inspectable_AddRef(IInspectable *iface) { return 2; }
static ULONG WINAPI dummy_inspectable_Release(IInspectable *iface) { return 1; }
static HRESULT WINAPI dummy_inspectable_GetIids(IInspectable *iface, ULONG *count, GUID **iids) { if (count) *count = 0; return S_OK; }
static HRESULT WINAPI dummy_inspectable_GetRuntimeClassName(IInspectable *iface, HSTRING *className) { if (className) *className = NULL; return S_OK; }
static HRESULT WINAPI dummy_inspectable_GetTrustLevel(IInspectable *iface, TrustLevel *trustLevel) { if (trustLevel) *trustLevel = BaseTrust; return S_OK; }

static HRESULT WINAPI dummy_stub_method(void *iface, void *arg1, void *arg2, void *arg3)
{
    if (arg1 && (ULONG_PTR)arg1 > 0x10000 && !IsBadWritePtr(arg1, sizeof(void*))) *(void**)arg1 = iface;
    if (arg2 && (ULONG_PTR)arg2 > 0x10000 && !IsBadWritePtr(arg2, sizeof(void*))) *(void**)arg2 = iface;
    if (arg3 && (ULONG_PTR)arg3 > 0x10000 && !IsBadWritePtr(arg3, sizeof(void*))) *(void**)arg3 = iface;
    return S_OK;
}

static const void *dummy_inspectable_vtbl[] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method,
    dummy_stub_method
};

static struct { const void **vtbl; } dummy_inspectable = { dummy_inspectable_vtbl };

static HRESULT WINAPI dummy_factory_QueryInterface(IActivationFactory *iface, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = iface;
    return S_OK;
}

static ULONG WINAPI dummy_factory_AddRef(IActivationFactory *iface) { return 2; }
static ULONG WINAPI dummy_factory_Release(IActivationFactory *iface) { return 1; }
static HRESULT WINAPI dummy_factory_GetIids(IActivationFactory *iface, ULONG *count, GUID **iids) { if (count) *count = 0; return S_OK; }
static HRESULT WINAPI dummy_factory_GetRuntimeClassName(IActivationFactory *iface, HSTRING *className) { if (className) *className = NULL; return S_OK; }
static HRESULT WINAPI dummy_factory_GetTrustLevel(IActivationFactory *iface, TrustLevel *trustLevel) { if (trustLevel) *trustLevel = BaseTrust; return S_OK; }
static HRESULT WINAPI dummy_factory_ActivateInstance(IActivationFactory *iface, IInspectable **instance)
{
    if (!instance) return E_POINTER;
    *instance = (IInspectable*)&dummy_inspectable;
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

static struct { const void **vtbl; } dummy_activation_factory_obj = { dummy_factory_vtbl };

HRESULT WINAPI DllGetActivationFactory(HSTRING classid, IActivationFactory **factory)
{
    TRACE("classid %p, factory %p\n", classid, factory);
    if (!factory) return E_POINTER;
    *factory = (IActivationFactory*)&dummy_activation_factory_obj;
    return S_OK;
}
