/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XStore
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
#include <string.h>

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

struct x_store
{
    IXStoreImpl6 IXStoreImpl6_iface;
    LONG ref;
};

static inline struct x_store *impl_from_IXStoreImpl6( IXStoreImpl6 *iface )
{
    return CONTAINING_RECORD( iface, struct x_store, IXStoreImpl6_iface );
}

static HRESULT WINAPI x_store_QueryInterface( IXStoreImpl6 *iface, REFIID iid, void **out )
{
    struct x_store *impl = impl_from_IXStoreImpl6( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown     ) ||
        IsEqualGUID( iid, &IID_IXStoreImpl  ) ||
        IsEqualGUID( iid, &IID_IXStoreImpl2 ) ||
        IsEqualGUID( iid, &IID_IXStoreImpl3 ) ||
        IsEqualGUID( iid, &IID_IXStoreImpl4 ) ||
        IsEqualGUID( iid, &IID_IXStoreImpl5 ) ||
        IsEqualGUID( iid, &IID_IXStoreImpl6 ))
    {
        IXStoreImpl6_AddRef( *out = &impl->IXStoreImpl6_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_store_AddRef( IXStoreImpl6 *iface )
{
    struct x_store *impl = impl_from_IXStoreImpl6( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_store_Release( IXStoreImpl6 *iface )
{
    struct x_store *impl = impl_from_IXStoreImpl6( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI x_store_XStoreCreateContext( IXStoreImpl6 *iface, const XUserHandle user, XStoreContextHandle *storeContextHandle )
{
    TRACE( "iface %p, user %p, storeContextHandle %p.\n", iface, user, storeContextHandle );
    if (storeContextHandle)
    {
        *storeContextHandle = (XStoreContextHandle)(UINT_PTR)1;
        return S_OK;
    }
    return E_POINTER;
}

static void WINAPI x_store_XStoreCloseContextHandle( IXStoreImpl6 *iface, XStoreContextHandle storeContextHandle )
{
    TRACE( "iface %p, storeContextHandle %p.\n", iface, storeContextHandle );
}


static HRESULT WINAPI x_store_XStoreQueryAssociatedProductsAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, UINT32 maxItemsToRetrievePerPage, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, productKinds %#x, maxItemsToRetrievePerPage %u, async %p\n", iface, storeContextHandle, productKinds, maxItemsToRetrievePerPage, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryAssociatedProductsResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)(UINT_PTR)1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryProductsAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, const char **storeIds, SIZE_T storeIdsCount, const char **actionFilters, SIZE_T actionFiltersCount, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, productKinds %#x, storeIds %p, storeIdsCount %Iu, actionFilters %p, actionFiltersCount %Iu, async %p\n", iface, storeContextHandle, productKinds, storeIds, storeIdsCount, actionFilters, actionFiltersCount, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryProductsResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)(UINT_PTR)1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryEntitledProductsAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, UINT32 maxItemsToRetrievePerPage, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, productKinds %#x, maxItemsToRetrievePerPage %u, async %p\n", iface, storeContextHandle, productKinds, maxItemsToRetrievePerPage, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryEntitledProductsResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)(UINT_PTR)1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryProductForCurrentGameAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, async %p\n", iface, storeContextHandle, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryProductForCurrentGameResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)(UINT_PTR)1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryProductForPackageAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XStoreProductKind productKinds, const char *packageIdentifier, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, productKinds %#x, packageIdentifier %s, async %p\n", iface, storeContextHandle, productKinds, debugstr_a( packageIdentifier ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryProductForPackageResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)(UINT_PTR)1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreEnumerateProductsQuery( IXStoreImpl6 *iface, const XStoreProductQueryHandle productQueryHandle, void *context, XStoreProductQueryCallback *callback )
{
    TRACE( "iface %p, productQueryHandle %p, context %p, callback %p\n", iface, productQueryHandle, context, callback );
    return S_OK;
}

static BOOLEAN WINAPI x_store_XStoreProductsQueryHasMorePages( IXStoreImpl6 *iface, const XStoreProductQueryHandle productQueryHandle )
{
    TRACE( "iface %p, productQueryHandle %p\n", iface, productQueryHandle );
    return FALSE;
}

static HRESULT WINAPI x_store_XStoreProductsQueryNextPageAsync( IXStoreImpl6 *iface, const XStoreProductQueryHandle productQueryHandle, XAsyncBlock *async )
{
    TRACE( "iface %p, productQueryHandle %p, async %p\n", iface, productQueryHandle, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreProductsQueryNextPageResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)(UINT_PTR)1;
    return S_OK;
}

static void WINAPI x_store_XStoreCloseProductsQueryHandle( IXStoreImpl6 *iface, XStoreProductQueryHandle productQueryHandle )
{
    TRACE( "iface %p, productQueryHandle %p\n", iface, productQueryHandle );
}

static HRESULT WINAPI x_store_XStoreAcquireLicenseForPackageAsync( IXStoreImpl6 *iface, const XStoreProductQueryHandle productQueryHandle, const char *packageIdentifier, XAsyncBlock *async )
{
    TRACE( "iface %p, productQueryHandle %p, packageIdentifier %s, async %p.\n", iface, productQueryHandle, debugstr_a( packageIdentifier ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreAcquireLicenseForPackageResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreLicenseHandle *storeLicenseHandle )
{
    TRACE( "iface %p, async %p, storeLicenseHandle %p.\n", iface, async, storeLicenseHandle );
    if (storeLicenseHandle)
    {
        *storeLicenseHandle = (XStoreLicenseHandle)(UINT_PTR)1;
        return S_OK;
    }
    return E_POINTER;
}

static BOOLEAN WINAPI x_store_XStoreIsLicenseValid( IXStoreImpl6 *iface, const XStoreLicenseHandle storeLicenseHandle )
{
    TRACE( "iface %p, storeLicenseHandle %p.\n", iface, storeLicenseHandle );
    return TRUE;
}

static void WINAPI x_store_XStoreCloseLicenseHandle( IXStoreImpl6 *iface, XStoreLicenseHandle storeLicenseHandle )
{
    TRACE( "iface %p, storeLicenseHandle %p.\n", iface, storeLicenseHandle );
}

static HRESULT WINAPI x_store_XStoreCanAcquireLicenseForStoreIdAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeProductId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeProductId %s, async %p\n", iface, storeContextHandle, debugstr_a( storeProductId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreCanAcquireLicenseForStoreIdResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreCanAcquireLicenseResult *storeCanAcquireLicense )
{
    TRACE( "iface %p, async %p, storeCanAcquireLicense %p\n", iface, async, storeCanAcquireLicense );
    if (storeCanAcquireLicense)
    {
        memset(storeCanAcquireLicense, 0, sizeof(*storeCanAcquireLicense));
        storeCanAcquireLicense->status = XStoreCanLicenseStatus_Licensable;
    }
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreCanAcquireLicenseForPackageAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *packageIdentifier, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, packageIdentifier %s, async %p\n", iface, storeContextHandle, debugstr_a( packageIdentifier ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreCanAcquireLicenseForPackageResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreCanAcquireLicenseResult *storeCanAcquireLicense )
{
    TRACE( "iface %p, async %p, storeCanAcquireLicense %p\n", iface, async, storeCanAcquireLicense );
    if (storeCanAcquireLicense)
    {
        memset(storeCanAcquireLicense, 0, sizeof(*storeCanAcquireLicense));
        storeCanAcquireLicense->status = XStoreCanLicenseStatus_Licensable;
    }
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryGameLicenseAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    fprintf(stderr, "[GDK XStore] XStoreQueryGameLicenseAsync called: storeContextHandle=%p, async=%p\n", storeContextHandle, async);
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryGameLicenseResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreGameLicense *license )
{
    const char *store_id = getenv("XODUS_STORE_ID");
    if (!license) return E_POINTER;
    memset(license, 0, sizeof(*license));
    if (store_id && store_id[0])
    {
        lstrcpynA(license->skuStoreId, store_id, sizeof(license->skuStoreId));
    }
    else
    {
        strcpy(license->skuStoreId, "9P2N57MC619K");
    }
    license->isActive = TRUE;
    license->isTrial = FALSE;
    license->isDiscLicense = FALSE;
    license->isTrialOwnedByThisUser = FALSE;
    license->trialTimeRemainingInSeconds = 0;
    license->expirationDate = (time_t)0x7fffffff;
    fprintf(stderr, "[GDK XStore] XStoreQueryGameLicenseResult: skuStoreId='%s', isActive=%d\n",
            license->skuStoreId, license->isActive);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryAddOnLicensesAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, async %p\n", iface, storeContextHandle, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryAddOnLicensesResultCount( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 *count )
{
    TRACE( "iface %p, async %p, count %p\n", iface, async, count );
    if (count) *count = 0;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryAddOnLicensesResult( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 count, XStoreAddonLicense *addOnLicenses )
{
    TRACE( "iface %p, async %p, count %u, addOnLicenses %p\n", iface, async, count, addOnLicenses );
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryConsumableBalanceRemainingAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeProductId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeProductId %s, async %p\n", iface, storeContextHandle, debugstr_a( storeProductId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryConsumableBalanceRemainingResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreConsumableResult *consumableResult )
{
    TRACE( "iface %p, async %p, consumableResult %p\n", iface, async, consumableResult );
    if (consumableResult)
    {
        memset(consumableResult, 0, sizeof(*consumableResult));
        consumableResult->quantity = 0;
    }
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreReportConsumableFulfillmentAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeProductId, UINT32 quantity, GUID trackingId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeProductId %s, quantity %u, async %p\n", iface, storeContextHandle, debugstr_a( storeProductId ), quantity, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreReportConsumableFulfillmentResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreConsumableResult *consumableResult )
{
    TRACE( "iface %p, async %p, consumableResult %p\n", iface, async, consumableResult );
    if (consumableResult)
    {
        memset(consumableResult, 0, sizeof(*consumableResult));
        consumableResult->quantity = 0;
    }
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreGetUserCollectionsIdAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *serviceTicket, const char *publisherUserId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, serviceTicket %s, userId %s, async %p\n", iface, storeContextHandle, debugstr_a( serviceTicket ), debugstr_a( publisherUserId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreGetUserCollectionsIdResultSize( IXStoreImpl6 *iface, XAsyncBlock *async, SIZE_T *size )
{
    TRACE( "iface %p, async %p, size %p\n", iface, async, size );
    if (size) *size = 32;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreGetUserCollectionsIdResult( IXStoreImpl6 *iface, XAsyncBlock *async, SIZE_T size, char *result )
{
    TRACE( "iface %p, async %p, size %Iu, result %p\n", iface, async, size, result );
    if (result && size >= 16) strcpy(result, "MOCK_COLL_ID");
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreGetUserPurchaseIdAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *serviceTicket, const char *publisherUserId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, serviceTicket %s, publisherUserId %s, async %p\n", iface, storeContextHandle, debugstr_a( serviceTicket ), debugstr_a( publisherUserId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreGetUserPurchaseIdResultSize( IXStoreImpl6 *iface, XAsyncBlock *async, SIZE_T *size )
{
    TRACE( "iface %p, async %p, size %p\n", iface, async, size );
    if (size) *size = 32;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreGetUserPurchaseIdResult( IXStoreImpl6 *iface, XAsyncBlock *async, SIZE_T size, char *result )
{
    TRACE( "iface %p, async %p, size %Iu, result %p\n", iface, async, size, result );
    if (result && size >= 16) strcpy(result, "MOCK_PURCH_ID");
    return S_OK;
}

static char g_last_license_product_id[256] = {0};

static HRESULT WINAPI x_store_XStoreQueryLicenseTokenAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char **productIds, SIZE_T productIdsCount, const char *customDeveloperString, XAsyncBlock *async )
{
    if (productIdsCount > 0 && productIds && productIds[0] && productIds[0][0] != '\0') {
        lstrcpynA(g_last_license_product_id, productIds[0], sizeof(g_last_license_product_id));
    } else {
        lstrcpynA(g_last_license_product_id, "9P2N57MC619K", sizeof(g_last_license_product_id));
    }

    TRACE( "iface %p, storeContextHandle %p, idsCount %Iu (first ID: %s), custom %s, async %p\n",
           iface, storeContextHandle, productIdsCount, g_last_license_product_id, debugstr_a( customDeveloperString ), async );
    /* Always log at ERR level so we can capture product ID and customDeveloperString for debugging */
    ERR( "[XODUS-DIAG] XStoreQueryLicenseTokenAsync: productId='%s', customDeveloperString='%s'\n",
         g_last_license_product_id, customDeveloperString ? customDeveloperString : "(null)" );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryLicenseTokenResultSize( IXStoreImpl6 *iface, XAsyncBlock *async, SIZE_T *size )
{
    TRACE( "iface %p, async %p, size %p\n", iface, async, size );
    if (size) *size = 16384;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryLicenseTokenResult( IXStoreImpl6 *iface, XAsyncBlock *async, SIZE_T size, char *result )
{
    TRACE( "iface %p, async %p, size %Iu, result %p\n", iface, async, size, result );
    if (result && size >= 16) {
        UINT64 user_id = 1;
        TRACE("Requesting license token over IPC for product ID '%s'\n", g_last_license_product_id);
        ipc_xstore_query_license_token(user_id, g_last_license_product_id, result, size);
        /* Always log the token that gets returned (first 200 chars for debugging) */
        ERR( "[XODUS-DIAG] XStoreQueryLicenseTokenResult: returned token (first 200 chars): '%.200s'\n", result );
    }
    return S_OK;
}

static HRESULT WINAPI __PADDING__( IXStoreImpl6 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI __PADDING_2__( IXStoreImpl6 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI __PADDING_3__( IXStoreImpl6 *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreShowPurchaseUIAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeId, const char *name, const char *extendedJsonData, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, storeId %s, name %s, extendedJsonData %s, async %p stub!\n", iface, storeContextHandle, debugstr_a( storeId ), debugstr_a( name ), debugstr_a( extendedJsonData ), async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreShowPurchaseUIResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    FIXME( "iface %p, async %p stub!\n", iface, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreShowRateAndReviewUIAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, async %p stub!\n", iface, storeContextHandle, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreShowRateAndReviewUIResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreRateAndReviewResult *result )
{
    FIXME( "iface %p, async %p, result %p stub!\n", iface, async, result );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreShowRedeemTokenUIAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *token, const char **allowedStoreIds, SIZE_T allowedStoreIdsCount, BOOLEAN disallowCsvRedemption, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, token %s, allowedStoreIds %p, allowedStoreIdsCount %Iu, disallowCsvRedemption %d, async %p stub!\n", iface, storeContextHandle, debugstr_a( token ), allowedStoreIds, allowedStoreIdsCount, disallowCsvRedemption, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreShowRedeemTokenUIResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    FIXME( "iface %p, async %p stub!\n", iface, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreQueryGameAndDlcPackageUpdatesAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, async %p stub!\n", iface, storeContextHandle, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreQueryGameAndDlcPackageUpdatesResultCount( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 *count )
{
    FIXME( "iface %p, async %p, count %p stub!\n", iface, async, count );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreQueryGameAndDlcPackageUpdatesResult( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 count, XStorePackageUpdate *packageUpdates )
{
    FIXME( "iface %p, async %p, count %u, packageUpdates %p stub!\n", iface, async, count, packageUpdates );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadPackageUpdatesAsync( IXStoreImpl6 *iface, XStoreContextHandle storeContextHandle, const char **packageIdentifiers, SIZE_T packageIdentifiersCount, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, packageIdentifiers %p, packageIdentifiersCount %Iu, async %p stub!\n", iface, storeContextHandle, packageIdentifiers, packageIdentifiersCount, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadPackageUpdatesResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    FIXME( "iface %p, async %p stub!\n", iface, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadAndInstallPackageUpdatesAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char **packageIdentifiers, SIZE_T packageIdentifiersCount, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, packageIdentifiers %p, packageIdentifiersCount %Iu, async %p stub!\n", iface, storeContextHandle, packageIdentifiers, packageIdentifiersCount, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadAndInstallPackageUpdatesResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    FIXME( "iface %p, async %p stub!\n", iface, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadAndInstallPackagesAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char **storeIds, SIZE_T storeIdsCount, XAsyncBlock *async )
{
    FIXME( "iface %p, storeContextHandle %p, storeIds %p, storeIdsCount %Iu, async %p stub!\n", iface, storeContextHandle, storeIds, storeIdsCount, async );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadAndInstallPackagesResultCount( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 *count )
{
    FIXME( "iface %p, async %p, count %p stub!\n", iface, async, count );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreDownloadAndInstallPackagesResult( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 count, char **packageIdentifiers )
{
    FIXME( "iface %p, async %p, count %u, packageIdentifiers %p stub!\n", iface, async, count, packageIdentifiers );
    return E_NOTIMPL;
}

static HRESULT WINAPI x_store_XStoreQueryPackageIdentifier( IXStoreImpl6 *iface, const char *storeId, SIZE_T size, char *packageIdentifier )
{
    FIXME( "iface %p, storeId %s, size %Iu, packageIdentifier %p stub!\n", iface, debugstr_a( storeId ), size, packageIdentifier );
    return E_NOTIMPL;
}

struct license_callback_entry {
    UINT64 token;
    XStoreGameLicenseChangedCallback *game_callback;
    XStorePackageLicenseLostCallback *package_callback;
    void *context;
    XTaskQueueHandle queue;
    BOOL is_package;
};

static struct license_callback_entry g_license_callbacks[32];
static int g_license_callback_count = 0;
static UINT64 g_license_token_counter = 100;

static HRESULT WINAPI x_store_XStoreRegisterGameLicenseChanged( IXStoreImpl6 *iface, XStoreContextHandle storeContextHandle, XTaskQueueHandle queue, void *context, XStoreGameLicenseChangedCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, storeContextHandle %p, queue %p, context %p, callback %p, token %p\n", iface, storeContextHandle, queue, context, callback, token );
    if (!token) return E_POINTER;
    if (g_license_callback_count < 32)
    {
        g_license_callbacks[g_license_callback_count].token = ++g_license_token_counter;
        g_license_callbacks[g_license_callback_count].game_callback = callback;
        g_license_callbacks[g_license_callback_count].package_callback = NULL;
        g_license_callbacks[g_license_callback_count].context = context;
        g_license_callbacks[g_license_callback_count].queue = queue;
        g_license_callbacks[g_license_callback_count].is_package = FALSE;
        token->token = g_license_callbacks[g_license_callback_count].token;
        g_license_callback_count++;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

static BOOLEAN WINAPI x_store_XStoreUnregisterGameLicenseChanged( IXStoreImpl6 *iface, XStoreContextHandle storeContextHandle, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    int i;
    TRACE( "iface %p, storeContextHandle %p, token %llu, wait %d\n", iface, storeContextHandle, (unsigned long long)token.token, wait );
    for (i = 0; i < g_license_callback_count; i++)
    {
        if (g_license_callbacks[i].token == token.token)
        {
            g_license_callbacks[i] = g_license_callbacks[g_license_callback_count - 1];
            g_license_callback_count--;
            return TRUE;
        }
    }
    return TRUE;
}

static HRESULT WINAPI x_store_XStoreRegisterPackageLicenseLost( IXStoreImpl6 *iface, XStoreLicenseHandle storeLicenseHandle, XTaskQueueHandle queue, void *context, XStorePackageLicenseLostCallback *callback, XTaskQueueRegistrationToken *token)
{
    TRACE( "iface %p, storeLicenseHandle %p, queue %p, context %p, callback %p, token %p\n", iface, storeLicenseHandle, queue, context, callback, token );
    if (!token) return E_POINTER;
    if (g_license_callback_count < 32)
    {
        g_license_callbacks[g_license_callback_count].token = ++g_license_token_counter;
        g_license_callbacks[g_license_callback_count].game_callback = NULL;
        g_license_callbacks[g_license_callback_count].package_callback = callback;
        g_license_callbacks[g_license_callback_count].context = context;
        g_license_callbacks[g_license_callback_count].queue = queue;
        g_license_callbacks[g_license_callback_count].is_package = TRUE;
        token->token = g_license_callbacks[g_license_callback_count].token;
        g_license_callback_count++;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

static BOOLEAN WINAPI x_store_XStoreUnregisterPackageLicenseLost( IXStoreImpl6 *iface, XStoreLicenseHandle licenseHandle, XTaskQueueRegistrationToken token, BOOLEAN wait )
{
    int i;
    TRACE( "iface %p, licenseHandle %p, token %llu, wait %d\n", iface, licenseHandle, (unsigned long long)token.token, wait );
    for (i = 0; i < g_license_callback_count; i++)
    {
        if (g_license_callbacks[i].token == token.token)
        {
            g_license_callbacks[i] = g_license_callbacks[g_license_callback_count - 1];
            g_license_callback_count--;
            return TRUE;
        }
    }
    return TRUE;
}

static BOOLEAN WINAPI x_store_XStoreIsAvailabilityPurchasable( IXStoreImpl6 *iface, const XStoreAvailability availability )
{
    TRACE( "iface %p, availability %p\n", iface, &availability );
    return TRUE;
}

static HRESULT WINAPI x_store_XStoreAcquireLicenseForDurablesAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeId %s, async %p\n", iface, storeContextHandle, debugstr_a( storeId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreAcquireLicenseForDurablesResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreLicenseHandle *storeLicenseHandle )
{
    TRACE( "iface %p, async %p, storeLicenseHandle %p\n", iface, async, storeLicenseHandle );
    if (storeLicenseHandle) *storeLicenseHandle = (XStoreLicenseHandle)0x1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreShowAssociatedProductsUIAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeId, XStoreProductKind productKinds, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeId %s, productKinds %#x, async %p\n", iface, storeContextHandle, debugstr_a( storeId ), productKinds, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreShowAssociatedProductsUIResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p\n", iface, async );
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreShowProductPageUIAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeId, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeId %s, async %p\n", iface, storeContextHandle, debugstr_a( storeId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreShowProductPageUIResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p\n", iface, async );
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryAssociatedProductsForStoreIdAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeId, XStoreProductKind productKinds, UINT32 maxItemsToRetrievePerPage, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeId %s, productKinds %#x, maxItemsToRetrievePerPage %u, async %p\n", iface, storeContextHandle, debugstr_a( storeId ), productKinds, maxItemsToRetrievePerPage, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryAssociatedProductsForStoreIdResult( IXStoreImpl6 *iface, XAsyncBlock *async, XStoreProductQueryHandle *productQueryHandle )
{
    TRACE( "iface %p, async %p, productQueryHandle %p\n", iface, async, productQueryHandle );
    if (productQueryHandle) *productQueryHandle = (XStoreProductQueryHandle)0x1;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryPackageUpdatesAsync( IXStoreImpl6 *iface, XStoreContextHandle storeContextHandle, const char **packageIdentifiers, SIZE_T packageIdentifiersCount, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, packageIdentifiers %p, packageIdentifiersCount %Iu, async %p\n", iface, storeContextHandle, packageIdentifiers, packageIdentifiersCount, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryPackageUpdatesResultCount( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 *count )
{
    TRACE( "iface %p, async %p, count %p\n", iface, async, count );
    if (count) *count = 0;
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreQueryPackageUpdatesResult( IXStoreImpl6 *iface, XAsyncBlock *async, UINT32 count, XStorePackageUpdate *packageUpdates )
{
    TRACE( "iface %p, async %p, count %u, packageUpdates %p\n", iface, async, count, packageUpdates );
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreShowGiftingUIAsync( IXStoreImpl6 *iface, const XStoreContextHandle storeContextHandle, const char *storeId, const char *name, const char *extendedJsonData, XAsyncBlock *async )
{
    TRACE( "iface %p, storeContextHandle %p, storeId %s, name %s, extendedJsonData %s, async %p\n", iface, storeContextHandle, debugstr_a( storeId ), debugstr_a( name ), debugstr_a( extendedJsonData ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_store_XStoreShowGiftingUIResult( IXStoreImpl6 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p\n", iface, async );
    return S_OK;
}

static const struct IXStoreImpl6Vtbl x_store_vtbl =
{
    x_store_QueryInterface,
    x_store_AddRef,
    x_store_Release,
    /* IXStoreImpl methods */
    x_store_XStoreCreateContext,
    x_store_XStoreCloseContextHandle,
    x_store_XStoreQueryAssociatedProductsAsync,
    x_store_XStoreQueryAssociatedProductsResult,
    x_store_XStoreQueryProductsAsync,
    x_store_XStoreQueryProductsResult,
    x_store_XStoreQueryEntitledProductsAsync,
    x_store_XStoreQueryEntitledProductsResult,
    x_store_XStoreQueryProductForCurrentGameAsync,
    x_store_XStoreQueryProductForCurrentGameResult,
    x_store_XStoreQueryProductForPackageAsync,
    x_store_XStoreQueryProductForPackageResult,
    x_store_XStoreEnumerateProductsQuery,
    x_store_XStoreProductsQueryHasMorePages,
    x_store_XStoreProductsQueryNextPageAsync,
    x_store_XStoreProductsQueryNextPageResult,
    x_store_XStoreCloseProductsQueryHandle,
    x_store_XStoreAcquireLicenseForPackageAsync,
    x_store_XStoreAcquireLicenseForPackageResult,
    x_store_XStoreIsLicenseValid,
    x_store_XStoreCloseLicenseHandle,
    x_store_XStoreCanAcquireLicenseForStoreIdAsync,
    x_store_XStoreCanAcquireLicenseForStoreIdResult,
    x_store_XStoreCanAcquireLicenseForPackageAsync,
    x_store_XStoreCanAcquireLicenseForPackageResult,
    x_store_XStoreQueryGameLicenseAsync,
    x_store_XStoreQueryGameLicenseResult,
    x_store_XStoreQueryAddOnLicensesAsync,
    x_store_XStoreQueryAddOnLicensesResultCount,
    x_store_XStoreQueryAddOnLicensesResult,
    x_store_XStoreQueryConsumableBalanceRemainingAsync,
    x_store_XStoreQueryConsumableBalanceRemainingResult,
    x_store_XStoreReportConsumableFulfillmentAsync,
    x_store_XStoreReportConsumableFulfillmentResult,
    x_store_XStoreGetUserCollectionsIdAsync,
    x_store_XStoreGetUserCollectionsIdResultSize,
    x_store_XStoreGetUserCollectionsIdResult,
    x_store_XStoreGetUserPurchaseIdAsync,
    x_store_XStoreGetUserPurchaseIdResultSize,
    x_store_XStoreGetUserPurchaseIdResult,
    x_store_XStoreQueryLicenseTokenAsync,
    x_store_XStoreQueryLicenseTokenResultSize,
    x_store_XStoreQueryLicenseTokenResult,
    __PADDING__,
    __PADDING_2__,
    __PADDING_3__,
    x_store_XStoreShowPurchaseUIAsync,
    x_store_XStoreShowPurchaseUIResult,
    x_store_XStoreShowRateAndReviewUIAsync,
    x_store_XStoreShowRateAndReviewUIResult,
    x_store_XStoreShowRedeemTokenUIAsync,
    x_store_XStoreShowRedeemTokenUIResult,
    x_store_XStoreQueryGameAndDlcPackageUpdatesAsync,
    x_store_XStoreQueryGameAndDlcPackageUpdatesResultCount,
    x_store_XStoreQueryGameAndDlcPackageUpdatesResult,
    x_store_XStoreDownloadPackageUpdatesAsync,
    x_store_XStoreDownloadPackageUpdatesResult,
    x_store_XStoreDownloadAndInstallPackageUpdatesAsync,
    x_store_XStoreDownloadAndInstallPackageUpdatesResult,
    x_store_XStoreDownloadAndInstallPackagesAsync,
    x_store_XStoreDownloadAndInstallPackagesResultCount,
    x_store_XStoreDownloadAndInstallPackagesResult,
    x_store_XStoreQueryPackageIdentifier,
    x_store_XStoreRegisterGameLicenseChanged,
    x_store_XStoreUnregisterGameLicenseChanged,
    x_store_XStoreRegisterPackageLicenseLost,
    x_store_XStoreUnregisterPackageLicenseLost,
    /* IXStoreImpl2 methods */
    x_store_XStoreIsAvailabilityPurchasable,
    /* IXStoreImpl3 methods */
    x_store_XStoreAcquireLicenseForDurablesAsync,
    x_store_XStoreAcquireLicenseForDurablesResult,
    /* IXStoreImpl4 methods */
    x_store_XStoreShowAssociatedProductsUIAsync,
    x_store_XStoreShowAssociatedProductsUIResult,
    x_store_XStoreShowProductPageUIAsync,
    x_store_XStoreShowProductPageUIResult,
    /* IXStoreImpl5 methods */
    x_store_XStoreQueryAssociatedProductsForStoreIdAsync,
    x_store_XStoreQueryAssociatedProductsForStoreIdResult,
    x_store_XStoreQueryPackageUpdatesAsync,
    x_store_XStoreQueryPackageUpdatesResultCount,
    x_store_XStoreQueryPackageUpdatesResult,
    /* IXStoreImpl6 methods */
    x_store_XStoreShowGiftingUIAsync,
    x_store_XStoreShowGiftingUIResult,
};

static struct x_store x_store =
{
    {&x_store_vtbl},
    0,
};

IXStoreImpl *x_store_impl = (IXStoreImpl *)&x_store.IXStoreImpl6_iface;
