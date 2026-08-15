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
                default_user_obj.xuid = strtoull(info.xuid, NULL, 10);
                if (default_user_obj.xuid == 0)
                    default_user_obj.xuid = strtoull(info.xuid, NULL, 16);
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

static HRESULT WINAPI x_user_XUserAddAsync( IXUserImpl6 *iface, XUserAddOptions options, XAsyncBlock *async )
{
    fprintf(stderr, "[GDK XUser] XUserAddAsync called: options=0x%x, async=%p\n", options, async);
    ensure_user_info();
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserAddResult( IXUserImpl6 *iface, XAsyncBlock *async, XUserHandle *newUser )
{
    ensure_user_info();
    fprintf(stderr, "[GDK XUser] XUserAddResult called: async=%p, newUser=%p\n", async, newUser);
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

static char last_relying_party[512] = "http://xboxlive.com";

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureAsync( IXUserImpl6 *iface, XUserHandle user, XUserGetTokenAndSignatureOptions options, const char *method, const char *url, SIZE_T headerCount, const XUserGetTokenAndSignatureHttpHeader *headers, SIZE_T bodySize, const void *bodyBuffer, XAsyncBlock *async )
{
    ensure_user_info();
    if (url && url[0])
    {
        snprintf(last_relying_party, sizeof(last_relying_party), "%s", url);
    }
    fprintf(stderr, "[GDK XUser] XUserGetTokenAndSignatureAsync: options=0x%x, method='%s', url='%s', RP='%s', async=%p\n",
            options, method ? method : "", url ? url : "", last_relying_party, async);
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *bufferSize )
{
    if (bufferSize)
    {
        *bufferSize = sizeof(XUserGetTokenAndSignatureData) + 16384;
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureResult( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T bufferSize, void *buffer, XUserGetTokenAndSignatureData **ptrToBuffer, SIZE_T *bufferUsed )
{
    XUserGetTokenAndSignatureData *data;
    char *token_ptr;
    char *sig_ptr;
    SIZE_T max_token_len;
    SIZE_T max_sig_len = 1024;

    if (!buffer || bufferSize < sizeof(XUserGetTokenAndSignatureData) + 512)
        return E_INVALIDARG;

    data = (XUserGetTokenAndSignatureData *)buffer;
    token_ptr = (char *)buffer + sizeof(XUserGetTokenAndSignatureData);
    max_token_len = bufferSize - sizeof(XUserGetTokenAndSignatureData) - max_sig_len;
    sig_ptr = token_ptr + max_token_len;

    data->token = token_ptr;
    data->signature = sig_ptr;

    ipc_xuser_get_token(1, last_relying_party, (char *)data->token, max_token_len, (char *)data->signature, max_sig_len);

    data->tokenSize = strlen(data->token) + 1;
    data->signatureSize = strlen(data->signature) + 1;

    fprintf(stderr, "[GDK XUser] XUserGetTokenAndSignatureResult: token len=%zu, sig len=%zu for RP '%s'\n",
            data->tokenSize, data->signatureSize, last_relying_party);

    if (ptrToBuffer) *ptrToBuffer = data;
    if (bufferUsed) *bufferUsed = sizeof(XUserGetTokenAndSignatureData) + data->tokenSize + data->signatureSize;

    return S_OK;
}


static HRESULT WINAPI x_user_XUserGetTokenAndSignatureUtf16Async( IXUserImpl6 *iface, XUserHandle user, XUserGetTokenAndSignatureOptions options, const WCHAR *method, const WCHAR *url, SIZE_T headerCount, const XUserGetTokenAndSignatureUtf16HttpHeader *headers, SIZE_T bodySize, const void *bodyBuffer, XAsyncBlock *async )
{
    char method_a[64] = {0};
    ensure_user_info();
    if (method && method[0]) WideCharToMultiByte(CP_UTF8, 0, method, -1, method_a, sizeof(method_a) - 1, NULL, NULL);
    if (url && url[0])
    {
        WideCharToMultiByte(CP_UTF8, 0, url, -1, last_relying_party, sizeof(last_relying_party) - 1, NULL, NULL);
        last_relying_party[sizeof(last_relying_party) - 1] = '\0';
    }
    TRACE( "[GDK XUser] XUserGetTokenAndSignatureUtf16Async: options=0x%x, method='%s', url='%s', async=%p\n",
           options, method_a, last_relying_party, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureUtf16ResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *bufferSize )
{
    if (!bufferSize) return E_POINTER;
    *bufferSize = sizeof(XUserGetTokenAndSignatureUtf16Data) + 8192 * sizeof(WCHAR);
    TRACE( "[GDK XUser] XUserGetTokenAndSignatureUtf16ResultSize: returning bufferSize=%zu\n", *bufferSize );
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetTokenAndSignatureUtf16Result( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T bufferSize, void *buffer, XUserGetTokenAndSignatureUtf16Data **ptrToBuffer, SIZE_T *bufferUsed )
{
    char token_buf[8192] = {0};
    char sig_buf[1024] = {0};
    XUserGetTokenAndSignatureUtf16Data *data;
    WCHAR *token_w;
    WCHAR *sig_w;
    SIZE_T max_wchars;
    SIZE_T token_count;
    SIZE_T sig_count;

    if (!buffer || bufferSize < sizeof(XUserGetTokenAndSignatureUtf16Data) + 256 * sizeof(WCHAR))
        return E_INVALIDARG;

    data = (XUserGetTokenAndSignatureUtf16Data *)buffer;
    token_w = (WCHAR *)((BYTE *)buffer + sizeof(XUserGetTokenAndSignatureUtf16Data));
    max_wchars = (bufferSize - sizeof(XUserGetTokenAndSignatureUtf16Data)) / sizeof(WCHAR);

    ipc_xuser_get_token(1, last_relying_party, token_buf, sizeof(token_buf), sig_buf, sizeof(sig_buf));

    token_count = (SIZE_T)MultiByteToWideChar(CP_UTF8, 0, token_buf, -1, token_w, (int)(max_wchars > 8192 ? 8192 : max_wchars));
    if (token_count == 0)
    {
        wcscpy(token_w, L"MOCK_XSTS_TOKEN");
        token_count = wcslen(token_w) + 1;
    }

    sig_w = token_w + token_count;
    sig_count = (SIZE_T)MultiByteToWideChar(CP_UTF8, 0, sig_buf, -1, sig_w, (int)(max_wchars > token_count ? (max_wchars - token_count) : 0));
    if (sig_count == 0)
    {
        sig_w[0] = L'\0';
        sig_count = 1;
    }

    data->token = token_w;
    data->signature = sig_w;
    data->tokenCount = token_count;
    data->signatureCount = sig_count;

    TRACE( "[GDK XUser] XUserGetTokenAndSignatureUtf16Result: tokenCount=%zu, sigCount=%zu for RP '%s'\n",
           token_count, sig_count, last_relying_party );

    if (ptrToBuffer) *ptrToBuffer = data;
    if (bufferUsed) *bufferUsed = sizeof(XUserGetTokenAndSignatureUtf16Data) + (token_count + sig_count) * sizeof(WCHAR);

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
    if (token) token->token = 1;
    return S_OK;
}

static BOOLEAN WINAPI x_user_XUserUnregisterForChangeEvent( IXUserImpl6 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    TRACE( "iface %p, token %p, wait %d\n", iface, &token, wait );
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
    if (newUser) *newUser = (XUserHandle)(UINT_PTR)1;
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetMsaTokenSilentlyAsync( IXUserImpl6 *iface, XUserHandle user, XUserGetMsaTokenSilentlyOptions options, const char *scope, XAsyncBlock *async )
{
    TRACE( "iface %p, user %p, options %u, scope %s, async %p\n", iface, user, options, debugstr_a( scope ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetMsaTokenSilentlyResult( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T resultTokenSize, char *resultToken, SIZE_T *resultTokenUsed )
{
    const char *tok = "MOCK_MSA_TOKEN";
    SIZE_T len = strlen(tok) + 1;
    TRACE( "iface %p, async %p, resultTokenSize %Iu, resultToken %p, resultTokenUsed %p\n", iface, async, resultTokenSize, resultToken, resultTokenUsed );
    if (resultTokenUsed) *resultTokenUsed = len;
    if (resultToken && resultTokenSize >= len) strcpy(resultToken, tok);
    return S_OK;
}

static HRESULT WINAPI x_user_XUserGetMsaTokenSilentlyResultSize( IXUserImpl6 *iface, XAsyncBlock *async, SIZE_T *tokenSize )
{
    TRACE( "iface %p, async %p, tokenSize %p\n", iface, async, tokenSize );
    if (!tokenSize) return E_POINTER;
    *tokenSize = 32;
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
    FIXME( "iface %p, deviceId %p, handle %p stub!\n", iface, deviceId, handle );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_device_XUserRegisterForDeviceAssociationChanged( IXUserDeviceImpl2 *iface, XTaskQueueHandle queue, void *context, XUserDeviceAssociationChangedCallback *callback, XTaskQueueRegistrationToken *token )
{
    FIXME( "iface %p, queue %p, context %p, callback %p, token %p stub!\n", iface, queue, context, callback, token );
    return E_NOTIMPL;
}

static BOOLEAN WINAPI x_user_device_XUserUnregisterForDeviceAssociationChanged( IXUserDeviceImpl2 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    FIXME( "iface %p, token %p, wait %d stub!\n", iface, &token, wait );
    return FALSE;
}

static HRESULT WINAPI x_user_device_XUserGetDefaultAudioEndpointUtf16( IXUserDeviceImpl2 *iface, XUserLocalId user, XUserDefaultAudioEndpointKind defaultAudioEndpointKind, SIZE_T endpointIdUtf16Count, WCHAR *endpointIdUtf16, SIZE_T *endpointIdUtf16Used )
{
    FIXME( "iface %p, user %p, defaultAudioEndpointKind %d, endpointIdUtf16Count %Iu, endpointIdUtf16 %p, endpointIdUtf16Used %p stub!\n", iface, &user, defaultAudioEndpointKind, endpointIdUtf16Count, endpointIdUtf16, endpointIdUtf16Used );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_device_XUserRegisterForDefaultAudioEndpointUtf16Changed( IXUserDeviceImpl2 *iface, XTaskQueueHandle queue, void *context, XUserDefaultAudioEndpointUtf16ChangedCallback *callback, XTaskQueueRegistrationToken *token )
{
    FIXME( "iface %p, queue %p, context %p, callback %p, token %p stub!\n", iface, queue, context, callback, token );
    return E_NOTIMPL;
}

static BOOLEAN WINAPI x_user_device_XUserUnregisterForDefaultAudioEndpointUtf16Changed( IXUserDeviceImpl2 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    FIXME( "iface %p, token %p, wait %d stub!\n", iface, &token, wait );
    return FALSE;
}

static HRESULT WINAPI x_user_device_XUserFindControllerForUserWithUiAsync( IXUserDeviceImpl2 *iface, XUserHandle user, XAsyncBlock *async )
{
    FIXME( "iface %p, user %p, async %p stub!\n", iface, user, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_user_device_XUserFindControllerForUserWithUiResult( IXUserDeviceImpl2 *iface, XAsyncBlock *async, APP_LOCAL_DEVICE_ID *deviceId )
{
    FIXME( "iface %p, async %p, deviceId %p stub!\n", iface, async, deviceId );
    return E_NOTIMPL;
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
