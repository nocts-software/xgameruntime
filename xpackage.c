/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XPackage
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

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

struct x_package
{
    IXPackageImpl4 IXPackageImpl4_iface;
    LONG ref;
};

static inline struct x_package *impl_from_IXPackageImpl4( IXPackageImpl4 *iface )
{
    return CONTAINING_RECORD( iface, struct x_package, IXPackageImpl4_iface );
}

static HRESULT WINAPI x_package_QueryInterface( IXPackageImpl4 *iface, REFIID iid, void **out )
{
    struct x_package *impl = impl_from_IXPackageImpl4( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown       ) ||
        IsEqualGUID( iid, &IID_IXPackageImpl  ) ||
        IsEqualGUID( iid, &IID_IXPackageImpl2 ) ||
        IsEqualGUID( iid, &IID_IXPackageImpl3 ) ||
        IsEqualGUID( iid, &IID_IXPackageImpl4 ))
    {
        IXPackageImpl_AddRef( *out = &impl->IXPackageImpl4_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_package_AddRef( IXPackageImpl4 *iface )
{
    struct x_package *impl = impl_from_IXPackageImpl4( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_package_Release( IXPackageImpl4 *iface )
{
    struct x_package *impl = impl_from_IXPackageImpl4( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI x_package_XPackageGetCurrentProcessPackageIdentifier( IXPackageImpl4 *iface, SIZE_T bufferSize, char *buffer )
{
    const char *pfn = getenv("LOCAL_APP_MODEL_PACKAGE_FAMILY_NAME");
    SIZE_T len;
    TRACE( "iface %p, bufferSize %Iu, buffer %p\n", iface, bufferSize, buffer );
    if (!pfn || !pfn[0]) pfn = "XodusGame_8wekyb3d8bbwe";
    len = strlen(pfn) + 1;
    if (!buffer || bufferSize < len) return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    strcpy(buffer, pfn);
    return S_OK;
}

static BOOLEAN WINAPI x_package_XPackageIsPackagedProcess( IXPackageImpl4 *iface )
{
    TRACE( "iface %p\n", iface );
    return TRUE;
}

static HRESULT WINAPI x_package_XPackageCreateInstallationMonitor( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors, UINT32 minimumUpdateIntervalMs, XTaskQueueHandle queue, XPackageInstallationMonitorHandle *installationMonitor )
{
    TRACE( "iface %p, packageIdentifier %s, selectorCount %u, selectors %p, minimumUpdateIntervalMs %u, queue %p, installationMonitor %p\n", iface, debugstr_a( packageIdentifier ), selectorCount, selectors, minimumUpdateIntervalMs, queue, installationMonitor );
    if (installationMonitor) *installationMonitor = (XPackageInstallationMonitorHandle)(UINT_PTR)1;
    return S_OK;
}

static void WINAPI x_package_XPackageCloseInstallationMonitorHandle( IXPackageImpl4 *iface, XPackageInstallationMonitorHandle installationMonitor )
{
    TRACE( "iface %p, installationMonitor %p\n", iface, installationMonitor );
}

static void WINAPI x_package_XPackageGetInstallationProgress( IXPackageImpl4 *iface, XPackageInstallationMonitorHandle installationMonitor, XPackageInstallationProgress *progress )
{
    TRACE( "iface %p, installationMonitor %p, progress %p.\n", iface, installationMonitor, progress );
    if (progress)
    {
        progress->totalBytes = 1000000000;
        progress->installedBytes = 1000000000;
        progress->launchBytes = 1000000000;
        progress->completed = TRUE;
        progress->launchable = TRUE;
    }
}

static HRESULT WINAPI x_package_XPackageGetMountPathSize( IXPackageImpl4 *iface, XPackageMountHandle mount, SIZE_T *pathSize )
{
    DWORD len = GetCurrentDirectoryA(0, NULL);
    TRACE( "iface %p, mount %p, pathSize %p.\n", iface, mount, pathSize );
    if (pathSize)
    {
        *pathSize = len > 0 ? len : 1;
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_package_XPackageGetMountPath( IXPackageImpl4 *iface, XPackageMountHandle mount, SIZE_T pathSize, char *path )
{
    TRACE( "iface %p, mount %p, pathSize %Iu, path %p.\n", iface, mount, pathSize, path );
    if (path && pathSize > 0)
    {
        GetCurrentDirectoryA((DWORD)pathSize, path);
        return S_OK;
    }
    return E_POINTER;
}

static BOOLEAN WINAPI x_package_XPackageUpdateInstallationMonitor( IXPackageImpl4 *iface, XPackageInstallationMonitorHandle installationMonitor )
{
    TRACE( "iface %p, installationMonitor %p\n", iface, installationMonitor );
    return TRUE;
}

static HRESULT WINAPI x_package_XPackageRegisterInstallationProgressChanged( IXPackageImpl4 *iface, XPackageInstallationMonitorHandle installationMonitor, void *context, XPackageInstallationProgressCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, installationMonitor %p, context %p, callback %p, token %p\n", iface, installationMonitor, context, callback, token );
    if (token) token->token = 1;
    return S_OK;
}

static BOOLEAN WINAPI x_package_XPackageUnregisterInstallationProgressChanged( IXPackageImpl4 *iface, XPackageInstallationMonitorHandle installationMonitor, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    TRACE( "iface %p, installationMonitor %p, token %p, wait %d\n", iface, installationMonitor, &token, wait );
    return TRUE;
}

static HRESULT WINAPI x_package_XPackageGetUserLocale( IXPackageImpl4 *iface, SIZE_T localeSize, char *locale )
{
    TRACE( "iface %p, localeSize %Iu, locale %p\n", iface, localeSize, locale );
    if (locale && localeSize >= 6)
    {
        strcpy(locale, "en-US");
        return S_OK;
    }
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageFindChunkAvailability( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors, XPackageChunkAvailability *availability )
{
    TRACE( "iface %p, packageIdentifier %s, selectorCount %u, selectors %p, availability %p\n", iface, packageIdentifier, selectorCount, selectors, availability );
    if (availability) *availability = XPackageChunkAvailability_Ready;
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageEnumerateChunkAvailability( IXPackageImpl4 *iface, const char *packageIdentifier, XPackageChunkSelectorType type, void *context, XPackageChunkAvailabilityCallback *callback )
{
    TRACE( "iface %p, packageIdentifier %s, type %d, context %p, callback %p\n", iface, debugstr_a( packageIdentifier ), type, context, callback );
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageChangeChunkInstallOrder( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors )
{
    TRACE( "iface %p, packageIdentifier %s, selectorCount %u, selectors %p\n", iface, debugstr_a( packageIdentifier ), selectorCount, selectors );
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageInstallChunks( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors, UINT32 minimumUpdateIntervalMs, BOOLEAN suppressUserConfirmation, XTaskQueueHandle queue, XPackageInstallationMonitorHandle *installationMonitor )
{
    FIXME( "iface %p, packageIdentifier %s, selectorCount %u, selectors %p, minimumUpdateIntervalMs %u, suppressUserConfirmation %d, queue %p, installationMonitor %p stub!\n", iface, debugstr_a( packageIdentifier ), selectorCount, selectors, minimumUpdateIntervalMs, suppressUserConfirmation, queue, installationMonitor );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageInstallChunksAsync( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors, UINT32 minimumUpdateIntervalMs, BOOLEAN suppressUserConfirmation, XAsyncBlock *asyncBlock )
{
    FIXME( "iface %p, packageIdentifier %s, selectorCount %u, selectors %p, minimumUpdateIntervalMs %u, suppressUserConfirmation %d, asyncBlock %p stub!\n", iface, packageIdentifier, selectorCount, selectors, minimumUpdateIntervalMs, suppressUserConfirmation, asyncBlock );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageInstallChunksResult( IXPackageImpl4 *iface, XAsyncBlock *asyncBlock, XPackageInstallationMonitorHandle *installationMonitor )
{
    FIXME( "iface %p, asyncBlock %p, installationMonitor %p stub!\n", iface, asyncBlock, installationMonitor );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageEstimateDownloadSize( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors, UINT64 *downloadSize, BOOLEAN *shouldPresentUserConfirmation )
{
    FIXME( "iface %p, packageIdentifier %s, selectorCount %u, selectors %p, downloadSize %p, shouldPresentUserConfirmation %p stub!\n", iface, packageIdentifier, selectorCount, selectors, downloadSize, shouldPresentUserConfirmation );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageUninstallChunks( IXPackageImpl4 *iface, const char *packageIdentifier, UINT32 selectorCount, XPackageChunkSelector *selectors )
{
    FIXME( "iface %p, packageIdentifier %s, selectorCount %u, selectores %p stub!\n", iface, packageIdentifier, selectorCount, selectors );
    return E_NOTIMPL;
}

static HRESULT WINAPI __PADDING__( IXPackageImpl4 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI __PADDING_2__( IXPackageImpl4 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static BOOLEAN WINAPI x_package_XPackageUnregisterPackageInstalled( IXPackageImpl4 *iface, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    FIXME( "iface %p, token %p, wait %d stub!\n", iface, &token, wait );
    return TRUE;
}

static HRESULT WINAPI __PADDING_3__( IXPackageImpl4 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static void WINAPI x_package_XPackageCloseMountHandle( IXPackageImpl4 *iface, XPackageMountHandle mount )
{
    FIXME( "iface %p, mount %p stub!\n", iface, mount );
}

static HRESULT WINAPI __PADDING_4__( IXPackageImpl4 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageEnumeratePackages( IXPackageImpl4 *iface, XPackageKind kind, XPackageEnumerationScope scope, void *context, XPackageEnumerationCallback *callback )
{
    XPackageDetails details;
    const char *pfn = getenv("LOCAL_APP_MODEL_PACKAGE_FAMILY_NAME");
    const char *title_name = getenv("XODUS_DISPLAY_NAME");
    const char *publisher = getenv("XODUS_PUBLISHER");
    const char *store_id = getenv("XODUS_STORE_ID");
    const char *title_id = getenv("XODUS_TITLE_ID");

    TRACE( "iface %p, kind %d, scope %d, context %p, callback %p\n", iface, kind, scope, context, callback );
    if (!pfn || !pfn[0]) pfn = "XodusGame_8wekyb3d8bbwe";
    if (!title_name || !title_name[0]) title_name = "Xodus Game";
    if (!publisher || !publisher[0]) publisher = "Xbox Game Studios";
    if (!store_id || !store_id[0]) store_id = "000000000000";
    if (!title_id || !title_id[0]) title_id = "00000000";

    memset(&details, 0, sizeof(details));
    details.packageIdentifier = pfn;
    details.displayName = title_name;
    details.description = title_name;
    details.publisher = publisher;
    details.storeId = store_id;
    details.titleID = title_id;
    details.kind = kind;
    details.installing = FALSE;
    if (callback) callback(context, &details);
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageRegisterPackageInstalled( IXPackageImpl4 *iface, XTaskQueueHandle queue, void *context, XPackageInstalledCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, context %p, callback %p, token %p\n", iface, queue, context, callback, token );
    if (token) token->token = 1;
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageGetWriteStats( IXPackageImpl4 *iface, XPackageWriteStats *writeStats )
{
    TRACE( "iface %p, writeStats %p\n", iface, writeStats );
    if (writeStats) memset(writeStats, 0, sizeof(*writeStats));
    return S_OK;
}

static HRESULT WINAPI __PADDING_5__( IXPackageImpl4 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageUninstallUWPInstance( IXPackageImpl4 *iface, const char *packageName )
{
    TRACE( "iface %p, packageName %s\n", iface, debugstr_a( packageName ) );
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageEnumerateFeatures( IXPackageImpl4 *iface, const char *packageIdentifier, void *context, XPackageFeatureEnumerationCallback *callback )
{
    TRACE( "iface %p, packageIdentifier %s, context %p, callback %p\n", iface, packageIdentifier, context, callback );
    return S_OK;
}

static BOOLEAN WINAPI x_package_XPackageUninstallPackage( IXPackageImpl4 *iface, const char *packageIdentifier )
{
    TRACE( "iface %p, packageIdentifier %s\n", iface, debugstr_a( packageIdentifier ) );
    return TRUE;
}

static HRESULT WINAPI x_package_XPackageEnumeratePackages2( IXPackageImpl4 *iface, XPackageKind kind, XPackageEnumerationScope scope, void *context, XPackageEnumerationCallback *callback )
{
    XPackageDetails details;
    const char *pfn = getenv("LOCAL_APP_MODEL_PACKAGE_FAMILY_NAME");
    const char *title_name = getenv("XODUS_DISPLAY_NAME");
    const char *publisher = getenv("XODUS_PUBLISHER");
    const char *store_id = getenv("XODUS_STORE_ID");
    const char *title_id = getenv("XODUS_TITLE_ID");

    TRACE( "iface %p, kind %d, scope %d, context %p, callback %p\n", iface, kind, scope, context, callback );
    if (!pfn || !pfn[0]) pfn = "XodusGame_8wekyb3d8bbwe";
    if (!title_name || !title_name[0]) title_name = "Xodus Game";
    if (!publisher || !publisher[0]) publisher = "Xbox Game Studios";
    if (!store_id || !store_id[0]) store_id = "000000000000";
    if (!title_id || !title_id[0]) title_id = "00000000";

    memset(&details, 0, sizeof(details));
    details.packageIdentifier = pfn;
    details.displayName = title_name;
    details.description = title_name;
    details.publisher = publisher;
    details.storeId = store_id;
    details.titleID = title_id;
    details.kind = kind;
    details.installing = FALSE;
    if (callback) callback(context, &details);
    return S_OK;
}

static HRESULT WINAPI x_package_XPackageRegisterPackageInstalled2( IXPackageImpl4 *iface, XTaskQueueHandle queue, void *context, XPackageInstalledCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, context %p, callback %p, token %p\n", iface, queue, context, callback, token );
    if (token) token->token = 1;
    return S_OK;
}



static HRESULT WINAPI x_package_XPackageMountWithUiAsync( IXPackageImpl4 *iface, const char *packageIdentifier, XAsyncBlock *async )
{
    FIXME( "iface %p, packageIdentifier %s, async %p stub!\n", iface, debugstr_a( packageIdentifier ), async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageMountWithUiResult( IXPackageImpl4 *iface, XAsyncBlock *async, XPackageMountHandle *mount )
{
    FIXME( "iface %p, async %p, mount %p stub!\n", iface, async, mount );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageEnumeratePackages3( IXPackageImpl4 *iface, XPackageKind kind, XPackageEnumerationScope scope, void *context, XPackageEnumerationCallback *callback )
{
    FIXME( "iface %p, kind %d, scope %d, context %p, callback %p stub!\n", iface, kind, scope, context, callback );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageRegisterPackageInstalled3( IXPackageImpl4 *iface, XTaskQueueHandle queue, void *context, XPackageInstalledCallback *callback, XTaskQueueRegistrationToken *token )
{
    FIXME( "iface %p, queue %p, context %p, callback %p, token %p stub!\n", iface, queue, context, callback, token );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_package_XPackageGetPackageKind( IXPackageImpl4 *iface, const char *packageIdentifier, XPackageKind *kind )
{
    FIXME( "iface %p, packageIdentifier %s, kind %p stub!\n", iface, debugstr_a( packageIdentifier ), kind );
    return E_NOTIMPL;
}

static const struct IXPackageImpl4Vtbl x_package_vtbl =
{
    x_package_QueryInterface,
    x_package_AddRef,
    x_package_Release,
    /* IXPackageImpl methods */
    x_package_XPackageGetCurrentProcessPackageIdentifier,
    x_package_XPackageIsPackagedProcess,
    x_package_XPackageCreateInstallationMonitor,
    x_package_XPackageCloseInstallationMonitorHandle,
    x_package_XPackageGetInstallationProgress,
    x_package_XPackageUpdateInstallationMonitor,
    x_package_XPackageRegisterInstallationProgressChanged,
    x_package_XPackageUnregisterInstallationProgressChanged,
    x_package_XPackageGetUserLocale,
    x_package_XPackageFindChunkAvailability,
    x_package_XPackageEnumerateChunkAvailability,
    x_package_XPackageChangeChunkInstallOrder,
    x_package_XPackageInstallChunks,
    x_package_XPackageInstallChunksAsync,
    x_package_XPackageInstallChunksResult,
    x_package_XPackageEstimateDownloadSize,
    x_package_XPackageUninstallChunks,
    __PADDING__,
    __PADDING_2__,
    x_package_XPackageUnregisterPackageInstalled,
    __PADDING_3__,
    x_package_XPackageGetMountPathSize,
    x_package_XPackageGetMountPath,
    x_package_XPackageCloseMountHandle,
    __PADDING_4__,
    x_package_XPackageEnumeratePackages,
    x_package_XPackageRegisterPackageInstalled,
    x_package_XPackageGetWriteStats,
    __PADDING_5__,
    x_package_XPackageUninstallUWPInstance,
    x_package_XPackageEnumerateFeatures,
    x_package_XPackageUninstallPackage,
    /* IXPackageImpl2 methods */
    x_package_XPackageEnumeratePackages2,
    x_package_XPackageRegisterPackageInstalled2,
    x_package_XPackageMountWithUiAsync,
    x_package_XPackageMountWithUiResult,
    /* IXPackageImpl3 methods */
    x_package_XPackageEnumeratePackages3,
    x_package_XPackageRegisterPackageInstalled3,
    /* IXPackageImpl4 methods */
    x_package_XPackageGetPackageKind,
};

static struct x_package x_package =
{
    {&x_package_vtbl},
    0,
};

IXPackageImpl *x_package_impl = (IXPackageImpl *)&x_package.IXPackageImpl4_iface;

/* WinRT IPackageId & IPackage COM implementation */
static HRESULT WINAPI winrt_package_id_get_Name(void *iface, void **name)
{
    TRACE("winrt_package_id_get_Name iface %p, name %p.\n", iface, name);
    if (!name) return E_POINTER;
    *name = NULL;
    return S_OK;
}

static HRESULT WINAPI winrt_package_id_get_Publisher(void *iface, void **publisher)
{
    TRACE("winrt_package_id_get_Publisher iface %p, publisher %p.\n", iface, publisher);
    if (!publisher) return E_POINTER;
    *publisher = NULL;
    return S_OK;
}

static HRESULT WINAPI winrt_package_id_get_Version(void *iface, void *version)
{
    TRACE("winrt_package_id_get_Version iface %p, version %p.\n", iface, version);
    return S_OK;
}

static HRESULT WINAPI winrt_package_id_get_FamilyName(void *iface, void **familyName)
{
    TRACE("winrt_package_id_get_FamilyName iface %p, familyName %p.\n", iface, familyName);
    if (!familyName) return E_POINTER;
    *familyName = NULL;
    return S_OK;
}

static HRESULT WINAPI winrt_package_id_QueryInterface(void *iface, REFIID iid, void **out)
{
    TRACE("winrt_package_id_QueryInterface iface %p, iid %s, out %p.\n", iface, debugstr_guid(iid), out);
    if (!out) return E_POINTER;
    *out = iface;
    return S_OK;
}

static const void *winrt_package_id_vtbl[] = {
    winrt_package_id_QueryInterface,
    x_package_AddRef,
    x_package_Release,
    winrt_package_id_get_Name,
    winrt_package_id_get_Name,
    winrt_package_id_get_Name,
    winrt_package_id_get_Name,
    winrt_package_id_get_Publisher,
    winrt_package_id_get_Version,
    winrt_package_id_get_Name,
    winrt_package_id_get_FamilyName,
    winrt_package_id_get_Name
};

static struct {
    const void **vtbl;
} winrt_package_id_obj = { winrt_package_id_vtbl };

static HRESULT WINAPI winrt_package_get_Id(void *iface, void **id)
{
    TRACE("winrt_package_get_Id iface %p, id %p.\n", iface, id);
    if (!id) return E_POINTER;
    *id = &winrt_package_id_obj;
    return S_OK;
}

static const void *winrt_package_vtbl[] = {
    winrt_package_id_QueryInterface,
    x_package_AddRef,
    x_package_Release,
    winrt_package_get_Id,
    winrt_package_get_Id,
    winrt_package_get_Id,
    winrt_package_get_Id,
    winrt_package_get_Id,
    winrt_package_get_Id,
    winrt_package_get_Id
};


static struct {
    const void **vtbl;
} winrt_package_obj = { winrt_package_vtbl };

void *get_winrt_package_factory(void)
{
    return &winrt_package_obj;
}

