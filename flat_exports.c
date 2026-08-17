/*
 * Xbox Game Runtime - Flat C Exports
 *
 * Implements flat C exported API entrypoints for Unity / IL2CPP / PInvoke / C# GameCore wrappers
 * and dynamically loaded GDK applications.
 */

#include "private.h"
#include <wchar.h>

WINE_DEFAULT_DEBUG_CHANNEL(xgameruntime);

/* =========================================================================
 * 1. Runtime Initialization & Feature Query
 * ========================================================================= */

HRESULT WINAPI XGameRuntimeInitialize(void)
{
    fprintf(stderr, "[GDK Flat Export] XGameRuntimeInitialize called\n");
    return InitializeApiImpl( 0, 0 );
}

HRESULT WINAPI XGameRuntimeInitializeWithOptions( const struct initialize_options *options )
{
    fprintf(stderr, "[GDK Flat Export] XGameRuntimeInitializeWithOptions called: options=%p\n", options);
    return InitializeApiImplEx2( 0, 0, 0, options );
}

void WINAPI XGameRuntimeUninitialize(void)
{
    fprintf(stderr, "[GDK Flat Export] XGameRuntimeUninitialize called\n");
    UninitializeApiImpl();
}

BOOLEAN WINAPI XGameRuntimeIsFeatureAvailable( XGameRuntimeFeature feature )
{
    TRACE("(%d)\n", feature);
    if (!x_game_runtime_feature_impl) return TRUE;
    return IXGameRuntimeFeatureImpl_XGameRuntimeIsFeatureAvailable( x_game_runtime_feature_impl, feature );
}

HRESULT WINAPI XGameGetXboxTitleId( UINT32 *titleId )
{
    TRACE("(%p)\n", titleId);
    if (!x_game_impl) return E_FAIL;
    return IXGameImpl3_XGameGetXboxTitleId( (IXGameImpl3 *)x_game_impl, titleId );
}

/* =========================================================================
 * 2. XUser Management & Authentication
 * ========================================================================= */

