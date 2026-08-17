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
#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(xgameruntime);

DWORD tlsIndex;

/* Dummy WinRT Activation Factory and Inspectable COM implementation */
static HRESULT WINAPI dummy_inspectable_QueryInterface(IUnknown *iface, REFIID iid, void **out)
{
    TRACE("dummy_inspectable_QueryInterface iface %p, iid %s, out %p.\n", iface, debugstr_guid(iid), out);
    if (!out) return E_POINTER;
    *out = iface;
    return S_OK;
}

static ULONG WINAPI dummy_inspectable_AddRef(IUnknown *iface) { return 2; }
static ULONG WINAPI dummy_inspectable_Release(IUnknown *iface) { return 1; }
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
static HRESULT WINAPI dummy_stub_method(void *iface)
{
    TRACE("dummy_stub_method called on %p.\n", iface);
    return S_OK;
}

static const void *dummy_inspectable_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method
};

static struct {
    const void **vtbl;
} dummy_inspectable = { dummy_inspectable_vtbl };

/* CoreTextEditContext */
static HRESULT WINAPI edit_context_prop_get(void *iface, void *val)
{
    if (val) *(void**)val = NULL;
    return S_OK;
}
static HRESULT WINAPI edit_context_prop_put(void *iface, void *val)
{
    return S_OK;
}
static HRESULT WINAPI edit_context_event_add(void *iface, void *handler, __int64 *token)
{
    if (token) *token = 1;
    return S_OK;
}
static HRESULT WINAPI edit_context_event_remove(void *iface, __int64 token)
{
    return S_OK;
}

static const void *core_text_edit_context_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    edit_context_prop_get,           /* slot 6: get_Name */
    edit_context_prop_put,           /* slot 7: put_Name */
    edit_context_prop_get,           /* slot 8: get_InputScope */
    edit_context_prop_put,           /* slot 9: put_InputScope */
    edit_context_prop_get,           /* slot 10: get_IsReadOnly */
    edit_context_prop_put,           /* slot 11: put_IsReadOnly */
    edit_context_prop_get,           /* slot 12: get_InputPaneDisplayPolicy */
    edit_context_prop_put,           /* slot 13: put_InputPaneDisplayPolicy */
    edit_context_event_add,          /* slot 14: TextRequested */
    edit_context_event_remove,       /* slot 15: TextRequested */
    edit_context_event_add,          /* slot 16: SelectionRequested */
    edit_context_event_remove,       /* slot 17: SelectionRequested */
    edit_context_event_add,          /* slot 18: LayoutRequested */
    edit_context_event_remove,       /* slot 19: LayoutRequested */
    edit_context_event_add,          /* slot 20: TextUpdating */
    edit_context_event_remove,       /* slot 21: TextUpdating */
    edit_context_event_add,          /* slot 22: SelectionUpdating */
    edit_context_event_remove,       /* slot 23: SelectionUpdating */
    edit_context_event_add,          /* slot 24: FormatUpdating */
    edit_context_event_remove,       /* slot 25: FormatUpdating */
    edit_context_event_add,          /* slot 26: CompositionStarted */
    edit_context_event_remove,       /* slot 27: CompositionStarted */
    edit_context_event_add,          /* slot 28: CompositionCompleted */
    edit_context_event_remove,       /* slot 29: CompositionCompleted */
    edit_context_event_add,          /* slot 30: FocusRemoved */
    edit_context_event_remove,       /* slot 31: FocusRemoved */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method
};

static struct {
    const void **vtbl;
} core_text_edit_context_obj = { core_text_edit_context_vtbl };

/* CoreTextServicesManager */
static HRESULT WINAPI text_services_get_InputLanguage(void *iface, void **value)
{
    if (value) *value = NULL;
    return S_OK;
}
static HRESULT WINAPI text_services_InputLanguageChanged(void *iface, void *handler, __int64 *token)
{
    if (token) *token = 1;
    return S_OK;
}
static HRESULT WINAPI text_services_InputLanguageChanged_remove(void *iface, __int64 token)
{
    return S_OK;
}
static HRESULT WINAPI text_services_CreateEditContext(void *iface, void **out)
{
    TRACE("ICoreTextServicesManager::CreateEditContext %p -> %p\n", iface, out);
    if (!out) return E_POINTER;
    *out = &core_text_edit_context_obj;
    return S_OK;
}

