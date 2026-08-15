/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XAsync, XTaskQueue and XThread
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

#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

struct x_threading
{
    IXThreadingImpl IXThreadingImpl_iface;
    LONG ref;
};

static inline struct x_threading *impl_from_IXThreadingImpl( IXThreadingImpl *iface )
{
    return CONTAINING_RECORD( iface, struct x_threading, IXThreadingImpl_iface );
}

static HRESULT WINAPI x_threading_QueryInterface( IXThreadingImpl *iface, REFIID iid, void **out )
{
    struct x_threading *impl = impl_from_IXThreadingImpl( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown        ) ||
        IsEqualGUID( iid, &IID_IXThreadingImpl ))
    {
        IXThreadingImpl_AddRef( *out = &impl->IXThreadingImpl_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_threading_AddRef( IXThreadingImpl *iface )
{
    struct x_threading *impl = impl_from_IXThreadingImpl( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_threading_Release( IXThreadingImpl *iface )
{
    struct x_threading *impl = impl_from_IXThreadingImpl( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI x_threading_XAsyncGetStatus( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, BOOLEAN wait )
{
    TRACE( "iface %p, asyncBlock %p, wait %d\n", iface, asyncBlock, wait );
    return S_OK;
}

static HRESULT WINAPI x_threading_XAsyncGetResultSize( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, SIZE_T *bufferSize )
{
    TRACE( "iface %p, asyncBlock %p, bufferSize %p\n", iface, asyncBlock, bufferSize );
    if (bufferSize) *bufferSize = sizeof(void*);
    return S_OK;
}

static void WINAPI x_threading_XAsyncCancel( IXThreadingImpl *iface, XAsyncBlock *asyncBlock )
{
    TRACE( "iface %p, asyncBlock %p\n", iface, asyncBlock );
}

static HRESULT WINAPI x_threading_XAsyncRun( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, XAsyncWork *work )
{
    TRACE( "iface %p, asyncBlock %p, work %p\n", iface, asyncBlock, work );
    if (work) work(asyncBlock);
    if (asyncBlock && asyncBlock->callback) asyncBlock->callback(asyncBlock);
    return S_OK;
}

static HRESULT WINAPI x_threading_XAsyncBegin( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, void *context, const void *identity, const char *identityName, XAsyncProvider *provider )
{
    XAsyncProviderData data;
    TRACE( "iface %p, asyncBlock %p, context %p, identity %p, identityName %s, provider %p\n", iface, asyncBlock, context, identity, identityName, provider );
    memset(&data, 0, sizeof(data));
    data.async = asyncBlock;
    data.context = context;
    if (provider)
    {
        provider(XAsyncOp_Begin, &data);
    }
    return S_OK;
}


static HRESULT WINAPI __PADDING__( IXThreadingImpl *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return S_OK;
}

static HRESULT WINAPI x_threading_XAsyncSchedule( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, UINT32 delayInMs )
{
    TRACE( "iface %p, asyncBlock %p, delayInMs %d\n", iface, asyncBlock, delayInMs );
    if (asyncBlock && asyncBlock->callback) asyncBlock->callback(asyncBlock);
    return S_OK;
}

static void WINAPI x_threading_XAsyncComplete( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, HRESULT result, SIZE_T requiredBufferSize )
{
    TRACE( "iface %p, asyncBlock %p, result %#lx, requiredBufferSize %Iu\n", iface, asyncBlock, result, requiredBufferSize );
    if (asyncBlock && asyncBlock->callback) asyncBlock->callback(asyncBlock);
}

static HRESULT WINAPI x_threading_XAsyncGetResult( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, const void *identity, SIZE_T bufferSize, void *buffer, SIZE_T *bufferUsed )
{
    TRACE( "iface %p asyncBlock %p, identity %p, bufferSize %Iu, buffer %p, bufferUsed %p\n", iface, asyncBlock, identity, bufferSize, buffer, bufferUsed );
    if (bufferUsed) *bufferUsed = bufferSize;
    return S_OK;
}


struct task_queue
{
    XTaskQueueDispatchMode work_mode;
    XTaskQueueDispatchMode comp_mode;
    LONG ref;
};

static struct task_queue default_process_queue = { 0, 0, 1 };

static HRESULT WINAPI x_threading_XTaskQueueCreate( IXThreadingImpl *iface, XTaskQueueDispatchMode workDispatchMode, XTaskQueueDispatchMode completionDispatchMode, XTaskQueueHandle *queue )
{
    struct task_queue *tq;
    TRACE( "iface %p, workDispatchMode %d, completionDispatchMode %d, queue %p\n", iface, workDispatchMode, completionDispatchMode, queue );
    if (!queue) return E_POINTER;
    tq = malloc( sizeof(*tq) );
    if (!tq) return E_OUTOFMEMORY;
    tq->work_mode = workDispatchMode;
    tq->comp_mode = completionDispatchMode;
    tq->ref = 1;
    *queue = (XTaskQueueHandle)tq;
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueCreateComposite( IXThreadingImpl *iface, XTaskQueuePortHandle workPort, XTaskQueuePortHandle completionPort, XTaskQueueHandle *queue )
{
    struct task_queue *tq;
    TRACE( "iface %p, workPort %p, completionPort %p, queue %p\n", iface, workPort, completionPort, queue );
    if (!queue) return E_POINTER;
    tq = malloc( sizeof(*tq) );
    if (!tq) return E_OUTOFMEMORY;
    tq->work_mode = 0;
    tq->comp_mode = 0;
    tq->ref = 1;
    *queue = (XTaskQueueHandle)tq;
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueGetPort( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, XTaskQueuePortHandle *portHandle )
{
    TRACE( "iface %p, queue %p, port %d, portHandle %p\n", iface, queue, port, portHandle );
    if (!portHandle) return E_POINTER;
    *portHandle = (XTaskQueuePortHandle)queue;
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueDuplicateHandle( IXThreadingImpl *iface, XTaskQueueHandle queueHandle, XTaskQueueHandle *duplicatedHandle )
{
    TRACE( "iface %p, queueHandle %p, duplicatedHandle %p\n", iface, queueHandle, duplicatedHandle );
    if (!duplicatedHandle) return E_POINTER;
    if (queueHandle && queueHandle != (XTaskQueueHandle)&default_process_queue)
    {
        struct task_queue *tq = (struct task_queue *)queueHandle;
        InterlockedIncrement( &tq->ref );
    }
    *duplicatedHandle = queueHandle;
    return S_OK;
}

static BOOLEAN WINAPI x_threading_XTaskQueueDispatch( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, UINT32 timeoutInMs )
{
    TRACE( "iface %p, queue %p, port %d, timeoutInMs %d\n", iface, queue, port, timeoutInMs );
    return FALSE;
}

static void WINAPI x_threading_XTaskQueueCloseHandle( IXThreadingImpl *iface, XTaskQueueHandle queue )
{
    TRACE( "iface %p, queue %p\n", iface, queue );
    if (queue && queue != (XTaskQueueHandle)&default_process_queue)
    {
        struct task_queue *tq = (struct task_queue *)queue;
        if (InterlockedDecrement( &tq->ref ) == 0)
        {
            free( tq );
        }
    }
}

static HRESULT WINAPI x_threading_XTaskQueueSubmitCallback( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, void *callbackContext, XTaskQueueCallback *callback )
{
    TRACE( "iface %p, queue %p, port %d, callbackContext %p, callback %p\n", iface, queue, port, callbackContext, callback );
    if (callback)
    {
        callback( callbackContext, FALSE );
    }
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueSubmitDelayedCallback( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, UINT32 delayMs, void *callbackContext, XTaskQueueCallback *callback )
{
    TRACE( "iface %p, queue %p, port %d, delayMs %d, callbackContext %p, callback %p\n", iface, queue, port, delayMs, callbackContext, callback );
    if (callback)
    {
        callback( callbackContext, FALSE );
    }
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueRegisterWaiter( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, HANDLE waitHandle, void *callbackContext, XTaskQueueCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, port %d, waitHandle %p, callbackContext %p, callback %p, token %p\n", iface, queue, port, waitHandle, callbackContext, callback, token );
    if (token) token->token = 1;
    return S_OK;
}

static void WINAPI x_threading_XTaskQueueUnregisterWaiter( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueueRegistrationToken token )
{
    TRACE( "iface %p, queue %p, token %p\n", iface, queue, &token );
}

static HRESULT WINAPI x_threading_XTaskQueueTerminate( IXThreadingImpl *iface, XTaskQueueHandle queue, BOOLEAN wait, void *callbackContext, XTaskQueueTerminatedCallback *callback )
{
    TRACE( "iface %p, queue %p, wait %d, callbackContext %p, callback %p\n", iface, queue, wait, callbackContext, callback );
    if (callback) callback( callbackContext );
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueRegisterMonitor( IXThreadingImpl *iface, XTaskQueueHandle queue, void *callbackContext, XTaskQueueMonitorCallback *callback, XTaskQueueRegistrationToken *token )
{
    TRACE( "iface %p, queue %p, callbackContext %p, callback %p, token %p\n", iface, queue, callbackContext, callback, token );
    if (token) token->token = 1;
    return S_OK;
}

static void WINAPI x_threading_XTaskQueueUnregisterMonitor( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueueRegistrationToken token )
{
    TRACE( "iface %p, queue %p, token %p\n", iface, queue, &token );
}

static BOOLEAN WINAPI x_threading_XTaskQueueGetCurrentProcessTaskQueue( IXThreadingImpl *iface, XTaskQueueHandle *queue )
{
    TRACE( "iface %p, queue %p\n", iface, queue );
    if (!queue) return FALSE;
    *queue = (XTaskQueueHandle)&default_process_queue;
    return TRUE;
}

static void WINAPI x_threading_XTaskQueueSetCurrentProcessTaskQueue( IXThreadingImpl *iface, XTaskQueueHandle queue )
{
    TRACE( "iface %p, queue %p\n", iface, queue );
}


static HRESULT WINAPI x_threading_XThreadSetTimeSensitive( IXThreadingImpl *iface, BOOLEAN isTimeSensitiveThread )
{
    TRACE( "iface %p, isTimeSensitiveThread %d.\n", iface, isTimeSensitiveThread );
    if (!TlsSetValue( tlsIndex, (void *)(UINT_PTR)isTimeSensitiveThread )) return HRESULT_FROM_WIN32( GetLastError() );
    return S_OK;
}

static void WINAPI x_threading_XThreadAssertNotTimeSensitive( IXThreadingImpl *iface )
{
    TRACE( "iface %p.\n", iface );
    if (TlsGetValue( tlsIndex )) DebugBreak();
}

static BOOLEAN WINAPI x_threading_XThreadIsTimeSensitive( IXThreadingImpl *iface )
{
    TRACE( "iface %p.\n", iface );
    return TlsGetValue( tlsIndex ) ? 1 : 0;
}

static const struct IXThreadingImplVtbl x_threading_vtbl =
{
    x_threading_QueryInterface,
    x_threading_AddRef,
    x_threading_Release,
    /* IXThreadingImpl methods */
    x_threading_XAsyncGetStatus,
    x_threading_XAsyncGetResultSize,
    x_threading_XAsyncCancel,
    x_threading_XAsyncRun,
    x_threading_XAsyncBegin,
    __PADDING__,
    x_threading_XAsyncSchedule,
    x_threading_XAsyncComplete,
    x_threading_XAsyncGetResult,
    x_threading_XTaskQueueCreate,
    x_threading_XTaskQueueCreateComposite,
    x_threading_XTaskQueueGetPort,
    x_threading_XTaskQueueDuplicateHandle,
    x_threading_XTaskQueueDispatch,
    x_threading_XTaskQueueCloseHandle,
    x_threading_XTaskQueueSubmitCallback,
    x_threading_XTaskQueueSubmitDelayedCallback,
    x_threading_XTaskQueueRegisterWaiter,
    x_threading_XTaskQueueUnregisterWaiter,
    x_threading_XTaskQueueTerminate,
    x_threading_XTaskQueueRegisterMonitor,
    x_threading_XTaskQueueUnregisterMonitor,
    x_threading_XTaskQueueGetCurrentProcessTaskQueue,
    x_threading_XTaskQueueSetCurrentProcessTaskQueue,
    x_threading_XThreadSetTimeSensitive,
    __PADDING__,
    x_threading_XThreadAssertNotTimeSensitive,
    x_threading_XThreadIsTimeSensitive
};

static struct x_threading x_threading =
{
    {&x_threading_vtbl},
    0,
};

IXThreadingImpl *x_threading_impl = &x_threading.IXThreadingImpl_iface;
