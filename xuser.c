/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XUser
 *
 * Copyright 2026 Olivia Ryan
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

#include "private.h"
#include <wchar.h>

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

struct x_user
{
    IXUserImpl6 IXUserImpl6_iface;
    IXUserGamertagImpl IXUserGamertagImpl_iface;
    IXUserDeviceImpl2 IXUserDeviceImpl2_iface;
    LONG ref;
};

static inline struct x_user *impl_from_IXUserImpl6( IXUserImpl6 *iface )
{
    return CONTAINING_RECORD( iface, struct x_user, IXUserImpl6_iface );
}

static HRESULT WINAPI x_user_QueryInterface( IXUserImpl6 *iface, REFIID iid, void **out )
{
    struct x_user *impl = impl_from_IXUserImpl6( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown    ) ||
        IsEqualGUID( iid, &IID_IXUserImpl  ) ||
        IsEqualGUID( iid, &IID_IXUserImpl2 ) ||
        IsEqualGUID( iid, &IID_IXUserImpl3 ) ||
        IsEqualGUID( iid, &IID_IXUserImpl4 ) ||
        IsEqualGUID( iid, &IID_IXUserImpl5 ) ||
        IsEqualGUID( iid, &IID_IXUserImpl6 ))
    {
        IXUserImpl6_AddRef( *out = &impl->IXUserImpl6_iface );
        return S_OK;
    }

    if (IsEqualGUID( iid, &IID_IXUserGamertagImpl ))
    {
        IXUserGamertagImpl_AddRef( *out = &impl->IXUserGamertagImpl_iface );
        return S_OK;
    }

    if (IsEqualGUID( iid, &IID_IXUserDeviceImpl ) ||
        IsEqualGUID( iid, &IID_IXUserDeviceImpl2 ))
    {
        IXUserDeviceImpl2_AddRef( *out = &impl->IXUserDeviceImpl2_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_user_AddRef( IXUserImpl6 *iface )
{
    struct x_user *impl = impl_from_IXUserImpl6( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_user_Release( IXUserImpl6 *iface )
{
    struct x_user *impl = impl_from_IXUserImpl6( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI dummy_user_func(void *self) { return S_OK; }
static void *dummy_user_vtable[64] = {
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
    dummy_user_func, dummy_user_func, dummy_user_func, dummy_user_func,
};

struct user_handle_data {
    void **vtable;
    UINT64 xuid;
    UINT32 local_id;
    UINT32 state;
    char gamertag[64];
    void *extra[32];
};

static struct user_handle_data default_user_obj = {
    dummy_user_vtable,
    0x0009000000000001ULL,
    1,
    0,
    "Player"
};


static HRESULT WINAPI x_user_XUserDuplicateHandle( IXUserImpl6 *iface, XUserHandle handle, XUserHandle *duplicatedHandle )
{
    TRACE( "iface %p, handle %p, duplicatedHandle %p\n", iface, handle, duplicatedHandle );
    if (!duplicatedHandle) return E_POINTER;
    *duplicatedHandle = (handle) ? handle : (XUserHandle)&default_user_obj;
    return S_OK;
}

static void WINAPI x_user_XUserCloseHandle( IXUserImpl6 *iface, XUserHandle user )
{
    TRACE( "iface %p, user %p\n", iface, user );
}

static INT32 WINAPI x_user_XUserCompare( IXUserImpl6 *iface, XUserHandle user1, XUserHandle user2 )
{
    TRACE( "iface %p, user1 %p, user2 %p\n", iface, user1, user2 );
    return (user1 == user2) ? 0 : ((user1 < user2) ? -1 : 1);
}

static HRESULT WINAPI x_user_XUserGetMaxUsers( IXUserImpl6 *iface, UINT32 *maxUsers )
{
    TRACE( "iface %p, maxUsers %p.\n", iface, maxUsers );
    *maxUsers = 1;
    return S_OK;
}

static void ensure_user_info(void)
{
    if (default_user_obj.xuid == 0 || default_user_obj.xuid == 0x0009000000000001ULL || !strcmp(default_user_obj.gamertag, "Player"))
    {
        XodusUserInfo info;
        memset(&info, 0, sizeof(info));
        if (ipc_xuser_add(0, &info) == S_OK)
        {
            if (info.xuid[0])
            {
                char *end = NULL;
                unsigned long long parsed = strtoull(info.xuid, &end, 10);
                if ((end && *end != '\0') || (strlen(info.xuid) == 16 && (info.xuid[0] == '0' && info.xuid[1] == '0')))
                {
                    parsed = strtoull(info.xuid, NULL, 16);
                }
                if (parsed != 0) default_user_obj.xuid = parsed;
            }
            if (info.gamertag[0])
            {
                snprintf(default_user_obj.gamertag, sizeof(default_user_obj.gamertag), "%s", info.gamertag);
            }
            fprintf(stderr, "[GDK XUser] ensure_user_info loaded: XUID %llu (0x%llx), Gamertag '%s'\n",
                    (unsigned long long)default_user_obj.xuid,
                    (unsigned long long)default_user_obj.xuid,
                    default_user_obj.gamertag);
        }
    }
}

struct user_change_listener {
    XTaskQueueHandle queue;
    void *context;
    XUserChangeEventCallback *callback;
    UINT64 token;
};

static struct user_change_listener g_user_change_listeners[16];
static int g_user_change_listener_count = 0;

static void fire_user_change_event(XUserLocalId local_id, XUserChangeEvent event)
{
    int i;
    for (i = 0; i < g_user_change_listener_count; i++)
    {
        if (g_user_change_listeners[i].callback)
        {
            TRACE("[GDK XUser] Firing user change event %d to listener %d (cb %p, ctx %p)\n",
                  event, i, g_user_change_listeners[i].callback, g_user_change_listeners[i].context);
            g_user_change_listeners[i].callback(g_user_change_listeners[i].context, local_id, event);
        }
    }
}

static HRESULT WINAPI x_user_XUserAddAsync( IXUserImpl6 *iface, XUserAddOptions options, XAsyncBlock *async )
{
    XUserLocalId local_id = {1};
    fprintf(stderr, "[GDK XUser] XUserAddAsync called: options=0x%x, async=%p (queue=%p, ctx=%p, cb=%p)\n",
            options, async, async ? async->queue : NULL, async ? async->context : NULL, async ? async->callback : NULL);
    ensure_user_info();
    complete_async(async);
    fire_user_change_event(local_id, XUserChangeEvent_SignedInAgain);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserAddResult( IXUserImpl6 *iface, XAsyncBlock *async, XUserHandle *newUser )
{
    ensure_user_info();
    fprintf(stderr, "[GDK XUser] XUserAddResult called: async=%p, newUser=%p\n", async, newUser);
    if (async)
    {
        void *state = *(void**)&async->internal[0];
        if (state)
        {
            free(state);
            *(void**)&async->internal[0] = NULL;
        }
    }
    if (newUser)
    {
        *newUser = (XUserHandle)&default_user_obj;
        fprintf(stderr, "[GDK XUser] Returning user handle %p (XUID: %llu, Gamertag: '%s')\n",
                *newUser, (unsigned long long)default_user_obj.xuid, default_user_obj.gamertag);
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserGetLocalId( IXUserImpl6 *iface, XUserHandle user, XUserLocalId *userLocalId )
{
    TRACE( "iface %p, user %p, userLocalId %p.\n", iface, user, userLocalId );
    if (userLocalId)
    {
        userLocalId->value = 1;
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserFindUserByLocalId( IXUserImpl6 *iface, XUserLocalId userLocalId, XUserHandle *handle )
{
    TRACE( "iface %p, userLocalId %llu, handle %p\n", iface, (unsigned long long)userLocalId.value, handle );
    if (!handle) return E_POINTER;
    ensure_user_info();
    *handle = (XUserHandle)&default_user_obj;
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetId( IXUserImpl6 *iface, XUserHandle user, UINT64 *userId )
{
    ensure_user_info();
    fprintf(stderr, "[GDK XUser] XUserGetId called: user=%p -> userId=%llu (0x%llx)\n",
            user, (unsigned long long)default_user_obj.xuid, (unsigned long long)default_user_obj.xuid);
    if (userId)
    {
        *userId = default_user_obj.xuid;
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserFindUserById( IXUserImpl6 *iface, UINT64 userId, XUserHandle *handle )
{
    fprintf(stderr, "[GDK XUser] XUserFindUserById called: userId=%llu\n", (unsigned long long)userId);
    if (!handle) return E_POINTER;
    ensure_user_info();
    *handle = (XUserHandle)&default_user_obj;
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetIsGuest( IXUserImpl6 *iface, XUserHandle user, BOOLEAN *isGuest )
{
    TRACE( "iface %p, user %p, isGuest %p\n", iface, user, isGuest );
    if (!isGuest) return E_POINTER;
    *isGuest = FALSE;
    return S_OK;
}


static HRESULT WINAPI x_user_XUserGetState( IXUserImpl6 *iface, XUserHandle user, XUserState *state )
{
    TRACE( "iface %p, user %p, state %p\n", iface, user, state );
    if (!state) return E_POINTER;
    *state = XUserState_SignedIn;
    return S_OK;
}


static HRESULT WINAPI __PADDING__( IXUserImpl6 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_XUserGetGamerPictureAsync( IXUserImpl6 *iface, XUserHandle user, XUserGamerPictureSize pictureSize, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, pictureSize %d, async %p.\n", iface, user, pictureSize, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetGamerPictureResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *bufferSize )
{
    TRACE( "iface %p, async %p, bufferSize %p.\n", iface, async, bufferSize );
    if (bufferSize)
    {
        *bufferSize = 4; /* PNG header length */
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserGetGamerPictureResult( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T bufferSize, void *buffer, SIZE_T *bufferUsed )
{
    TRACE( "iface %p, async %p, bufferSize %Iu, buffer %p, bufferUsed %p.\n", iface, async, bufferSize, buffer, bufferUsed );
    return ipc_xuser_get_gamer_picture(1, 0, buffer, bufferSize, bufferUsed);
}

static HRESULT WINAPI x_user_XUserGetAgeGroup( IXUserImpl6 *iface, XUserHandle user, XUserAgeGroup *ageGroup )
{
    TRACE( "iface %p, user %p, ageGroup %p.\n", iface, user, ageGroup );
    if (ageGroup)
    {
        *ageGroup = XUserAgeGroup_Adult;
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserCheckPrivilege( IXUserImpl6 *iface, XUserHandle user, XUserPrivilegeOptions options, XUserPrivilege privilege, BOOLEAN *hasPrivilege, XUserPrivilegeDenyReason *reason )
{
    BOOL has_priv = TRUE;
    UINT32 deny_reason = 0;
    HRESULT hr;
    hr = ipc_xuser_check_privilege(1, (UINT32)privilege, &has_priv, &deny_reason);
    if (hasPrivilege) *hasPrivilege = has_priv ? TRUE : FALSE;
    if (reason) *reason = (XUserPrivilegeDenyReason)deny_reason;
    fprintf(stderr, "[GDK XUser] XUserCheckPrivilege: privilege=%d, hasPrivilege=%d, denyReason=%u\n",
            privilege, has_priv, deny_reason);
    return hr;
}

static HRESULT WINAPI x_user_XUserResolvePrivilegeWithUiAsync( IXUserImpl6 *iface, XUserHandle user, XUserPrivilegeOptions options, XUserPrivilege privilege, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, options %d, privilege %d, async %p.\n", iface, user, options, privilege, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserResolvePrivilegeWithUiResult( IXUserImpl6 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p.\n", iface, async );
    return S_OK;
}

struct async_token_req {
    XAsyncBlock *async;
    char method[64];
    char url[1024];
    BYTE body[4096];
    SIZE_T body_len;
};

static struct async_token_req g_token_reqs[64];
static int g_token_req_count = 0;

static void register_token_req(XAsyncBlock *async, const char *method, const char *url, const void *body, SIZE_T body_len)
{
    int i;
    for (i = 0; i < g_token_req_count; i++)
    {
        if (g_token_reqs[i].async == async)
        {
            if (method) snprintf(g_token_reqs[i].method, sizeof(g_token_reqs[i].method), "%s", method);
            if (url) snprintf(g_token_reqs[i].url, sizeof(g_token_reqs[i].url), "%s", url);
            g_token_reqs[i].body_len = 0;
            if (body && body_len > 0)
            {
                SIZE_T copy_len = body_len < sizeof(g_token_reqs[i].body) ? body_len : sizeof(g_token_reqs[i].body);
                memcpy(g_token_reqs[i].body, body, copy_len);
                g_token_reqs[i].body_len = copy_len;
            }
            return;
        }
    }
    if (g_token_req_count < 64)
    {
        g_token_reqs[g_token_req_count].async = async;
        snprintf(g_token_reqs[g_token_req_count].method, sizeof(g_token_reqs[g_token_req_count].method), "%s", method ? method : "GET");
        snprintf(g_token_reqs[g_token_req_count].url, sizeof(g_token_reqs[g_token_req_count].url), "%s", url ? url : "http://xboxlive.com");
        g_token_reqs[g_token_req_count].body_len = 0;
        if (body && body_len > 0)
        {
            SIZE_T copy_len = body_len < sizeof(g_token_reqs[g_token_req_count].body) ? body_len : sizeof(g_token_reqs[g_token_req_count].body);
            memcpy(g_token_reqs[g_token_req_count].body, body, copy_len);
            g_token_reqs[g_token_req_count].body_len = copy_len;
        }
        g_token_req_count++;
    }
}

static struct async_token_req *get_token_req(XAsyncBlock *async)
{
    int i;
    for (i = 0; i < g_token_req_count; i++)
    {
        if (g_token_reqs[i].async == async)
            return &g_token_reqs[i];
    }
    return NULL;
}

static const char *get_token_req_url(XAsyncBlock *async)
{
    struct async_token_req *req = get_token_req(async);
    return req ? req->url : "http://xboxlive.com";
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureAsync( IXUserImpl6 *iface, XUserHandle user, XUserGetTokenAndSignatureOptions options, const char *method, const char *url, SIZE_T headerCount, const XUserGetTokenAndSignatureHttpHeader *headers, SIZE_T bodySize, const void *bodyBuffer, XAsyncBlock *async )
{
    SIZE_T i;
    ensure_user_info();
    register_token_req(async, method, url, bodyBuffer, bodySize);
    ERR( "[GDK XUser] XUserGetTokenAndSignatureAsync: options=0x%x, method='%s', url='%s', headerCount=%zu, bodySize=%zu, async=%p\n",
           options, method ? method : "", url ? url : "", headerCount, bodySize, async );
    for (i = 0; i < headerCount && headers; i++)
    {
        ERR( "  Header[%zu]: '%s' = '%s'\n", i, headers[i].name ? headers[i].name : "", headers[i].value ? headers[i].value : "" );
    }
    if (bodySize > 0 && bodyBuffer)
    {
        ERR( "  Body (len %zu): '%.*s'\n", bodySize, (int)(bodySize > 256 ? 256 : bodySize), (const char *)bodyBuffer );
    }
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *bufferSize )
{
    if (bufferSize)
    {
        *bufferSize = sizeof(XUserGetTokenAndSignatureData) + 16384;
        TRACE( "[GDK XUser] XUserGetTokenAndSignatureResultSize: bufferSize=%zu\n", *bufferSize );
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureResult( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T bufferSize, void *buffer, XUserGetTokenAndSignatureData **ptrToBuffer, SIZE_T *bufferUsed )
{
    char temp_token[8192] = {0};
    char temp_sig[2048] = {0};
    XUserGetTokenAndSignatureData *data;
    char *token_ptr;
    char *sig_ptr;
    struct async_token_req *req = get_token_req(async);
    const char *req_url = req ? req->url : "http://xboxlive.com";
    const char *req_method = req ? req->method : "GET";
    const void *req_body = req ? req->body : NULL;
    SIZE_T req_body_len = req ? req->body_len : 0;

    if (!buffer || bufferSize < sizeof(XUserGetTokenAndSignatureData) + 512)
        return E_INVALIDARG;

    ipc_xuser_get_token(1, req_url, req_method, req_url, req_body, req_body_len, temp_token, sizeof(temp_token), temp_sig, sizeof(temp_sig));

    data = (XUserGetTokenAndSignatureData *)buffer;
    token_ptr = (char *)buffer + sizeof(XUserGetTokenAndSignatureData);
    data->tokenSize = strlen(temp_token);
    memcpy(token_ptr, temp_token, data->tokenSize + 1);
    data->token = token_ptr;

    if (temp_sig[0] != '\0')
    {
        sig_ptr = token_ptr + data->tokenSize + 1;
        data->signatureSize = strlen(temp_sig);
        memcpy(sig_ptr, temp_sig, data->signatureSize + 1);
        data->signature = sig_ptr;
    }
    else
    {
        data->signature = NULL;
        data->signatureSize = 0;
    }

    TRACE( "[GDK XUser] XUserGetTokenAndSignatureResult: token len=%zu, sig len=%zu for RP '%s'\n",
           data->tokenSize, data->signatureSize, req_url );

    if (ptrToBuffer) *ptrToBuffer = data;
    if (bufferUsed) *bufferUsed = sizeof(XUserGetTokenAndSignatureData) + (data->tokenSize + 1) + (data->signature ? data->signatureSize + 1 : 0);

    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureUtf16Async( IXUserImpl6 *iface, XUserHandle user, XUserGetTokenAndSignatureOptions options, const WCHAR *method, const WCHAR *url, SIZE_T headerCount, const XUserGetTokenAndSignatureUtf16HttpHeader *headers, SIZE_T bodySize, const void *bodyBuffer, XAsyncBlock *async )
{
    char method_a[64] = {0};
    char url_a[1024] = {0};
    SIZE_T i;
    ensure_user_info();
    if (method && method[0]) WideCharToMultiByte(CP_UTF8, 0, method, -1, method_a, sizeof(method_a) - 1, NULL, NULL);
    if (url && url[0]) WideCharToMultiByte(CP_UTF8, 0, url, -1, url_a, sizeof(url_a) - 1, NULL, NULL);

    register_token_req(async, method_a, url_a, bodyBuffer, bodySize);
    ERR( "[GDK XUser] XUserGetTokenAndSignatureUtf16Async: options=0x%x, method='%s', url='%s', headerCount=%zu, bodySize=%zu, async=%p\n",
           options, method_a, url_a, headerCount, bodySize, async );
    for (i = 0; i < headerCount && headers; i++)
    {
        char name_a[256] = {0};
        char val_a[512] = {0};
        if (headers[i].name) WideCharToMultiByte(CP_UTF8, 0, headers[i].name, -1, name_a, sizeof(name_a) - 1, NULL, NULL);
        if (headers[i].value) WideCharToMultiByte(CP_UTF8, 0, headers[i].value, -1, val_a, sizeof(val_a) - 1, NULL, NULL);
        ERR( "  HeaderUtf16[%zu]: '%s' = '%s'\n", i, name_a, val_a );
    }
    if (bodySize > 0 && bodyBuffer)
    {
        ERR( "  Body (len %zu): '%.*s'\n", bodySize, (int)(bodySize > 256 ? 256 : bodySize), (const char *)bodyBuffer );
    }
    /* ERR-level to always capture Athena/Ares requests for debugging */
    if (strstr(url_a, "ares") || strstr(url_a, "athena") || strstr(url_a, "msrareservices"))
    {
        ERR( "[XODUS-DIAG][ERMINEBEARD] XUserGetTokenAndSignatureUtf16Async: method='%s', url='%s'\n", method_a, url_a );
        if (bodySize > 0 && bodyBuffer)
            ERR( "[XODUS-DIAG][ERMINEBEARD] Request body (len=%zu): '%.*s'\n",
                 bodySize, (int)(bodySize > 2048 ? 2048 : bodySize), (const char *)bodyBuffer );
    }
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureUtf16ResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *bufferSize )
{
    if (!bufferSize) return E_POINTER;
    *bufferSize = sizeof(XUserGetTokenAndSignatureUtf16Data) + 16384 * sizeof(WCHAR);
    TRACE( "[GDK XUser] XUserGetTokenAndSignatureUtf16ResultSize: returning bufferSize=%zu\n", *bufferSize );
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureUtf16Result( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T bufferSize, void *buffer, XUserGetTokenAndSignatureUtf16Data **ptrToBuffer, SIZE_T *bufferUsed )
{
    char token_buf[8192] = {0};
    char sig_buf[2048] = {0};
    XUserGetTokenAndSignatureUtf16Data *data;
    WCHAR *token_w;
    WCHAR *sig_w;
    SIZE_T max_wchars;
    struct async_token_req *req = get_token_req(async);
    const char *req_url = req ? req->url : "http://xboxlive.com";
    const char *req_method = req ? req->method : "GET";
    const void *req_body = req ? req->body : NULL;
    SIZE_T req_body_len = req ? req->body_len : 0;

    if (!buffer || bufferSize < sizeof(XUserGetTokenAndSignatureUtf16Data) + 256 * sizeof(WCHAR))
        return E_INVALIDARG;

    data = (XUserGetTokenAndSignatureUtf16Data *)buffer;
    token_w = (WCHAR *)((BYTE *)buffer + sizeof(XUserGetTokenAndSignatureUtf16Data));
    max_wchars = (bufferSize - sizeof(XUserGetTokenAndSignatureUtf16Data)) / sizeof(WCHAR);

    ipc_xuser_get_token(1, req_url, req_method, req_url, req_body, req_body_len, token_buf, sizeof(token_buf), sig_buf, sizeof(sig_buf));

    if (MultiByteToWideChar(CP_UTF8, 0, token_buf, -1, token_w, (int)(max_wchars > 8192 ? 8192 : max_wchars)) == 0)
    {
        wcscpy(token_w, L"MOCK_XSTS_TOKEN");
    }

    sig_w = token_w + wcslen(token_w) + 1;
    if (sig_buf[0] != '\0')
    {
        MultiByteToWideChar(CP_UTF8, 0, sig_buf, -1, sig_w, (int)(max_wchars > (wcslen(token_w) + 1) ? (max_wchars - (wcslen(token_w) + 1)) : 0));
    }
    else
    {
        sig_w[0] = L'\0';
    }

    data->token = token_w;
    data->tokenCount = wcslen(token_w);
    if (sig_buf[0] != '\0')
    {
        data->signature = sig_w;
        data->signatureCount = wcslen(sig_w);
    }
    else
    {
        data->signature = NULL;
        data->signatureCount = 0;
    }

    if (strstr(req_url, "discovery.prod.athena"))
        TRACE( "[BEARD-DIAG][KIWIBEARD-DIAG] Delivering token for Athena Title Discovery (len: %zu, sig: %zu)\n", data->tokenCount, data->signatureCount );
    else if (strstr(req_url, "ares") || strstr(req_url, "athena"))
        TRACE( "[BEARD-DIAG][ERMINEBEARD-DIAG] Delivering token for Ares Authentication Handshake (len: %zu, sig: %zu)\n", data->tokenCount, data->signatureCount );
    else if (strstr(req_url, "eos") || strstr(req_url, "epicgames"))
        TRACE( "[BEARD-DIAG][HAZELBEARD-DIAG] Delivering token for EOS Crossplay Handshake (len: %zu, sig: %zu)\n", data->tokenCount, data->signatureCount );

    TRACE( "[GDK XUser] XUserGetTokenAndSignatureUtf16Result: tokenChars=%zu, sigChars=%zu for RP '%s'\n",
           data->tokenCount, data->signatureCount, req_url );

    if (ptrToBuffer) *ptrToBuffer = data;
    if (bufferUsed) *bufferUsed = sizeof(XUserGetTokenAndSignatureUtf16Data) + (data->tokenCount + 1 + (data->signature ? data->signatureCount + 1 : 0)) * sizeof(WCHAR);

    return S_OK;
}

static HRESULT WINAPI x_user_XUserResolveIssueWithUiAsync( IXUserImpl6 *iface, XUserHandle user, const char *url, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, url %s, async %p\n", iface, user, debugstr_a( url ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserResolveIssueWithUiResult( IXUserImpl6 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p\n", iface, async );
    return S_OK;
}

static HRESULT WINAPI x_user_XUserResolveIssueWithUiUtf16Async( IXUserImpl6 *iface, XUserHandle user, const WCHAR *url, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, url %s, async %p\n", iface, user, debugstr_w( url ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserResolveIssueWithUiUtf16Result( IXUserImpl6 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p\n", iface, async );
    return S_OK;
}

static HRESULT WINAPI x_user_XUserRegisterForChangeEvent( IXUserImpl6 *iface, XTaskQueueHandle queue, void *context, XUserChangeEventCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, context %p, callback %p, token %p\n", iface, queue, context, callback, token );
    if (g_user_change_listener_count < 16)
    {
        g_user_change_listeners[g_user_change_listener_count].queue = queue;
        g_user_change_listeners[g_user_change_listener_count].context = context;
        g_user_change_listeners[g_user_change_listener_count].callback = callback;
        g_user_change_listeners[g_user_change_listener_count].token = g_user_change_listener_count + 1;
        if (token) token->token = g_user_change_listener_count + 1;
        g_user_change_listener_count++;
    }
    return S_OK;
}

static BOOLEAN WINAPI x_user_XUserUnregisterForChangeEvent( IXUserImpl6 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    int i;
    TRACE( "iface %p, token %p, wait %d\n", iface, &token, wait );
    for (i = 0; i < g_user_change_listener_count; i++)
    {
        if (g_user_change_listeners[i].token == token.token)
        {
            g_user_change_listeners[i].callback = NULL;
            break;
        }
    }
    return TRUE;
}

static HRESULT WINAPI x_user_XUserGetSignOutDeferral( IXUserImpl6 *iface, XUserSignOutDeferralHandle *deferral )
{
    TRACE( "iface %p, deferral %p.\n", iface, deferral );
    *deferral = NULL;
    return E_GAMEUSER_DEFERRAL_NOT_AVAILABLE;
}

static void WINAPI x_user_XUserCloseSignOutDeferralHandle( IXUserImpl6 *iface, XUserSignOutDeferralHandle deferral )
{
    TRACE( "iface %p, deferral %p.\n", iface, deferral );
}

static HRESULT WINAPI x_user_XUserAddByIdWithUiAsync( IXUserImpl6 *iface, UINT64 userId, XAsyncBlock *async )
{
    TRACE( "iface %p, userId %llu, async %p\n", iface, userId, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserAddByIdWithUiResult( IXUserImpl6 *iface, XAsyncBlock *async, XUserHandle *newUser )
{
    TRACE( "iface %p, async %p, newUser %p\n", iface, async, newUser );
    if (newUser)
    {
        ensure_user_info();
        *newUser = (XUserHandle)&default_user_obj;
        return S_OK;
    }
    return E_POINTER;
}

static char g_last_msa_scope[512] = {0};

static HRESULT WINAPI x_user_XUserGetMsaTokenSilentlyAsync( IXUserImpl6 *iface, XUserHandle user, XUserGetMsaTokenSilentlyOptions options, const char *scope, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, options %u, scope %s, async %p\n", iface, user, options, debugstr_a( scope ), async );
    if (scope && scope[0]) {
        lstrcpynA(g_last_msa_scope, scope, sizeof(g_last_msa_scope));
    } else {
        g_last_msa_scope[0] = '\0';
    }
    ERR( "[XODUS-DIAG][ERMINEBEARD] XUserGetMsaTokenSilentlyAsync: options=0x%x, scope='%s'\n", options, scope ? scope : "(null)" );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetMsaTokenSilentlyResult( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T resultTokenSize, char *resultToken, SIZE_T *resultTokenUsed )
{
    const char *tok = "MOCK_MSA_TOKEN";
    SIZE_T len = strlen(tok) + 1;
    TRACE( "iface %p, async %p, resultTokenSize %Iu, resultToken %p, resultTokenUsed %p\n", iface, async, resultTokenSize, resultToken, resultTokenUsed );
    ERR( "[XODUS-DIAG][ERMINEBEARD] XUserGetMsaTokenSilentlyResult: returning MOCK for scope='%s' - NEEDS REAL MSA TOKEN\n", g_last_msa_scope );
    if (resultTokenUsed) *resultTokenUsed = len;
    if (resultToken && resultTokenSize >= len) strcpy(resultToken, tok);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetMsaTokenSilentlyResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *tokenSize )
{
    TRACE( "iface %p, async %p, tokenSize %p\n", iface, async, tokenSize );
    if (!tokenSize) return E_POINTER;
    *tokenSize = 8192;
    return S_OK;
}

static BOOLEAN WINAPI x_user_XUserIsStoreUser( IXUserImpl6 *iface, XUserHandle user )
{
    TRACE( "iface %p, user %p\n", iface, user );
    return TRUE;
}

static HRESULT WINAPI x_user_XUserPlatformRemoteConnectSetEventHandlers( IXUserImpl6 *iface, XTaskQueueHandle queue, XUserPlatformRemoteConnectEventHandlers *handlers )
{
    FIXME( "iface %p, queue %p, handlers %p stub!\n", iface, queue, handlers );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_XUserPlatformRemoteConnectCancelPrompt( IXUserImpl6 *iface, XUserPlatformOperation operation )
{
    FIXME( "iface %p, operation %p stub!\n", iface, operation );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_XUserPlatformSpopPromptSetEventHandlers( IXUserImpl6 *iface, XTaskQueueHandle queue, XUserPlatformSpopPromptEventHandler *handler, void *context )
{
    FIXME( "iface %p, queue %p, handler %p, context %p stub!\n", iface, queue, handler, context );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_XUserPlatformSpopPromptComplete( IXUserImpl6 *iface, XUserPlatformOperation operation, XUserPlatformOperationResult result )
{
    FIXME( "iface %p, operation %p, result %d stub!\n", iface, operation, result );
    return E_NOTIMPL;
}

static BOOLEAN WINAPI x_user_XUserIsSignOutPresent( IXUserImpl6 *iface )
{
    TRACE( "iface %p.\n", iface );
    return FALSE;
}

static HRESULT WINAPI x_user_XUserSignOutAsync( IXUserImpl6 *iface, XUserHandle user, XAsyncBlock *async )
{
    FIXME( "iface %p, user %p, async %p stub!\n", iface, user, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_XUserSignOutResult( IXUserImpl6 *iface, XAsyncBlock *async )
{
    FIXME( "iface %p, async %p stub!\n", iface, async );
    return E_NOTIMPL;
}

static const struct IXUserImpl6Vtbl x_user_vtbl =
{
    x_user_QueryInterface,
    x_user_AddRef,
    x_user_Release,
    /* IXUserImpl methods */
    x_user_XUserDuplicateHandle,
    x_user_XUserCloseHandle,
    x_user_XUserCompare,
    x_user_XUserGetMaxUsers,
    x_user_XUserAddAsync,
    x_user_XUserAddResult,
    x_user_XUserGetLocalId,
    x_user_XUserFindUserByLocalId,
    x_user_XUserGetId,
    x_user_XUserFindUserById,
    x_user_XUserGetIsGuest,
    x_user_XUserGetState,
    __PADDING__,
    x_user_XUserGetGamerPictureAsync,
    x_user_XUserGetGamerPictureResultSize,
    x_user_XUserGetGamerPictureResult,
    x_user_XUserGetAgeGroup,
    x_user_XUserCheckPrivilege,
    x_user_XUserResolvePrivilegeWithUiAsync,
    x_user_XUserResolvePrivilegeWithUiResult,
    x_user_XUserGetTokenAndSignatureAsync,
    x_user_XUserGetTokenAndSignatureResultSize,
    x_user_XUserGetTokenAndSignatureResult,
    x_user_XUserGetTokenAndSignatureUtf16Async,
    x_user_XUserGetTokenAndSignatureUtf16ResultSize,
    x_user_XUserGetTokenAndSignatureUtf16Result,
    x_user_XUserResolveIssueWithUiAsync,
    x_user_XUserResolveIssueWithUiResult,
    x_user_XUserResolveIssueWithUiUtf16Async,
    x_user_XUserResolveIssueWithUiUtf16Result,
    x_user_XUserRegisterForChangeEvent,
    x_user_XUserUnregisterForChangeEvent,
    x_user_XUserGetSignOutDeferral,
    x_user_XUserCloseSignOutDeferralHandle,
    /* IXUserImpl2 methods */
    x_user_XUserAddByIdWithUiAsync,
    x_user_XUserAddByIdWithUiResult,
    /* IXUserImpl3 methods */
    x_user_XUserGetMsaTokenSilentlyAsync,
    x_user_XUserGetMsaTokenSilentlyResult,
    x_user_XUserGetMsaTokenSilentlyResultSize,
    /* IXUserImpl4 methods */
    x_user_XUserIsStoreUser,
    /* IXUserImpl5 methods */
    x_user_XUserPlatformRemoteConnectSetEventHandlers,
    x_user_XUserPlatformRemoteConnectCancelPrompt,
    x_user_XUserPlatformSpopPromptSetEventHandlers,
    x_user_XUserPlatformSpopPromptComplete,
    /* IXUserImpl6 methods */
    x_user_XUserIsSignOutPresent,
    x_user_XUserSignOutAsync,
    x_user_XUserSignOutResult,
};

static inline struct x_user *impl_from_IXUserGamertagImpl( IXUserGamertagImpl *iface )
{
    return CONTAINING_RECORD( iface, struct x_user, IXUserGamertagImpl_iface );
}

static HRESULT WINAPI x_user_gamertag_QueryInterface( IXUserGamertagImpl *iface, REFIID riid, void **out )
{
    struct x_user *impl = impl_from_IXUserGamertagImpl( iface );
    return IXUserImpl6_QueryInterface( &impl->IXUserImpl6_iface, riid, out );
}

static ULONG WINAPI x_user_gamertag_AddRef( IXUserGamertagImpl *iface )
{
    struct x_user *impl = impl_from_IXUserGamertagImpl( iface );
    return IXUserImpl6_AddRef( &impl->IXUserImpl6_iface );
}

static ULONG WINAPI x_user_gamertag_Release( IXUserGamertagImpl *iface )
{
    struct x_user *impl = impl_from_IXUserGamertagImpl( iface );
    return IXUserImpl6_Release( &impl->IXUserImpl6_iface );
}

static HRESULT WINAPI x_user_gamertag_XUserGetGamertag( IXUserGamertagImpl *iface, XUserHandle user, XUserGamertagComponent gamertagComponent, SIZE_T gamertagSize, char *gamertag, SIZE_T *gamertagUsed )
{
    char tag[64] = {0};
    SIZE_T len;
    ensure_user_info();
    snprintf(tag, sizeof(tag), "%s", default_user_obj.gamertag);
    if (!tag[0]) strcpy(tag, "Player");
    len = strlen(tag) + 1;
    if (gamertagUsed) *gamertagUsed = len;
    if (!gamertag) return S_OK;
    if (gamertagSize < len) return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    strcpy(gamertag, tag);
    fprintf(stderr, "[GDK XUser] XUserGetGamertag -> '%s'\n", gamertag);
    return S_OK;
}


static const struct IXUserGamertagImplVtbl x_user_gamertag_vtbl =
{
    x_user_gamertag_QueryInterface,
    x_user_gamertag_AddRef,
    x_user_gamertag_Release,
    /* IXUserGamertag methods */
    x_user_gamertag_XUserGetGamertag,
};

static inline struct x_user *impl_from_IXUserDeviceImpl2( IXUserDeviceImpl2 *iface )
{
    return CONTAINING_RECORD( iface, struct x_user, IXUserDeviceImpl2_iface );
}

static HRESULT WINAPI x_user_device_QueryInterface( IXUserDeviceImpl2 *iface, REFIID iid, void **out )
{
    struct x_user *impl = impl_from_IXUserDeviceImpl2( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown          ) ||
        IsEqualGUID( iid, &IID_IXUserDeviceImpl  ) ||
        IsEqualGUID( iid, &IID_IXUserDeviceImpl2 ))
    {
        IXUserDeviceImpl2_AddRef( *out = &impl->IXUserDeviceImpl2_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_user_device_AddRef( IXUserDeviceImpl2 *iface )
{
    struct x_user *impl = impl_from_IXUserDeviceImpl2( iface );
    return IXUserImpl6_AddRef( &impl->IXUserImpl6_iface );
}

static ULONG WINAPI x_user_device_Release( IXUserDeviceImpl2 *iface )
{
    struct x_user *impl = impl_from_IXUserDeviceImpl2( iface );
    return IXUserImpl6_Release( &impl->IXUserImpl6_iface );
}

static HRESULT WINAPI x_user_device_XUserFindForDevice( IXUserDeviceImpl2 *iface, const APP_LOCAL_DEVICE_ID *deviceId, XUserHandle *handle )
{
    TRACE( "iface %p, deviceId %p, handle %p\n", iface, deviceId, handle );
    if (!handle) return E_POINTER;
    ensure_user_info();
    *handle = (XUserHandle)&default_user_obj;
    return S_OK;
}

static HRESULT WINAPI x_user_device_XUserRegisterForDeviceAssociationChanged( IXUserDeviceImpl2 *iface, XTaskQueueHandle queue, void *context, XUserDeviceAssociationChangedCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, context %p, callback %p, token %p\n", iface, queue, context, callback, token );
    if (token) token->token = 1;
    return S_OK;
}

static BOOLEAN WINAPI x_user_device_XUserUnregisterForDeviceAssociationChanged( IXUserDeviceImpl2 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    TRACE( "iface %p, token %p, wait %d\n", iface, &token, wait );
    return TRUE;
}

static HRESULT WINAPI x_user_device_XUserGetDefaultAudioEndpointUtf16( IXUserDeviceImpl2 *iface, XUserLocalId user, XUserDefaultAudioEndpointKind defaultAudioEndpointKind, SIZE_T endpointIdUtf16Count, WCHAR *endpointIdUtf16, SIZE_T *endpointIdUtf16Used )
{
    TRACE( "iface %p, user %llu, defaultAudioEndpointKind %d, endpointIdUtf16Count %Iu, endpointIdUtf16 %p, endpointIdUtf16Used %p\n",
           iface, (unsigned long long)user.value, defaultAudioEndpointKind, endpointIdUtf16Count, endpointIdUtf16, endpointIdUtf16Used );
    if (endpointIdUtf16Used) *endpointIdUtf16Used = 1;
    if (endpointIdUtf16 && endpointIdUtf16Count > 0) endpointIdUtf16[0] = L'\0';
    return S_OK;
}

static HRESULT WINAPI x_user_device_XUserRegisterForDefaultAudioEndpointUtf16Changed( IXUserDeviceImpl2 *iface, XTaskQueueHandle queue, void *context, XUserDefaultAudioEndpointUtf16ChangedCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, context %p, callback %p, token %p\n", iface, queue, context, callback, token );
    if (token) token->token = 1;
    return S_OK;
}

static BOOLEAN WINAPI x_user_device_XUserUnregisterForDefaultAudioEndpointUtf16Changed( IXUserDeviceImpl2 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    TRACE( "iface %p, token %p, wait %d\n", iface, &token, wait );
    return TRUE;
}

static HRESULT WINAPI x_user_device_XUserFindControllerForUserWithUiAsync( IXUserDeviceImpl2 *iface, XUserHandle user, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, async %p\n", iface, user, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_device_XUserFindControllerForUserWithUiResult( IXUserDeviceImpl2 *iface, XAsyncBlock *async, APP_LOCAL_DEVICE_ID *deviceId )
{
    TRACE( "iface %p, async %p, deviceId %p\n", iface, async, deviceId );
    if (!deviceId) return E_POINTER;
    memset(deviceId, 0, sizeof(*deviceId));
    return S_OK;
}

static const struct IXUserDeviceImpl2Vtbl x_user_device_vtbl =
{
    x_user_device_QueryInterface,
    x_user_device_AddRef,
    x_user_device_Release,
    /* IXUserDeviceImpl/IXUserDeviceImpl2 methods */
    x_user_device_XUserFindForDevice,
    x_user_device_XUserRegisterForDeviceAssociationChanged,
    x_user_device_XUserUnregisterForDeviceAssociationChanged,
    x_user_device_XUserGetDefaultAudioEndpointUtf16,
    x_user_device_XUserRegisterForDefaultAudioEndpointUtf16Changed,
    x_user_device_XUserUnregisterForDefaultAudioEndpointUtf16Changed,
    x_user_device_XUserFindControllerForUserWithUiAsync,
    x_user_device_XUserFindControllerForUserWithUiResult,
};

static struct x_user x_user =
{
    {&x_user_vtbl},
    {&x_user_gamertag_vtbl},
    {&x_user_device_vtbl},
    0,
};

IXUserImpl *x_user_impl = (IXUserImpl *)&x_user.IXUserImpl6_iface;
IXUserDeviceImpl *x_user_device_impl = (IXUserDeviceImpl *)&x_user.IXUserDeviceImpl2_iface;