static const void *core_text_services_manager_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    text_services_get_InputLanguage,         /* slot 6: get_InputLanguage */
    text_services_InputLanguageChanged,      /* slot 7: eventadd InputLanguageChanged */
    text_services_InputLanguageChanged_remove, /* slot 8: eventremove InputLanguageChanged */
    text_services_CreateEditContext,         /* slot 9: CreateEditContext */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method
};

static struct {
    const void **vtbl;
} core_text_services_manager_obj = { core_text_services_manager_vtbl };

/* CoreInputView */
static HRESULT WINAPI input_view_OcclusionsChanged(void *iface, void *handler, __int64 *token)
{
    if (token) *token = 1;
    return S_OK;
}
static HRESULT WINAPI input_view_OcclusionsChanged_remove(void *iface, __int64 token)
{
    return S_OK;
}
static HRESULT WINAPI input_view_GetCoreInputViewOcclusions(void *iface, void **result)
{
    if (result) *result = NULL;
    return S_OK;
}
static HRESULT WINAPI input_view_TryShowPrimaryView(void *iface, unsigned char *result)
{
    if (result) *result = 1;
    return S_OK;
}
static HRESULT WINAPI input_view_TryHidePrimaryView(void *iface, unsigned char *result)
{
    if (result) *result = 1;
    return S_OK;
}

static const void *core_input_view_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    input_view_OcclusionsChanged,           /* slot 6 */
    input_view_OcclusionsChanged_remove,    /* slot 7 */
    input_view_GetCoreInputViewOcclusions,  /* slot 8 */
    input_view_TryShowPrimaryView,          /* slot 9 */
    input_view_TryHidePrimaryView,          /* slot 10 */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct {
    const void **vtbl;
} core_input_view_obj = { core_input_view_vtbl };

static const GUID IID_IApiInformationStatics = {0x997439fe, 0xf681, 0x4a11, {0xb4, 0x16, 0xc1, 0x3a, 0x47, 0xe8, 0xba, 0x36}};
static const GUID IID_ICoreTextServicesManagerStatics = {0x1520a388, 0xe2cf, 0x4d65, {0xae, 0xb9, 0xb3, 0x2d, 0x86, 0xfe, 0x39, 0xb9}};
static const GUID IID_ICoreInputViewStatics = {0x7d9b97cd, 0xedbe, 0x49cf, {0xa5, 0x4f, 0x33, 0x7d, 0xe0, 0x52, 0x90, 0x7f}};

static void *api_information_statics_obj_ptr;

static HRESULT WINAPI text_services_statics_GetForCurrentView(void *iface, void **value)
{
    TRACE("ICoreTextServicesManagerStatics::GetForCurrentView %p -> %p\n", iface, value);
    if (!value) return E_POINTER;
    *value = &core_text_services_manager_obj;
    return S_OK;
}

static const void *text_services_statics_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    text_services_statics_GetForCurrentView, /* slot 6: GetForCurrentView */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct {
    const void **vtbl;
} text_services_statics_obj = { text_services_statics_vtbl };

static HRESULT WINAPI input_view_statics_GetForCurrentView(void *iface, void **value)
{
    TRACE("ICoreInputViewStatics::GetForCurrentView %p -> %p\n", iface, value);
    if (!value) return E_POINTER;
    *value = &core_input_view_obj;
    return S_OK;
}

static const void *input_view_statics_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    input_view_statics_GetForCurrentView,   /* slot 6: GetForCurrentView */
    input_view_statics_GetForCurrentView,   /* slot 7: GetForUIContext */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method
};

static struct {
    const void **vtbl;
} input_view_statics_obj = { input_view_statics_vtbl };

struct dummy_com_obj {
    const void **vtbl;
};

static struct dummy_com_obj dummy_async_op_obj;
static struct dummy_com_obj dummy_device_info_obj;
static struct dummy_com_obj dummy_map_view_obj;

/* IMapView dummy */
static HRESULT WINAPI map_view_Lookup(void *iface, void *key, void **value)
{
    TRACE("IMapView::Lookup %p, key %p -> %p\n", iface, key, value);
    if (value) *value = NULL;
    return 0x8000000b; /* E_BOUNDS */
}
static HRESULT WINAPI map_view_get_Size(void *iface, DWORD *size)
{
    if (size) *size = 0;
    return S_OK;
}
static HRESULT WINAPI map_view_HasKey(void *iface, void *key, unsigned char *found)
{
    if (found) *found = 0;
    return S_OK;
}
static HRESULT WINAPI map_view_Split(void *iface, void **first, void **second)
{
    if (first) *first = NULL;
    if (second) *second = NULL;
    return S_OK;
}