HRESULT WINAPI XUserAddAsync( XUserAddOptions options, XAsyncBlock *async )
{
    TRACE("(0x%x, %p)\n", options, async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserAddAsync( (IXUserImpl6 *)x_user_impl, options, async );
}

HRESULT WINAPI XUserAddResult( XAsyncBlock *async, XUserHandle *user )
{
    TRACE("(%p, %p)\n", async, user);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserAddResult( (IXUserImpl6 *)x_user_impl, async, user );
}

HRESULT WINAPI XUserAddByIdWithUiAsync( UINT32 userId, XAsyncBlock *async )
{
    TRACE("(%u, %p)\n", userId, async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserAddByIdWithUiAsync( (IXUserImpl6 *)x_user_impl, userId, async );
}

HRESULT WINAPI XUserAddByIdWithUiResult( XAsyncBlock *async, XUserHandle *user )
{
    TRACE("(%p, %p)\n", async, user);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserAddByIdWithUiResult( (IXUserImpl6 *)x_user_impl, async, user );
}

void WINAPI XUserCloseHandle( XUserHandle user )
{
    TRACE("(%p)\n", user);
    if (x_user_impl)
        IXUserImpl6_XUserCloseHandle( (IXUserImpl6 *)x_user_impl, user );
}

HRESULT WINAPI XUserDuplicateHandle( XUserHandle user, XUserHandle *duplicatedUserHandle )
{
    TRACE("(%p, %p)\n", user, duplicatedUserHandle);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserDuplicateHandle( (IXUserImpl6 *)x_user_impl, user, duplicatedUserHandle );
}

HRESULT WINAPI XUserCompare( XUserHandle user1, XUserHandle user2, INT32 *comparisonResult )
{
    TRACE("(%p, %p, %p)\n", user1, user2, comparisonResult);
    if (!x_user_impl) return E_FAIL;
    if (comparisonResult)
        *comparisonResult = IXUserImpl6_XUserCompare( (IXUserImpl6 *)x_user_impl, user1, user2 );
    return S_OK;
}

HRESULT WINAPI XUserGetId( XUserHandle user, UINT64 *userId )
{
    TRACE("(%p, %p)\n", user, userId);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetId( (IXUserImpl6 *)x_user_impl, user, userId );
}

HRESULT WINAPI XUserGetLocalId( XUserHandle user, XUserLocalId *userLocalId )
{
    TRACE("(%p, %p)\n", user, userLocalId);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetLocalId( (IXUserImpl6 *)x_user_impl, user, userLocalId );
}

HRESULT WINAPI XUserGetState( XUserHandle user, XUserState *state )
{
    TRACE("(%p, %p)\n", user, state);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetState( (IXUserImpl6 *)x_user_impl, user, state );
}

HRESULT WINAPI XUserGetIsGuest( XUserHandle user, bool *isGuest )
{
    BOOLEAN guest = FALSE;
    HRESULT hr;
    TRACE("(%p, %p)\n", user, isGuest);
    if (!x_user_impl) return E_FAIL;
    hr = IXUserImpl6_XUserGetIsGuest( (IXUserImpl6 *)x_user_impl, user, &guest );
    if (isGuest) *isGuest = guest;
    return hr;
}

HRESULT WINAPI XUserGetAgeGroup( XUserHandle user, XUserAgeGroup *ageGroup )
{
    TRACE("(%p, %p)\n", user, ageGroup);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetAgeGroup( (IXUserImpl6 *)x_user_impl, user, ageGroup );
}

HRESULT WINAPI XUserGetGamertag( XUserHandle user, XUserGamertagComponent gamertagComponent, SIZE_T gamertagSize, char *gamertag, SIZE_T *gamertagUsed )
{
    void *factory = NULL;
    TRACE("(%p, %d, %zu, %p, %p)\n", user, gamertagComponent, gamertagSize, gamertag, gamertagUsed);
    if (SUCCEEDED(QueryApiImpl(&CLSID_XUserImpl, &IID_IXUserGamertagImpl, &factory)) && factory)
    {
        return IXUserGamertagImpl_XUserGetGamertag( (IXUserGamertagImpl *)factory, user, gamertagComponent, gamertagSize, gamertag, gamertagUsed );
    }
    return E_FAIL;
}

HRESULT WINAPI XUserGetGamerPictureAsync( XUserHandle user, XUserGamerPictureSize pictureSize, XAsyncBlock *async )
{
    TRACE("(%p, %d, %p)\n", user, pictureSize, async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetGamerPictureAsync( (IXUserImpl6 *)x_user_impl, user, pictureSize, async );
}

HRESULT WINAPI XUserGetGamerPictureResultSize( XAsyncBlock *async, SIZE_T *bufferSize )
{
    TRACE("(%p, %p)\n", async, bufferSize);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetGamerPictureResultSize( (IXUserImpl6 *)x_user_impl, async, bufferSize );
}

HRESULT WINAPI XUserGetGamerPictureResult( XAsyncBlock *async, SIZE_T bufferSize, void *buffer, SIZE_T *bufferUsed )
{
    TRACE("(%p, %zu, %p, %p)\n", async, bufferSize, buffer, bufferUsed);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetGamerPictureResult( (IXUserImpl6 *)x_user_impl, async, bufferSize, buffer, bufferUsed );
}

HRESULT WINAPI XUserGetTokenAndSignatureAsync( XUserHandle user, XUserGetTokenAndSignatureOptions options, const char *method, const char *url, SIZE_T headerCount, const XUserGetTokenAndSignatureHttpHeader *headers, SIZE_T bodySize, const void *body, XAsyncBlock *async )
{
    TRACE("(%p, %u, %s, %s, %zu, %p, %zu, %p, %p)\n", user, options, method, url, headerCount, headers, bodySize, body, async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetTokenAndSignatureAsync( (IXUserImpl6 *)x_user_impl, user, options, method, url, headerCount, (const XUserGetTokenAndSignatureHttpHeader *)headers, bodySize, body, async );
}

HRESULT WINAPI XUserGetTokenAndSignatureResultSize( XAsyncBlock *async, SIZE_T *bufferSize )
{
    TRACE("(%p, %p)\n", async, bufferSize);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetTokenAndSignatureResultSize( (IXUserImpl6 *)x_user_impl, async, bufferSize );
}

HRESULT WINAPI XUserGetTokenAndSignatureResult( XAsyncBlock *async, SIZE_T bufferSize, void *buffer, XUserGetTokenAndSignatureData **ptrToBuffer, SIZE_T *bufferUsed )
{
    TRACE("(%p, %zu, %p, %p, %p)\n", async, bufferSize, buffer, ptrToBuffer, bufferUsed);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetTokenAndSignatureResult( (IXUserImpl6 *)x_user_impl, async, bufferSize, buffer, ptrToBuffer, bufferUsed );
}

HRESULT WINAPI XUserGetTokenAndSignatureUtf16Async( XUserHandle user, XUserGetTokenAndSignatureOptions options, const WCHAR *method, const WCHAR *url, SIZE_T headerCount, const XUserGetTokenAndSignatureUtf16HttpHeader *headers, SIZE_T bodySize, const void *body, XAsyncBlock *async )
{
    TRACE("(%p, %u, %p, %p, %zu, %p, %zu, %p, %p)\n", user, options, method, url, headerCount, headers, bodySize, body, async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetTokenAndSignatureUtf16Async( (IXUserImpl6 *)x_user_impl, user, options, method, url, headerCount, (const XUserGetTokenAndSignatureUtf16HttpHeader *)headers, bodySize, body, async );
}

HRESULT WINAPI XUserGetTokenAndSignatureUtf16ResultSize( XAsyncBlock *async, SIZE_T *bufferSize )
{
    TRACE("(%p, %p)\n", async, bufferSize);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetTokenAndSignatureUtf16ResultSize( (IXUserImpl6 *)x_user_impl, async, bufferSize );
}

HRESULT WINAPI XUserGetTokenAndSignatureUtf16Result( XAsyncBlock *async, SIZE_T bufferSize, void *buffer, XUserGetTokenAndSignatureUtf16Data **ptrToBuffer, SIZE_T *bufferUsed )
{
    TRACE("(%p, %zu, %p, %p, %p)\n", async, bufferSize, buffer, ptrToBuffer, bufferUsed);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetTokenAndSignatureUtf16Result( (IXUserImpl6 *)x_user_impl, async, bufferSize, buffer, ptrToBuffer, bufferUsed );
}

HRESULT WINAPI XUserCheckPrivilege( XUserHandle user, XUserPrivilegeOptions options, XUserPrivilege privilege, bool *hasPrivilege, XUserPrivilegeDenyReason *reason )
{
    BOOLEAN hasPriv = TRUE;
    HRESULT hr;
    TRACE("(%p, 0x%x, %d, %p, %p)\n", user, options, privilege, hasPrivilege, reason);
    if (!x_user_impl) return E_FAIL;
    hr = IXUserImpl6_XUserCheckPrivilege( (IXUserImpl6 *)x_user_impl, user, options, privilege, &hasPriv, reason );
    if (hasPrivilege) *hasPrivilege = hasPriv;
    return hr;
}

HRESULT WINAPI XUserResolvePrivilegeWithUiAsync( XUserHandle user, XUserPrivilegeOptions options, XUserPrivilege privilege, XAsyncBlock *async )
{
    TRACE("(%p, 0x%x, %d, %p)\n", user, options, privilege, async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserResolvePrivilegeWithUiAsync( (IXUserImpl6 *)x_user_impl, user, options, privilege, async );
}

HRESULT WINAPI XUserResolvePrivilegeWithUiResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserResolvePrivilegeWithUiResult( (IXUserImpl6 *)x_user_impl, async );
}

HRESULT WINAPI XUserRegisterForChangeEvent( XTaskQueueHandle queue, void *context, XUserChangeEventCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE("(%p, %p, %p, %p)\n", queue, context, callback, token);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserRegisterForChangeEvent( (IXUserImpl6 *)x_user_impl, queue, context, callback, token );
}

bool WINAPI XUserUnregisterForChangeEvent( XTaskQueueRegistrationToken token, bool wait )
{
    TRACE("(%p, %d)\n", &token, wait);
    if (!x_user_impl) return FALSE;
    return IXUserImpl6_XUserUnregisterForChangeEvent( (IXUserImpl6 *)x_user_impl, token, wait ? TRUE : FALSE );
}

HRESULT WINAPI XUserFindUserById( UINT64 userId, XUserHandle *handle )
{
    TRACE("(%lu, %p)\n", userId, handle);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserFindUserById( (IXUserImpl6 *)x_user_impl, userId, handle );
}

HRESULT WINAPI XUserFindUserByLocalId( XUserLocalId userLocalId, XUserHandle *handle )
{
    TRACE("(%lu, %p)\n", userLocalId.value, handle);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserFindUserByLocalId( (IXUserImpl6 *)x_user_impl, userLocalId, handle );
}

HRESULT WINAPI XUserGetMaxUsers( UINT32 *maxUsers )
{
    TRACE("(%p)\n", maxUsers);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetMaxUsers( (IXUserImpl6 *)x_user_impl, maxUsers );
}

bool WINAPI XUserIsStoreUser( XUserHandle user )
{
    TRACE("(%p)\n", user);
    if (!x_user_impl) return TRUE;
    return IXUserImpl6_XUserIsStoreUser( (IXUserImpl6 *)x_user_impl, user );
}

HRESULT WINAPI XUserGetSignOutDeferral( XUserSignOutDeferralHandle *deferral )
{
    TRACE("(%p)\n", deferral);
    if (!x_user_impl) return E_FAIL;
    return IXUserImpl6_XUserGetSignOutDeferral( (IXUserImpl6 *)x_user_impl, deferral );
}

void WINAPI XUserCloseSignOutDeferralHandle( XUserSignOutDeferralHandle deferral )
{
    TRACE("(%p)\n", deferral);
    if (x_user_impl)
        IXUserImpl6_XUserCloseSignOutDeferralHandle( (IXUserImpl6 *)x_user_impl, deferral );
}

/* =========================================================================
 * 3. XTaskQueue & XAsync
 * ========================================================================= */

HRESULT WINAPI XTaskQueueCreate( XTaskQueueDispatchMode workDispatchMode, XTaskQueueDispatchMode completionDispatchMode, XTaskQueueHandle *queue )
{
    TRACE("(%d, %d, %p)\n", workDispatchMode, completionDispatchMode, queue);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueCreate( x_threading_impl, workDispatchMode, completionDispatchMode, queue );
}

HRESULT WINAPI XTaskQueueCreateComposite( XTaskQueuePortHandle workPort, XTaskQueuePortHandle completionPort, XTaskQueueHandle *queue )
{
    TRACE("(%p, %p, %p)\n", workPort, completionPort, queue);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueCreateComposite( x_threading_impl, workPort, completionPort, queue );
}

bool WINAPI XTaskQueueDispatch( XTaskQueueHandle queue, XTaskQueuePort port, UINT32 timeoutInMs )
{
    TRACE("(%p, %d, %u)\n", queue, port, timeoutInMs);
    if (!x_threading_impl) return FALSE;
    return IXThreadingImpl_XTaskQueueDispatch( x_threading_impl, queue, port, timeoutInMs );
}

void WINAPI XTaskQueueCloseHandle( XTaskQueueHandle queue )
{
    TRACE("(%p)\n", queue);
    if (x_threading_impl)
        IXThreadingImpl_XTaskQueueCloseHandle( x_threading_impl, queue );
}

HRESULT WINAPI XTaskQueueDuplicateHandle( XTaskQueueHandle queue, XTaskQueueHandle *duplicatedHandle )
{
    TRACE("(%p, %p)\n", queue, duplicatedHandle);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueDuplicateHandle( x_threading_impl, queue, duplicatedHandle );
}

HRESULT WINAPI XTaskQueueGetCurrentProcessTaskQueue( XTaskQueueHandle *queue )
{
    TRACE("(%p)\n", queue);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueGetCurrentProcessTaskQueue( x_threading_impl, queue );
}

void WINAPI XTaskQueueSetCurrentProcessTaskQueue( XTaskQueueHandle queue )
{
    TRACE("(%p)\n", queue);
    if (x_threading_impl)
        IXThreadingImpl_XTaskQueueSetCurrentProcessTaskQueue( x_threading_impl, queue );
}

HRESULT WINAPI XTaskQueueGetPort( XTaskQueueHandle queue, XTaskQueuePort port, XTaskQueuePortHandle *portHandle )
{
    TRACE("(%p, %d, %p)\n", queue, port, portHandle);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueGetPort( x_threading_impl, queue, port, portHandle );
}

HRESULT WINAPI XTaskQueueRegisterMonitor( XTaskQueueHandle queue, void *context, XTaskQueueMonitorCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE("(%p, %p, %p, %p)\n", queue, context, callback, token);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueRegisterMonitor( x_threading_impl, queue, context, callback, token );
}

void WINAPI XTaskQueueUnregisterMonitor( XTaskQueueHandle queue, XTaskQueueRegistrationToken token )
{
    TRACE("(%p, %p)\n", queue, &token);
    if (x_threading_impl)
        IXThreadingImpl_XTaskQueueUnregisterMonitor( x_threading_impl, queue, token );
}

HRESULT WINAPI XTaskQueueRegisterWaiter( XTaskQueueHandle queue, XTaskQueuePort port, HANDLE waitHandle, void *context, XTaskQueueCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE("(%p, %d, %p, %p, %p, %p)\n", queue, port, waitHandle, context, callback, token);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueRegisterWaiter( x_threading_impl, queue, port, waitHandle, context, callback, token );
}

void WINAPI XTaskQueueUnregisterWaiter( XTaskQueueHandle queue, XTaskQueueRegistrationToken token )
{
    TRACE("(%p, %p)\n", queue, &token);
    if (x_threading_impl)
        IXThreadingImpl_XTaskQueueUnregisterWaiter( x_threading_impl, queue, token );
}

HRESULT WINAPI XTaskQueueSubmitCallback( XTaskQueueHandle queue, XTaskQueuePort port, void *context, XTaskQueueCallback *callback )
{
    TRACE("(%p, %d, %p, %p)\n", queue, port, context, callback);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueSubmitCallback( x_threading_impl, queue, port, context, callback );
}

HRESULT WINAPI XTaskQueueSubmitDelayedCallback( XTaskQueueHandle queue, XTaskQueuePort port, UINT32 delayInMs, void *context, XTaskQueueCallback *callback )
{
    TRACE("(%p, %d, %u, %p, %p)\n", queue, port, delayInMs, context, callback);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueSubmitDelayedCallback( x_threading_impl, queue, port, delayInMs, context, callback );
}

HRESULT WINAPI XTaskQueueTerminate( XTaskQueueHandle queue, bool wait, void *context, XTaskQueueTerminatedCallback *callback )
{
    TRACE("(%p, %d, %p, %p)\n", queue, wait, context, callback);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XTaskQueueTerminate( x_threading_impl, queue, wait ? TRUE : FALSE, context, callback );
}

HRESULT WINAPI XAsyncGetStatus( XAsyncBlock *async, bool wait )
{
    TRACE("(%p, %d)\n", async, wait);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XAsyncGetStatus( x_threading_impl, async, wait ? TRUE : FALSE );
}

HRESULT WINAPI XAsyncGetResult( XAsyncBlock *async, const void *identity, SIZE_T bufferSize, void *buffer, SIZE_T *bufferUsed )
{
    TRACE("(%p, %p, %zu, %p, %p)\n", async, identity, bufferSize, buffer, bufferUsed);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XAsyncGetResult( x_threading_impl, async, identity, bufferSize, buffer, bufferUsed );
}

HRESULT WINAPI XAsyncGetResultSize( XAsyncBlock *async, SIZE_T *bufferSize )
{
    TRACE("(%p, %p)\n", async, bufferSize);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XAsyncGetResultSize( x_threading_impl, async, bufferSize );
}

void WINAPI XAsyncCancel( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (x_threading_impl)
        IXThreadingImpl_XAsyncCancel( x_threading_impl, async );
}

HRESULT WINAPI XAsyncRun( XAsyncBlock *async, XAsyncWork *work )
{
    TRACE("(%p, %p)\n", async, work);
    if (!x_threading_impl) return E_FAIL;
    return IXThreadingImpl_XAsyncRun( x_threading_impl, async, work );
}

/* =========================================================================
 * 4. XPersistentLocalStorage
 * ========================================================================= */

HRESULT WINAPI XPersistentLocalStorageGetPath( SIZE_T pathSize, char *path, SIZE_T *pathUsed )
{
    TRACE("(%zu, %p, %p)\n", pathSize, path, pathUsed);
    if (!x_persistent_local_storage_impl) return E_FAIL;
    return IXPersistentLocalStorageImpl_XPersistentLocalStorageGetPath( x_persistent_local_storage_impl, pathSize, path, pathUsed );
}

HRESULT WINAPI XPersistentLocalStorageGetPathSize( SIZE_T *pathSize )
{
    TRACE("(%p)\n", pathSize);
    if (!x_persistent_local_storage_impl) return E_FAIL;
    return IXPersistentLocalStorageImpl_XPersistentLocalStorageGetPathSize( x_persistent_local_storage_impl, pathSize );
}

HRESULT WINAPI XPersistentLocalStorageGetSpaceInfo( XPersistentLocalStorageSpaceInfo *info )
{
    TRACE("(%p)\n", info);
    if (!x_persistent_local_storage_impl) return E_FAIL;
    return IXPersistentLocalStorageImpl_XPersistentLocalStorageGetSpaceInfo( x_persistent_local_storage_impl, info );
}

HRESULT WINAPI XPersistentLocalStoragePromptUserForSpaceAsync( UINT64 requestedBytes, XAsyncBlock *async )
{
    TRACE("(%lu, %p)\n", requestedBytes, async);
    if (!x_persistent_local_storage_impl) return E_FAIL;
    return IXPersistentLocalStorageImpl_XPersistentLocalStoragePromptUserForSpaceAsync( x_persistent_local_storage_impl, requestedBytes, async );
}

HRESULT WINAPI XPersistentLocalStoragePromptUserForSpaceResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_persistent_local_storage_impl) return E_FAIL;
    return IXPersistentLocalStorageImpl_XPersistentLocalStoragePromptUserForSpaceResult( x_persistent_local_storage_impl, async );
}

/* =========================================================================
 * 5. XPackage
 * ========================================================================= */

bool WINAPI XPackageIsPackagedProcess(void)
{
    TRACE("()\n");
    if (!x_package_impl) return FALSE;
    return IXPackageImpl_XPackageIsPackagedProcess( x_package_impl );
}

HRESULT WINAPI XPackageGetCurrentProcessPackageIdentifier( SIZE_T bufferSize, char *buffer )
{
    TRACE("(%zu, %p)\n", bufferSize, buffer);
    if (!x_package_impl) return E_FAIL;
    return IXPackageImpl_XPackageGetCurrentProcessPackageIdentifier( x_package_impl, bufferSize, buffer );
}

HRESULT WINAPI XPackageGetMountPath( XPackageMountHandle mount, SIZE_T pathSize, char *path )
{
    TRACE("(%p, %zu, %p)\n", mount, pathSize, path);
    if (!x_package_impl) return E_FAIL;
    return IXPackageImpl_XPackageGetMountPath( x_package_impl, mount, pathSize, path );
}

HRESULT WINAPI XPackageGetMountPathSize( XPackageMountHandle mount, SIZE_T *pathSize )
{
    TRACE("(%p, %p)\n", mount, pathSize);
    if (!x_package_impl) return E_FAIL;
    return IXPackageImpl_XPackageGetMountPathSize( x_package_impl, mount, pathSize );
}

HRESULT WINAPI XPackageCreateInstallationMonitor( const char *packageIdentifier, UINT32 selectorCount, const XPackageChunkSelector *selectors, UINT32 minimumUpdateIntervalMs, XTaskQueueHandle queue, XPackageInstallationMonitorHandle *installationMonitor )
{
    TRACE("(%s, %u, %p, %u, %p, %p)\n", packageIdentifier, selectorCount, selectors, minimumUpdateIntervalMs, queue, installationMonitor);
    if (!x_package_impl) return E_FAIL;
    return IXPackageImpl_XPackageCreateInstallationMonitor( x_package_impl, packageIdentifier, selectorCount, (XPackageChunkSelector *)selectors, minimumUpdateIntervalMs, queue, installationMonitor );
}

void WINAPI XPackageCloseInstallationMonitorHandle( XPackageInstallationMonitorHandle installationMonitor )
{
    TRACE("(%p)\n", installationMonitor);
    if (x_package_impl)
        IXPackageImpl_XPackageCloseInstallationMonitorHandle( x_package_impl, installationMonitor );
}

void WINAPI XPackageCloseMountHandle( XPackageMountHandle mount )
{
    TRACE("(%p)\n", mount);
    if (x_package_impl)
        IXPackageImpl_XPackageCloseMountHandle( x_package_impl, mount );
}

void WINAPI XPackageGetInstallationProgress( XPackageInstallationMonitorHandle installationMonitor, XPackageInstallationProgress *progress )
{
    TRACE("(%p, %p)\n", installationMonitor, progress);
    if (x_package_impl)
        IXPackageImpl_XPackageGetInstallationProgress( x_package_impl, installationMonitor, progress );
}

bool WINAPI XPackageUpdateInstallationMonitor( XPackageInstallationMonitorHandle installationMonitor )
{
    TRACE("(%p)\n", installationMonitor);
    if (!x_package_impl) return FALSE;
    return IXPackageImpl_XPackageUpdateInstallationMonitor( x_package_impl, installationMonitor );
}

HRESULT WINAPI XPackageGetUserLocale( SIZE_T localeSize, char *locale )
{
    TRACE("(%zu, %p)\n", localeSize, locale);
    if (!x_package_impl) return E_FAIL;
    return IXPackageImpl_XPackageGetUserLocale( x_package_impl, localeSize, locale );
}

/* =========================================================================
 * 6. XGameSave
 * ========================================================================= */

HRESULT WINAPI XGameSaveInitializeProvider( XUserHandle user, const char *serviceConfigurationId, bool syncOnDemand, XGameSaveProviderHandle *provider )
{
    TRACE("(%p, %s, %d, %p)\n", user, serviceConfigurationId, syncOnDemand, provider);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveInitializeProvider( (IXGameSaveImpl3 *)x_game_save_impl, user, serviceConfigurationId, syncOnDemand ? TRUE : FALSE, provider );
}

HRESULT WINAPI XGameSaveInitializeProviderAsync( XUserHandle user, const char *serviceConfigurationId, bool syncOnDemand, XAsyncBlock *async )
{
    TRACE("(%p, %s, %d, %p)\n", user, serviceConfigurationId, syncOnDemand, async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveInitializeProviderAsync( (IXGameSaveImpl3 *)x_game_save_impl, user, serviceConfigurationId, syncOnDemand ? TRUE : FALSE, async );
}

HRESULT WINAPI XGameSaveInitializeProviderResult( XAsyncBlock *async, XGameSaveProviderHandle *provider )
{
    TRACE("(%p, %p)\n", async, provider);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveInitializeProviderResult( (IXGameSaveImpl3 *)x_game_save_impl, async, provider );
}

void WINAPI XGameSaveCloseProvider( XGameSaveProviderHandle provider )
{
    TRACE("(%p)\n", provider);
    if (x_game_save_impl)
        IXGameSaveImpl3_XGameSaveCloseProvider( (IXGameSaveImpl3 *)x_game_save_impl, provider );
}

HRESULT WINAPI XGameSaveCreateUpdate( XGameSaveContainerHandle container, const char *containerDisplayName, XGameSaveUpdateHandle *updateContext )
{
    TRACE("(%p, %s, %p)\n", container, containerDisplayName, updateContext);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveCreateUpdate( (IXGameSaveImpl3 *)x_game_save_impl, container, containerDisplayName, updateContext );
}

void WINAPI XGameSaveCloseUpdate( XGameSaveUpdateHandle updateContext )
{
    TRACE("(%p)\n", updateContext);
    if (x_game_save_impl)
        IXGameSaveImpl3_XGameSaveCloseUpdate( (IXGameSaveImpl3 *)x_game_save_impl, updateContext );
}

HRESULT WINAPI XGameSaveSubmitUpdate( XGameSaveUpdateHandle updateContext )
{
    TRACE("(%p)\n", updateContext);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveSubmitUpdate( (IXGameSaveImpl3 *)x_game_save_impl, updateContext );
}

HRESULT WINAPI XGameSaveSubmitUpdateAsync( XGameSaveUpdateHandle updateContext, XAsyncBlock *async )
{
    TRACE("(%p, %p)\n", updateContext, async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveSubmitUpdateAsync( (IXGameSaveImpl3 *)x_game_save_impl, updateContext, async );
}

HRESULT WINAPI XGameSaveSubmitUpdateResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveSubmitUpdateResult( (IXGameSaveImpl3 *)x_game_save_impl, async );
}

HRESULT WINAPI XGameSaveDeleteContainer( XGameSaveProviderHandle provider, const char *containerName )
{
    TRACE("(%p, %s)\n", provider, containerName);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveDeleteContainer( (IXGameSaveImpl3 *)x_game_save_impl, provider, containerName );
}

HRESULT WINAPI XGameSaveDeleteContainerAsync( XGameSaveProviderHandle provider, const char *containerName, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", provider, containerName, async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveDeleteContainerAsync( (IXGameSaveImpl3 *)x_game_save_impl, provider, containerName, async );
}

HRESULT WINAPI XGameSaveDeleteContainerResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveDeleteContainerResult( (IXGameSaveImpl3 *)x_game_save_impl, async );
}

HRESULT WINAPI XGameSaveReadBlobData( XGameSaveContainerHandle container, const char **blobNames, UINT32 *countOfBlobs, SIZE_T blobsSize, XGameSaveBlob *blobData )
{
    TRACE("(%p, %p, %p, %zu, %p)\n", container, blobNames, countOfBlobs, blobsSize, blobData);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveReadBlobData( (IXGameSaveImpl3 *)x_game_save_impl, container, blobNames, countOfBlobs, blobsSize, blobData );
}

HRESULT WINAPI XGameSaveReadBlobDataAsync( XGameSaveContainerHandle container, const char **blobNames, UINT32 countOfBlobs, XAsyncBlock *async )
{
    TRACE("(%p, %p, %u, %p)\n", container, blobNames, countOfBlobs, async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveReadBlobDataAsync( (IXGameSaveImpl3 *)x_game_save_impl, container, blobNames, countOfBlobs, async );
}

HRESULT WINAPI XGameSaveReadBlobDataResult( XAsyncBlock *async, SIZE_T blobsSize, XGameSaveBlob *blobData, UINT32 *countOfBlobs )
{
    TRACE("(%p, %zu, %p, %p)\n", async, blobsSize, blobData, countOfBlobs);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveReadBlobDataResult( (IXGameSaveImpl3 *)x_game_save_impl, async, blobsSize, blobData, countOfBlobs );
}

HRESULT WINAPI XGameSaveGetContainerInfo( XGameSaveProviderHandle provider, const char *containerName, void *context, XGameSaveContainerInfoCallback *callback )
{
    TRACE("(%p, %s, %p, %p)\n", provider, containerName, context, callback);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveGetContainerInfo( (IXGameSaveImpl3 *)x_game_save_impl, provider, containerName, context, callback );
}

HRESULT WINAPI XGameSaveGetRemainingQuota( XGameSaveProviderHandle provider, INT64 *remainingQuota )
{
    TRACE("(%p, %p)\n", provider, remainingQuota);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveGetRemainingQuota( (IXGameSaveImpl3 *)x_game_save_impl, provider, remainingQuota );
}

HRESULT WINAPI XGameSaveGetRemainingQuotaAsync( XGameSaveProviderHandle provider, XAsyncBlock *async )
{
    TRACE("(%p, %p)\n", provider, async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveGetRemainingQuotaAsync( (IXGameSaveImpl3 *)x_game_save_impl, provider, async );
}

HRESULT WINAPI XGameSaveGetRemainingQuotaResult( XAsyncBlock *async, INT64 *remainingQuota )
{
    TRACE("(%p, %p)\n", async, remainingQuota);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveGetRemainingQuotaResult( (IXGameSaveImpl3 *)x_game_save_impl, async, remainingQuota );
}

HRESULT WINAPI XGameSaveFilesGetFolderWithUiAsync( XUserHandle requestingUser, const char *configurationId, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", requestingUser, configurationId, async);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveFilesGetFolderWithUiAsync( (IXGameSaveImpl3 *)x_game_save_impl, requestingUser, configurationId, async );
}

HRESULT WINAPI XGameSaveFilesGetFolderWithUiResult( XAsyncBlock *async, SIZE_T folderSize, char *folderResult )
{
    TRACE("(%p, %zu, %p)\n", async, folderSize, folderResult);
    if (!x_game_save_impl) return E_FAIL;
    return IXGameSaveImpl3_XGameSaveFilesGetFolderWithUiResult( (IXGameSaveImpl3 *)x_game_save_impl, async, folderSize, folderResult );
}

/* =========================================================================
 * 7. XStore
 * ========================================================================= */

HRESULT WINAPI XStoreCreateContext( const XUserHandle user, XStoreContextHandle *storeContextHandle )
{
    TRACE("(%p, %p)\n", user, storeContextHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreCreateContext( (IXStoreImpl6 *)x_store_impl, user, storeContextHandle );
}

void WINAPI XStoreCloseContextHandle( XStoreContextHandle storeContextHandle )
{
    TRACE("(%p)\n", storeContextHandle);
    if (x_store_impl)
        IXStoreImpl6_XStoreCloseContextHandle( (IXStoreImpl6 *)x_store_impl, storeContextHandle );
}

void WINAPI XStoreCloseLicenseHandle( XStoreLicenseHandle storeLicenseHandle )
{
    TRACE("(%p)\n", storeLicenseHandle);
    if (x_store_impl)
        IXStoreImpl6_XStoreCloseLicenseHandle( (IXStoreImpl6 *)x_store_impl, storeLicenseHandle );
}

void WINAPI XStoreCloseProductsQueryHandle( XStoreProductQueryHandle storeProductQueryHandle )
{
    TRACE("(%p)\n", storeProductQueryHandle);
    if (x_store_impl)
        IXStoreImpl6_XStoreCloseProductsQueryHandle( (IXStoreImpl6 *)x_store_impl, storeProductQueryHandle );
}

HRESULT WINAPI XStoreQueryGameLicenseAsync( const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    TRACE("(%p, %p)\n", storeContextHandle, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryGameLicenseAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, async );
}

HRESULT WINAPI XStoreQueryGameLicenseResult( XAsyncBlock *async, XStoreGameLicense *license )
{
    TRACE("(%p, %p)\n", async, license);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryGameLicenseResult( (IXStoreImpl6 *)x_store_impl, async, license );
}

bool WINAPI XStoreIsLicenseValid( const XStoreGameLicense *license )
{
    TRACE("(%p)\n", license);
    if (!license) return FALSE;
    return license->isActive ? TRUE : FALSE;
}

HRESULT WINAPI XStoreQueryAssociatedProductsAsync( const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, UINT32 maxItemsToRetrievePerPage, XAsyncBlock *async )
{
    TRACE("(%p, %d, %u, %p)\n", storeContextHandle, productKinds, maxItemsToRetrievePerPage, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryAssociatedProductsAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, productKinds, maxItemsToRetrievePerPage, async );
}

HRESULT WINAPI XStoreQueryAssociatedProductsResult( XAsyncBlock *async, XStoreProductQueryHandle *storeProductQueryHandle )
{
    TRACE("(%p, %p)\n", async, storeProductQueryHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryAssociatedProductsResult( (IXStoreImpl6 *)x_store_impl, async, storeProductQueryHandle );
}

HRESULT WINAPI XStoreQueryEntitledProductsAsync( const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, UINT32 maxItemsToRetrievePerPage, XAsyncBlock *async )
{
    TRACE("(%p, %d, %u, %p)\n", storeContextHandle, productKinds, maxItemsToRetrievePerPage, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryEntitledProductsAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, productKinds, maxItemsToRetrievePerPage, async );
}

HRESULT WINAPI XStoreQueryEntitledProductsResult( XAsyncBlock *async, XStoreProductQueryHandle *storeProductQueryHandle )
{
    TRACE("(%p, %p)\n", async, storeProductQueryHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryEntitledProductsResult( (IXStoreImpl6 *)x_store_impl, async, storeProductQueryHandle );
}

HRESULT WINAPI XStoreQueryProductsAsync( const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, const char **actionFilters, SIZE_T actionFiltersCount, const char **storeIds, SIZE_T storeIdsCount, XAsyncBlock *async )
{
    TRACE("(%p, %d, %p, %zu, %p, %zu, %p)\n", storeContextHandle, productKinds, actionFilters, actionFiltersCount, storeIds, storeIdsCount, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryProductsAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, productKinds, storeIds, storeIdsCount, actionFilters, actionFiltersCount, async );
}

HRESULT WINAPI XStoreQueryProductsResult( XAsyncBlock *async, XStoreProductQueryHandle *storeProductQueryHandle )
{
    TRACE("(%p, %p)\n", async, storeProductQueryHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryProductsResult( (IXStoreImpl6 *)x_store_impl, async, storeProductQueryHandle );
}

HRESULT WINAPI XStoreProductsQueryNextPageAsync( const XStoreProductQueryHandle storeProductQueryHandle, XAsyncBlock *async )
{
    TRACE("(%p, %p)\n", storeProductQueryHandle, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreProductsQueryNextPageAsync( (IXStoreImpl6 *)x_store_impl, storeProductQueryHandle, async );
}

HRESULT WINAPI XStoreProductsQueryNextPageResult( XAsyncBlock *async, XStoreProductQueryHandle *storeProductQueryHandle )
{
    TRACE("(%p, %p)\n", async, storeProductQueryHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreProductsQueryNextPageResult( (IXStoreImpl6 *)x_store_impl, async, storeProductQueryHandle );
}

bool WINAPI XStoreProductsQueryHasMorePages( const XStoreProductQueryHandle storeProductQueryHandle )
{
    TRACE("(%p)\n", storeProductQueryHandle);
    if (!x_store_impl) return FALSE;
    return IXStoreImpl6_XStoreProductsQueryHasMorePages( (IXStoreImpl6 *)x_store_impl, storeProductQueryHandle );
}

HRESULT WINAPI XStoreEnumerateProductsQuery( const XStoreProductQueryHandle storeProductQueryHandle, void *context, XStoreProductQueryCallback *callback )
{
    TRACE("(%p, %p, %p)\n", storeProductQueryHandle, context, callback);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreEnumerateProductsQuery( (IXStoreImpl6 *)x_store_impl, storeProductQueryHandle, context, callback );
}

HRESULT WINAPI XStoreAcquireLicenseForPackageAsync( const XStoreContextHandle storeContextHandle, const char *packageIdentifier, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", storeContextHandle, packageIdentifier, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreAcquireLicenseForPackageAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, packageIdentifier, async );
}

HRESULT WINAPI XStoreAcquireLicenseForPackageResult( XAsyncBlock *async, XStoreLicenseHandle *storeLicenseHandle )
{
    TRACE("(%p, %p)\n", async, storeLicenseHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreAcquireLicenseForPackageResult( (IXStoreImpl6 *)x_store_impl, async, storeLicenseHandle );
}

HRESULT WINAPI XStoreAcquireLicenseForDurablesAsync( const XStoreContextHandle storeContextHandle, const char *storeId, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", storeContextHandle, storeId, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreAcquireLicenseForDurablesAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, storeId, async );
}

HRESULT WINAPI XStoreAcquireLicenseForDurablesResult( XAsyncBlock *async, XStoreLicenseHandle *storeLicenseHandle )
{
    TRACE("(%p, %p)\n", async, storeLicenseHandle);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreAcquireLicenseForDurablesResult( (IXStoreImpl6 *)x_store_impl, async, storeLicenseHandle );
}

HRESULT WINAPI XStoreCanAcquireLicenseForPackageAsync( const XStoreContextHandle storeContextHandle, const char *packageIdentifier, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", storeContextHandle, packageIdentifier, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreCanAcquireLicenseForPackageAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, packageIdentifier, async );
}

HRESULT WINAPI XStoreCanAcquireLicenseForPackageResult( XAsyncBlock *async, XStoreCanAcquireLicenseResult *result )
{
    TRACE("(%p, %p)\n", async, result);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreCanAcquireLicenseForPackageResult( (IXStoreImpl6 *)x_store_impl, async, result );
}

HRESULT WINAPI XStoreCanAcquireLicenseForStoreIdAsync( const XStoreContextHandle storeContextHandle, const char *storeId, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", storeContextHandle, storeId, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreCanAcquireLicenseForStoreIdAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, storeId, async );
}

HRESULT WINAPI XStoreCanAcquireLicenseForStoreIdResult( XAsyncBlock *async, XStoreCanAcquireLicenseResult *result )
{
    TRACE("(%p, %p)\n", async, result);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreCanAcquireLicenseForStoreIdResult( (IXStoreImpl6 *)x_store_impl, async, result );
}

HRESULT WINAPI XStoreShowPurchaseUIAsync( const XStoreContextHandle storeContextHandle, const char *storeId, const char *name, const char *payload, XAsyncBlock *async )
{
    TRACE("(%p, %s, %s, %s, %p)\n", storeContextHandle, storeId, name, payload, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowPurchaseUIAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, storeId, name, payload, async );
}

HRESULT WINAPI XStoreShowPurchaseUIResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowPurchaseUIResult( (IXStoreImpl6 *)x_store_impl, async );
}

HRESULT WINAPI XStoreShowProductPageUIAsync( const XStoreContextHandle storeContextHandle, const char *storeId, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p)\n", storeContextHandle, storeId, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowProductPageUIAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, storeId, async );
}

HRESULT WINAPI XStoreShowProductPageUIResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowProductPageUIResult( (IXStoreImpl6 *)x_store_impl, async );
}

HRESULT WINAPI XStoreShowRateAndReviewUIAsync( const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    TRACE("(%p, %p)\n", storeContextHandle, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowRateAndReviewUIAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, async );
}

HRESULT WINAPI XStoreShowRateAndReviewUIResult( XAsyncBlock *async, XStoreRateAndReviewResult *result )
{
    TRACE("(%p, %p)\n", async, result);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowRateAndReviewUIResult( (IXStoreImpl6 *)x_store_impl, async, result );
}

HRESULT WINAPI XStoreShowRedeemTokenUIAsync( const XStoreContextHandle storeContextHandle, const char *token, const char **allowedStoreIds, SIZE_T allowedStoreIdsCount, bool disallowCsv, XAsyncBlock *async )
{
    TRACE("(%p, %s, %p, %zu, %d, %p)\n", storeContextHandle, token, allowedStoreIds, allowedStoreIdsCount, disallowCsv, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowRedeemTokenUIAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, token, allowedStoreIds, allowedStoreIdsCount, disallowCsv ? TRUE : FALSE, async );
}

HRESULT WINAPI XStoreShowRedeemTokenUIResult( XAsyncBlock *async )
{
    TRACE("(%p)\n", async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreShowRedeemTokenUIResult( (IXStoreImpl6 *)x_store_impl, async );
}

HRESULT WINAPI XStoreGetUserCollectionsIdAsync( const XStoreContextHandle storeContextHandle, const char *serviceTicket, const char *publisherUserId, XAsyncBlock *async )
{
    TRACE("(%p, %s, %s, %p)\n", storeContextHandle, serviceTicket, publisherUserId, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreGetUserCollectionsIdAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, serviceTicket, publisherUserId, async );
}

HRESULT WINAPI XStoreGetUserCollectionsIdResultSize( XAsyncBlock *async, SIZE_T *size )
{
    TRACE("(%p, %p)\n", async, size);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreGetUserCollectionsIdResultSize( (IXStoreImpl6 *)x_store_impl, async, size );
}

HRESULT WINAPI XStoreGetUserCollectionsIdResult( XAsyncBlock *async, SIZE_T size, char *result )
{
    TRACE("(%p, %zu, %p)\n", async, size, result);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreGetUserCollectionsIdResult( (IXStoreImpl6 *)x_store_impl, async, size, result );
}

HRESULT WINAPI XStoreGetUserPurchaseIdAsync( const XStoreContextHandle storeContextHandle, const char *serviceTicket, const char *publisherUserId, XAsyncBlock *async )
{
    TRACE("(%p, %s, %s, %p)\n", storeContextHandle, serviceTicket, publisherUserId, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreGetUserPurchaseIdAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, serviceTicket, publisherUserId, async );
}

HRESULT WINAPI XStoreGetUserPurchaseIdResultSize( XAsyncBlock *async, SIZE_T *size )
{
    TRACE("(%p, %p)\n", async, size);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreGetUserPurchaseIdResultSize( (IXStoreImpl6 *)x_store_impl, async, size );
}

HRESULT WINAPI XStoreGetUserPurchaseIdResult( XAsyncBlock *async, SIZE_T size, char *result )
{
    TRACE("(%p, %zu, %p)\n", async, size, result);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreGetUserPurchaseIdResult( (IXStoreImpl6 *)x_store_impl, async, size, result );
}

HRESULT WINAPI XStoreQueryLicenseTokenAsync( const XStoreContextHandle storeContextHandle, const char **productIds, SIZE_T productIdsCount, const char *customDeveloperString, XAsyncBlock *async )
{
    TRACE("(%p, %p, %zu, %s, %p)\n", storeContextHandle, productIds, productIdsCount, customDeveloperString, async);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryLicenseTokenAsync( (IXStoreImpl6 *)x_store_impl, storeContextHandle, productIds, productIdsCount, customDeveloperString, async );
}

HRESULT WINAPI XStoreQueryLicenseTokenResultSize( XAsyncBlock *async, SIZE_T *size )
{
    TRACE("(%p, %p)\n", async, size);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryLicenseTokenResultSize( (IXStoreImpl6 *)x_store_impl, async, size );
}

HRESULT WINAPI XStoreQueryLicenseTokenResult( XAsyncBlock *async, SIZE_T size, char *result )
{
    TRACE("(%p, %zu, %p)\n", async, size, result);
    if (!x_store_impl) return E_FAIL;
    return IXStoreImpl6_XStoreQueryLicenseTokenResult( (IXStoreImpl6 *)x_store_impl, async, size, result );
}