static const void *dummy_map_view_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    map_view_Lookup,                     /* slot 6 */
    map_view_get_Size,                   /* slot 7 */
    map_view_HasKey,                     /* slot 8 */
    map_view_Split,                      /* slot 9 */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct dummy_com_obj dummy_map_view_obj = { dummy_map_view_vtbl };

/* IDeviceInformation */
static HRESULT WINAPI dev_info_get_Id(void *iface, void **value)
{
    if (value) *value = NULL;
    return S_OK;
}
static HRESULT WINAPI dev_info_get_Name(void *iface, void **value)
{
    if (value) *value = NULL;
    return S_OK;
}
static HRESULT WINAPI dev_info_get_IsEnabled(void *iface, unsigned char *value)
{
    if (value) *value = 1;
    return S_OK;
}
static HRESULT WINAPI dev_info_get_IsDefault(void *iface, unsigned char *value)
{
    if (value) *value = 1;
    return S_OK;
}
static HRESULT WINAPI dev_info_get_EnclosureLocation(void *iface, void **value)
{
    if (value) *value = &dummy_inspectable;
    return S_OK;
}
static HRESULT WINAPI dev_info_get_Properties(void *iface, void **value)
{
    TRACE("IDeviceInformation::get_Properties %p -> %p\n", iface, value);
    if (!value) return E_POINTER;
    *value = &dummy_map_view_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info_Update(void *iface, void *info)
{
    return S_OK;
}
static HRESULT WINAPI dev_info_GetThumbnailAsync(void *iface, void **operation)
{
    if (operation) *operation = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info_GetGlyphThumbnailAsync(void *iface, void **operation)
{
    if (operation) *operation = &dummy_async_op_obj;
    return S_OK;
}

static const void *dummy_device_info_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    dev_info_get_Id,                     /* slot 6 */
    dev_info_get_Name,                   /* slot 7 */
    dev_info_get_IsEnabled,              /* slot 8 */
    dev_info_get_IsDefault,              /* slot 9 */
    dev_info_get_EnclosureLocation,      /* slot 10 */
    dev_info_get_Properties,             /* slot 11 */
    dev_info_Update,                     /* slot 12 */
    dev_info_GetThumbnailAsync,          /* slot 13 */
    dev_info_GetGlyphThumbnailAsync,     /* slot 14 */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct dummy_com_obj dummy_device_info_obj = { dummy_device_info_vtbl };

/* IAsyncOperation dummy */
static HRESULT WINAPI async_op_put_Completed(void *iface, void *handler)
{
    TRACE("IAsyncOperation::put_Completed %p, handler %p\n", iface, handler);
    if (handler)
    {
        void **handler_vtbl = *(void***)handler;
        if (handler_vtbl && handler_vtbl[3])
        {
            typedef HRESULT (WINAPI *InvokeFunc)(void*, void*, DWORD);
            ((InvokeFunc)handler_vtbl[3])(handler, iface, 1 /* Completed */);
        }
    }
    return S_OK;
}
static HRESULT WINAPI async_op_get_Completed(void *iface, void **handler)
{
    if (handler) *handler = NULL;
    return S_OK;
}
static HRESULT WINAPI async_op_GetResults(void *iface, void **results)
{
    TRACE("IAsyncOperation::GetResults %p -> %p\n", iface, results);
    if (results) *results = &dummy_device_info_obj;
    return S_OK;
}
static HRESULT WINAPI async_op_get_Status(void *iface, DWORD *status)
{
    if (status) *status = 1; /* Completed */
    return S_OK;
}
static HRESULT WINAPI async_op_get_ErrorCode(void *iface, HRESULT *error_code)
{
    if (error_code) *error_code = S_OK;
    return S_OK;
}

static const void *dummy_async_op_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    async_op_put_Completed,              /* slot 6 */
    async_op_get_Completed,              /* slot 7 */
    async_op_GetResults,                 /* slot 8 */
    async_op_get_Status,                 /* slot 9 */
    async_op_get_ErrorCode,              /* slot 10 */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct dummy_com_obj dummy_async_op_obj = { dummy_async_op_vtbl };

static const GUID IID_IDeviceInformationStatics = {0xc17f100e, 0x3a46, 0x4a78, {0x80, 0x13, 0x76, 0x9d, 0xc9, 0xb9, 0x73, 0x90}};
static const GUID IID_IDeviceInformationStatics2 = {0x493b4f34, 0xa84f, 0x45fd, {0x91, 0x67, 0x15, 0xd1, 0xcb, 0x1b, 0xd1, 0xf9}};

/* DeviceWatcher */
static HRESULT WINAPI device_watcher_event_add(void *iface, void *handler, __int64 *token)
{
    if (token) *token = 1;
    return S_OK;
}
static HRESULT WINAPI device_watcher_event_remove(void *iface, __int64 token)
{
    return S_OK;
}
static HRESULT WINAPI device_watcher_get_status(void *iface, DWORD *status)
{
    if (status) *status = 0;
    return S_OK;
}
static HRESULT WINAPI device_watcher_stub(void *iface)
{
    return S_OK;
}

static const void *device_watcher_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    device_watcher_event_add,            /* slot 6: Added */
    device_watcher_event_remove,         /* slot 7: Added */
    device_watcher_event_add,            /* slot 8: Updated */
    device_watcher_event_remove,         /* slot 9: Updated */
    device_watcher_event_add,            /* slot 10: Removed */
    device_watcher_event_remove,         /* slot 11: Removed */
    device_watcher_event_add,            /* slot 12: EnumerationCompleted */
    device_watcher_event_remove,         /* slot 13: EnumerationCompleted */
    device_watcher_event_add,            /* slot 14: Stopped */
    device_watcher_event_remove,         /* slot 15: Stopped */
    device_watcher_get_status,           /* slot 16: get_Status */
    device_watcher_stub,                 /* slot 17: Start */
    device_watcher_stub,                 /* slot 18: Stop */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct {
    const void **vtbl;
} device_watcher_obj = { device_watcher_vtbl };

/* IDeviceInformationStatics (1) */
static HRESULT WINAPI dev_info1_CreateFromIdAsync(void *iface, void *id, void **async_op)
{
    TRACE("IDeviceInformationStatics::CreateFromIdAsync %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_CreateFromIdAsyncAdditionalProperties(void *iface, void *id, void *props, void **async_op)
{
    TRACE("IDeviceInformationStatics::CreateFromIdAsyncAdditionalProperties %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_FindAllAsync(void *iface, void **async_op)
{
    TRACE("IDeviceInformationStatics::FindAllAsync %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_FindAllAsyncDeviceClass(void *iface, DWORD device_class, void **async_op)
{
    TRACE("IDeviceInformationStatics::FindAllAsyncDeviceClass %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_FindAllAsyncAqsFilter(void *iface, void *filter, void **async_op)
{
    TRACE("IDeviceInformationStatics::FindAllAsyncAqsFilter %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_FindAllAsyncAqsFilterAndAdditionalProperties(void *iface, void *filter, void *props, void **async_op)
{
    TRACE("IDeviceInformationStatics::FindAllAsyncAqsFilterAndAdditionalProperties %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_CreateWatcher(void *iface, void **watcher)
{
    TRACE("IDeviceInformationStatics::CreateWatcher %p -> %p\n", iface, watcher);
    if (!watcher) return E_POINTER;
    *watcher = &device_watcher_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_CreateWatcherDeviceClass(void *iface, DWORD device_class, void **watcher)
{
    TRACE("IDeviceInformationStatics::CreateWatcherDeviceClass %p, class %u -> %p\n", iface, device_class, watcher);
    if (!watcher) return E_POINTER;
    *watcher = &device_watcher_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_CreateWatcherAqsFilter(void *iface, void *filter, void **watcher)
{
    TRACE("IDeviceInformationStatics::CreateWatcherAqsFilter %p -> %p\n", iface, watcher);
    if (!watcher) return E_POINTER;
    *watcher = &device_watcher_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info1_CreateWatcherAqsFilterAndAdditionalProperties(void *iface, void *filter, void *props, void **watcher)
{
    TRACE("IDeviceInformationStatics::CreateWatcherAqsFilterAndAdditionalProperties %p -> %p\n", iface, watcher);
    if (!watcher) return E_POINTER;
    *watcher = &device_watcher_obj;
    return S_OK;
}

static const void *device_information_statics1_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    dev_info1_CreateFromIdAsync,                              /* slot 6 */
    dev_info1_CreateFromIdAsyncAdditionalProperties,          /* slot 7 */
    dev_info1_FindAllAsync,                                   /* slot 8 */
    dev_info1_FindAllAsyncDeviceClass,                        /* slot 9 */
    dev_info1_FindAllAsyncAqsFilter,                          /* slot 10 */
    dev_info1_FindAllAsyncAqsFilterAndAdditionalProperties,   /* slot 11 */
    dev_info1_CreateWatcher,                                  /* slot 12 */
    dev_info1_CreateWatcherDeviceClass,                       /* slot 13 */
    dev_info1_CreateWatcherAqsFilter,                         /* slot 14 */
    dev_info1_CreateWatcherAqsFilterAndAdditionalProperties,  /* slot 15 */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method
};

static struct {
    const void **vtbl;
} device_information_statics1_obj = { device_information_statics1_vtbl };

/* IDeviceInformationStatics2 (2) */
static HRESULT WINAPI dev_info2_GetAqsFilterFromDeviceClass(void *iface, DWORD device_class, void **filter)
{
    TRACE("IDeviceInformationStatics2::GetAqsFilterFromDeviceClass %p -> %p\n", iface, filter);
    if (filter) *filter = NULL;
    return S_OK;
}
static HRESULT WINAPI dev_info2_CreateFromIdAsync(void *iface, void *device_id, void *props, DWORD kind, void **async_op)
{
    TRACE("IDeviceInformationStatics2::CreateFromIdAsync %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info2_FindAllAsync(void *iface, void *filter, void *props, DWORD kind, void **async_op)
{
    TRACE("IDeviceInformationStatics2::FindAllAsync %p -> %p\n", iface, async_op);
    if (!async_op) return E_POINTER;
    *async_op = &dummy_async_op_obj;
    return S_OK;
}
static HRESULT WINAPI dev_info2_CreateWatcher(void *iface, void *filter, void *props, DWORD kind, void **watcher)
{
    TRACE("IDeviceInformationStatics2::CreateWatcher %p -> %p\n", iface, watcher);
    if (!watcher) return E_POINTER;
    *watcher = &device_watcher_obj;
    return S_OK;
}

static const void *device_information_statics2_vtbl[64] = {
    dummy_inspectable_QueryInterface,
    dummy_inspectable_AddRef,
    dummy_inspectable_Release,
    dummy_inspectable_GetIids,
    dummy_inspectable_GetRuntimeClassName,
    dummy_inspectable_GetTrustLevel,
    dev_info2_GetAqsFilterFromDeviceClass,                    /* slot 6 */
    dev_info2_CreateFromIdAsync,                              /* slot 7 */
    dev_info2_FindAllAsync,                                   /* slot 8 */
    dev_info2_CreateWatcher,                                  /* slot 9 */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method
};

static struct {
    const void **vtbl;
} device_information_statics2_obj = { device_information_statics2_vtbl };

static HRESULT WINAPI dummy_factory_QueryInterface(IUnknown *iface, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    if (IsEqualGUID(iid, &IID_IApiInformationStatics))
    {
        *out = api_information_statics_obj_ptr;
        return S_OK;
    }
    if (IsEqualGUID(iid, &IID_ICoreTextServicesManagerStatics))
    {
        *out = &text_services_statics_obj;
        return S_OK;
    }
    if (IsEqualGUID(iid, &IID_ICoreInputViewStatics))
    {
        *out = &input_view_statics_obj;
        return S_OK;
    }
    if (IsEqualGUID(iid, &IID_IDeviceInformationStatics))
    {
        *out = &device_information_statics1_obj;
        return S_OK;
    }
    if (IsEqualGUID(iid, &IID_IDeviceInformationStatics2))
    {
        *out = &device_information_statics2_obj;
        return S_OK;
    }
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

static const void *dummy_factory_vtbl[64] = {
    dummy_factory_QueryInterface,
    dummy_factory_AddRef,
    dummy_factory_Release,
    dummy_factory_GetIids,
    dummy_factory_GetRuntimeClassName,
    dummy_factory_GetTrustLevel,
    dummy_factory_ActivateInstance,      /* slot 6: ActivateInstance / GetForCurrentView */
    dummy_factory_ActivateInstance,      /* slot 7: GetForUIContext */
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method, dummy_stub_method, dummy_stub_method,
    dummy_stub_method, dummy_stub_method
};

static struct {
    const void **vtbl;
} dummy_activation_factory_obj = { dummy_factory_vtbl };

static HRESULT WINAPI api_info_IsTypePresent(void *This, void *type_name, unsigned char *value)
{
    TRACE("ApiInformation::IsTypePresent %p -> FALSE\n", type_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsMethodPresent(void *This, void *type_name, void *method_name, unsigned char *value)
{
    TRACE("ApiInformation::IsMethodPresent %p, %p -> FALSE\n", type_name, method_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsMethodPresentWithArity(void *This, void *type_name, void *method_name, UINT32 arity, unsigned char *value)
{
    TRACE("ApiInformation::IsMethodPresentWithArity %p, %p, %u -> FALSE\n", type_name, method_name, arity);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsEventPresent(void *This, void *type_name, void *event_name, unsigned char *value)
{
    TRACE("ApiInformation::IsEventPresent %p, %p -> FALSE\n", type_name, event_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsPropertyPresent(void *This, void *type_name, void *property_name, unsigned char *value)
{
    TRACE("ApiInformation::IsPropertyPresent %p, %p -> FALSE\n", type_name, property_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsReadOnlyPropertyPresent(void *This, void *type_name, void *property_name, unsigned char *value)
{
    TRACE("ApiInformation::IsReadOnlyPropertyPresent %p, %p -> FALSE\n", type_name, property_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsWriteablePropertyPresent(void *This, void *type_name, void *property_name, unsigned char *value)
{
    TRACE("ApiInformation::IsWriteablePropertyPresent %p, %p -> FALSE\n", type_name, property_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsEnumNamedValuePresent(void *This, void *enum_type_name, void *value_name, unsigned char *value)
{
    TRACE("ApiInformation::IsEnumNamedValuePresent %p, %p -> FALSE\n", enum_type_name, value_name);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsApiContractPresentByMajor(void *This, void *contract_name, UINT16 major, unsigned char *value)
{
    TRACE("ApiInformation::IsApiContractPresentByMajor %p, %u -> FALSE\n", contract_name, major);
    if (value) *value = FALSE;
    return S_OK;
}

static HRESULT WINAPI api_info_IsApiContractPresentByMajorAndMinor(void *This, void *contract_name, UINT16 major, UINT16 minor, unsigned char *value)
{
    TRACE("ApiInformation::IsApiContractPresentByMajorAndMinor %p, %u.%u -> FALSE\n", contract_name, major, minor);
    if (value) *value = FALSE;
    return S_OK;
}

static const void *api_info_statics_vtbl[] = {
    dummy_factory_QueryInterface,
    dummy_factory_AddRef,
    dummy_factory_Release,
    dummy_factory_GetIids,
    dummy_factory_GetRuntimeClassName,
    dummy_factory_GetTrustLevel,
    api_info_IsTypePresent,
    api_info_IsMethodPresent,
    api_info_IsMethodPresentWithArity,
    api_info_IsEventPresent,
    api_info_IsPropertyPresent,
    api_info_IsReadOnlyPropertyPresent,
    api_info_IsWriteablePropertyPresent,
    api_info_IsEnumNamedValuePresent,
    api_info_IsApiContractPresentByMajor,
    api_info_IsApiContractPresentByMajorAndMinor
};

static struct {
    const void **vtbl;
} api_information_statics_obj = { api_info_statics_vtbl };

static void *api_information_statics_obj_ptr = &api_information_statics_obj;

HRESULT WINAPI DllGetActivationFactory(void *classid, void **factory)
{
    TRACE( "classid %p, factory %p.\n", classid, factory );
    if (!factory) return E_POINTER;
    *factory = &dummy_activation_factory_obj;
    return S_OK;
}

HRESULT WINAPI DllCanUnloadNow(void)
{
    return S_FALSE;
}

HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID iid, void **out)
{
    if (!out) return E_POINTER;
    *out = &dummy_activation_factory_obj;
    return S_OK;
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
