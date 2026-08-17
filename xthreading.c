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

struct async_internal
{
    XAsyncProvider *provider;
    void *context;
    const void *identity;
    HRESULT result;
    SIZE_T requiredBufferSize;
    BOOL completed;
    BOOL cleaned;
};

struct task_queue;
static struct task_queue default_process_queue;
static XTaskQueueHandle g_current_process_queue;
static void CALLBACK async_callback_thunk(void *context, BOOLEAN canceled);
static HRESULT WINAPI x_threading_XTaskQueueSubmitCallback( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, void *callbackContext, XTaskQueueCallback *callback );
static HRESULT WINAPI x_threading_XTaskQueueSubmitDelayedCallback( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, UINT32 delayMs, void *callbackContext, XTaskQueueCallback *callback );

static HRESULT WINAPI x_threading_XAsyncGetStatus( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, BOOLEAN wait )
{
    struct async_internal *state;
    TRACE( "iface %p, asyncBlock %p, wait %d\n", iface, asyncBlock, wait );
    if (!asyncBlock) return E_POINTER;
    state = *(struct async_internal**)&asyncBlock->internal[0];
    if (!state) return S_OK;
    return state->completed ? state->result : E_PENDING;
}

static HRESULT WINAPI x_threading_XAsyncGetResultSize( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, SIZE_T *bufferSize )
{
    struct async_internal *state;
    TRACE( "iface %p, asyncBlock %p, bufferSize %p\n", iface, asyncBlock, bufferSize );
    if (!asyncBlock || !bufferSize) return E_POINTER;
    state = *(struct async_internal**)&asyncBlock->internal[0];
    if (state)
    {
        *bufferSize = state->requiredBufferSize;
        return state->result;
    }
    *bufferSize = sizeof(void*);
    return S_OK;
}

static void WINAPI x_threading_XAsyncCancel( IXThreadingImpl *iface, XAsyncBlock *asyncBlock )
{
    struct async_internal *state;
    TRACE( "iface %p, asyncBlock %p\n", iface, asyncBlock );
    if (!asyncBlock) return;
    state = *(struct async_internal**)&asyncBlock->internal[0];
    if (state && state->provider)
    {
        XAsyncProviderData data = {0};
        data.async = asyncBlock;
        data.context = state->context;
        state->provider(XAsyncOp_Cancel, &data);
        if (!state->cleaned)
        {
            state->provider(XAsyncOp_Cleanup, &data);
            state->cleaned = TRUE;
        }
        free(state);
        *(struct async_internal**)&asyncBlock->internal[0] = NULL;
    }
}

static HRESULT WINAPI x_threading_XAsyncRun( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, XAsyncWork *work )
{
    TRACE( "iface %p, asyncBlock %p, work %p\n", iface, asyncBlock, work );
    if (work) work(asyncBlock);
    if (asyncBlock && asyncBlock->callback)
    {
        if (asyncBlock->queue)
        {
            x_threading_XTaskQueueSubmitCallback(iface, asyncBlock->queue, XTaskQueuePort_Completion, asyncBlock, (XTaskQueueCallback*)asyncBlock->callback);
        }
        else
        {
            asyncBlock->callback(asyncBlock);
        }
    }
    return S_OK;
}

static HRESULT WINAPI x_threading_XAsyncBegin( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, void *context, const void *identity, const char *identityName, XAsyncProvider *provider )
{
    struct async_internal *state;
    XAsyncProviderData data;
    HRESULT hr;

    TRACE( "iface %p, asyncBlock %p, context %p, identity %p, identityName %s, provider %p\n", iface, asyncBlock, context, identity, identityName, provider );
    if (!asyncBlock) return E_POINTER;

    state = calloc(1, sizeof(*state));
    if (!state) return E_OUTOFMEMORY;
    state->provider = provider;
    state->context = context;
    state->identity = identity;
    state->result = E_PENDING;
    *(struct async_internal**)&asyncBlock->internal[0] = state;

    if (provider)
    {
        memset(&data, 0, sizeof(data));
        data.async = asyncBlock;
        data.context = context;
        hr = provider(XAsyncOp_Begin, &data);
        if (FAILED(hr))
        {
            free(state);
            *(struct async_internal**)&asyncBlock->internal[0] = NULL;
            return hr;
        }
    }
    return S_OK;
}


static HRESULT WINAPI __PADDING__( IXThreadingImpl *iface )
{
    WARN( "iface %p padding function called! It's unknown what this function does.\n", iface );
    return S_OK;
}

struct async_work_thunk_data {
    XAsyncBlock *async;
    XAsyncProvider *provider;
    void *context;
};

static void CALLBACK async_work_thunk(void *context, BOOLEAN canceled)
{
    struct async_work_thunk_data *thunk = (struct async_work_thunk_data *)context;
    if (thunk)
    {
        if (thunk->provider)
        {
            XAsyncProviderData data = {0};
            data.async = thunk->async;
            data.context = thunk->context;
            thunk->provider(canceled ? XAsyncOp_Cancel : XAsyncOp_DoWork, &data);
        }
        free(thunk);
    }
}

static HRESULT WINAPI x_threading_XAsyncSchedule( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, UINT32 delayInMs )
{
    struct async_internal *state;
    TRACE( "iface %p, asyncBlock %p, delayInMs %d\n", iface, asyncBlock, delayInMs );
    if (!asyncBlock) return E_POINTER;
    state = *(struct async_internal**)&asyncBlock->internal[0];
    if (state && state->provider)
    {
        struct async_work_thunk_data *thunk = malloc(sizeof(*thunk));
        if (!thunk) return E_OUTOFMEMORY;
        thunk->async = asyncBlock;
        thunk->provider = state->provider;
        thunk->context = state->context;
        XTaskQueueHandle queue = asyncBlock->queue ? asyncBlock->queue : (g_current_process_queue ? g_current_process_queue : (XTaskQueueHandle)&default_process_queue);
        if (delayInMs > 0)
        {
            return x_threading_XTaskQueueSubmitDelayedCallback(iface, queue, XTaskQueuePort_Work, delayInMs, thunk, async_work_thunk);
        }
        else
        {
            return x_threading_XTaskQueueSubmitCallback(iface, queue, XTaskQueuePort_Work, thunk, async_work_thunk);
        }
    }
    else
    {
        if (asyncBlock->callback)
        {
            XTaskQueueHandle queue = asyncBlock->queue ? asyncBlock->queue : (g_current_process_queue ? g_current_process_queue : (XTaskQueueHandle)&default_process_queue);
            x_threading_XTaskQueueSubmitCallback(iface, queue, XTaskQueuePort_Completion, asyncBlock, async_callback_thunk);
        }
    }
    return S_OK;
}

static void WINAPI x_threading_XAsyncComplete( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, HRESULT result, SIZE_T requiredBufferSize )
{
    struct async_internal *state;
    TRACE( "iface %p, asyncBlock %p, result %#lx, requiredBufferSize %Iu\n", iface, asyncBlock, result, requiredBufferSize );
    if (!asyncBlock) return;
    state = *(struct async_internal**)&asyncBlock->internal[0];
    if (state)
    {
        state->completed = TRUE;
        state->result = result;
        state->requiredBufferSize = requiredBufferSize;
    }
    if (asyncBlock->callback)
    {
        XTaskQueueHandle queue = asyncBlock->queue ? asyncBlock->queue : (g_current_process_queue ? g_current_process_queue : (XTaskQueueHandle)&default_process_queue);
        x_threading_XTaskQueueSubmitCallback(iface, queue, XTaskQueuePort_Completion, asyncBlock, async_callback_thunk);
    }
}

static HRESULT WINAPI x_threading_XAsyncGetResult( IXThreadingImpl *iface, XAsyncBlock *asyncBlock, const void *identity, SIZE_T bufferSize, void *buffer, SIZE_T *bufferUsed )
{
    struct async_internal *state;
    HRESULT hr = S_OK;

    TRACE( "iface %p asyncBlock %p, identity %p, bufferSize %Iu, buffer %p, bufferUsed %p\n", iface, asyncBlock, identity, bufferSize, buffer, bufferUsed );
    if (!asyncBlock) return E_POINTER;
    state = *(struct async_internal**)&asyncBlock->internal[0];

    if (state && state->provider)
    {
        XAsyncProviderData data = {0};
        data.async = asyncBlock;
        data.context = state->context;
        data.buffer = buffer;
        data.bufferSize = bufferSize;
        hr = state->provider(XAsyncOp_GetResult, &data);
        if (bufferUsed) *bufferUsed = data.bufferSize;
        if (!state->cleaned)
        {
            state->provider(XAsyncOp_Cleanup, &data);
            state->cleaned = TRUE;
        }
        free(state);
        *(struct async_internal**)&asyncBlock->internal[0] = NULL;
    }
    else
    {
        if (buffer && bufferSize > 0) memset(buffer, 0, bufferSize);
        if (bufferUsed) *bufferUsed = bufferSize;
    }
    return hr;
}


struct task_callback_entry
{
    void *context;
    XTaskQueueCallback *callback;
    UINT32 delay_ms;
    struct task_callback_entry *next;
};

struct task_queue
{
    XTaskQueueDispatchMode work_mode;
    XTaskQueueDispatchMode comp_mode;
    LONG ref;
    struct task_callback_entry *work_head;
    struct task_callback_entry *work_tail;
    struct task_callback_entry *comp_head;
    struct task_callback_entry *comp_tail;
    struct task_queue *work_parent;
    struct task_queue *comp_parent;
    XTaskQueueMonitorCallback *monitor_callback;
    void *monitor_context;
    CRITICAL_SECTION cs;
    BOOL cs_inited;
};

static struct task_queue default_process_queue = {
    XTaskQueueDispatchMode_ThreadPool,
    XTaskQueueDispatchMode_ThreadPool,
    1,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, {0}, FALSE
};

static XTaskQueueHandle g_current_process_queue = NULL;

static inline struct task_queue *get_effective_queue_ptr(XTaskQueueHandle queue)
{
    if (queue) return (struct task_queue *)queue;
    if (g_current_process_queue) return (struct task_queue *)g_current_process_queue;
    return &default_process_queue;
}

static void init_queue_cs(struct task_queue *tq)
{
    if (!tq->cs_inited)
    {
        InitializeCriticalSection(&tq->cs);
        tq->cs_inited = TRUE;
    }
}

static HRESULT WINAPI x_threading_XTaskQueueCreate( IXThreadingImpl *iface, XTaskQueueDispatchMode workDispatchMode, XTaskQueueDispatchMode completionDispatchMode, XTaskQueueHandle *queue )
{
    struct task_queue *tq;
    TRACE( "iface %p, workDispatchMode %d, completionDispatchMode %d, queue %p\n", iface, workDispatchMode, completionDispatchMode, queue );
    if (!queue) return E_POINTER;
    tq = calloc( 1, sizeof(*tq) );
    if (!tq) return E_OUTOFMEMORY;
    tq->work_mode = workDispatchMode;
    tq->comp_mode = completionDispatchMode;
    tq->ref = 1;
    init_queue_cs(tq);
    *queue = (XTaskQueueHandle)tq;
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueCreateComposite( IXThreadingImpl *iface, XTaskQueuePortHandle workPort, XTaskQueuePortHandle completionPort, XTaskQueueHandle *queue )
{
    struct task_queue *tq;
    TRACE( "iface %p, workPort %p, completionPort %p, queue %p\n", iface, workPort, completionPort, queue );
    if (!queue) return E_POINTER;
    tq = calloc( 1, sizeof(*tq) );
    if (!tq) return E_OUTOFMEMORY;
    if (workPort)
    {
        struct task_queue *wq = (struct task_queue *)workPort;
        tq->work_mode = wq->work_mode;
        tq->work_parent = wq;
    }
    else
    {
        tq->work_mode = XTaskQueueDispatchMode_ThreadPool;
    }
    if (completionPort)
    {
        struct task_queue *cq = (struct task_queue *)completionPort;
        tq->comp_mode = cq->comp_mode;
        tq->comp_parent = cq;
    }
    else
    {
        tq->comp_mode = XTaskQueueDispatchMode_ThreadPool;
    }
    tq->ref = 1;
    init_queue_cs(tq);
    *queue = (XTaskQueueHandle)tq;
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueGetPort( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, XTaskQueuePortHandle *portHandle )
{
    TRACE( "iface %p, queue %p, port %d, portHandle %p\n", iface, queue, port, portHandle );
    if (!portHandle) return E_POINTER;
    struct task_queue *tq = get_effective_queue_ptr(queue);
    if (port == XTaskQueuePort_Work && tq->work_parent) tq = tq->work_parent;
    if (port == XTaskQueuePort_Completion && tq->comp_parent) tq = tq->comp_parent;
    *portHandle = (XTaskQueuePortHandle)tq;
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
    struct task_queue *tq = get_effective_queue_ptr(queue);
    struct task_callback_entry *entry = NULL;

    if (port == XTaskQueuePort_Work && tq->work_parent) tq = tq->work_parent;
    if (port == XTaskQueuePort_Completion && tq->comp_parent) tq = tq->comp_parent;

    init_queue_cs(tq);
    EnterCriticalSection(&tq->cs);

    if (port == XTaskQueuePort_Work)
    {
        if (tq->work_head)
        {
            entry = tq->work_head;
            tq->work_head = entry->next;
            if (!tq->work_head) tq->work_tail = NULL;
        }
    }
    else
    {
        if (tq->comp_head)
        {
            entry = tq->comp_head;
            tq->comp_head = entry->next;
            if (!tq->comp_head) tq->comp_tail = NULL;
        }
    }

    LeaveCriticalSection(&tq->cs);

    if (entry)
    {
        TRACE( "Dispatching callback %p (ctx %p) on port %d\n", entry->callback, entry->context, port );
        if (entry->callback)
        {
            entry->callback( entry->context, FALSE );
        }
        free( entry );
        return TRUE;
    }

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
            struct task_callback_entry *cur, *next;
            init_queue_cs(tq);
            EnterCriticalSection(&tq->cs);
            cur = tq->work_head;
            while (cur) { next = cur->next; free(cur); cur = next; }
            cur = tq->comp_head;
            while (cur) { next = cur->next; free(cur); cur = next; }
            LeaveCriticalSection(&tq->cs);
            DeleteCriticalSection(&tq->cs);
            free( tq );
        }
    }
}

static DWORD WINAPI threadpool_callback_runner(void *param)
{
    struct task_callback_entry *entry = (struct task_callback_entry *)param;
    if (entry)
    {
        if (entry->delay_ms > 0)
        {
            Sleep(entry->delay_ms);
        }
        if (entry->callback)
        {
            entry->callback(entry->context, FALSE);
        }
        free(entry);
    }
    return 0;
}

static HRESULT WINAPI x_threading_XTaskQueueSubmitCallback( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, void *callbackContext, XTaskQueueCallback *callback )
{
    struct task_queue *tq = get_effective_queue_ptr(queue);
    struct task_callback_entry *entry;
    XTaskQueueDispatchMode mode;
    TRACE( "iface %p, queue %p (effective %p), port %d, callbackContext %p, callback %p\n", iface, queue, tq, port, callbackContext, callback );

    if (!callback) return S_OK;

    if (port == XTaskQueuePort_Work && tq->work_parent) tq = tq->work_parent;
    if (port == XTaskQueuePort_Completion && tq->comp_parent) tq = tq->comp_parent;

    mode = (port == XTaskQueuePort_Work) ? tq->work_mode : tq->comp_mode;

    entry = malloc( sizeof(*entry) );
    if (!entry) return E_OUTOFMEMORY;
    entry->context = callbackContext;
    entry->callback = callback;
    entry->delay_ms = 0;
    entry->next = NULL;

    if (mode == XTaskQueueDispatchMode_Immediate)
    {
        callback(callbackContext, FALSE);
        free(entry);
        return S_OK;
    }
    else if (mode == XTaskQueueDispatchMode_ThreadPool || mode == XTaskQueueDispatchMode_SerializedThreadPool)
    {
        HANDLE hThread = CreateThread(NULL, 0, threadpool_callback_runner, entry, 0, NULL);
        if (hThread)
        {
            CloseHandle(hThread);
            return S_OK;
        }
        /* Fallback */
        callback(callbackContext, FALSE);
        free(entry);
        return S_OK;
    }

    init_queue_cs(tq);
    EnterCriticalSection(&tq->cs);

    if (port == XTaskQueuePort_Work)
    {
        if (tq->work_tail)
            tq->work_tail->next = entry;
        else
            tq->work_head = entry;
        tq->work_tail = entry;
    }
    else
    {
        if (tq->comp_tail)
            tq->comp_tail->next = entry;
        else
            tq->comp_head = entry;
        tq->comp_tail = entry;
    }

    LeaveCriticalSection(&tq->cs);

    if (tq->monitor_callback)
    {
        tq->monitor_callback(tq->monitor_context, (XTaskQueueHandle)tq, port);
    }
    return S_OK;
}

static HRESULT WINAPI x_threading_XTaskQueueSubmitDelayedCallback( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueuePort port, UINT32 delayMs, void *callbackContext, XTaskQueueCallback *callback )
{
    struct task_queue *tq = get_effective_queue_ptr(queue);
    struct task_callback_entry *entry;
    XTaskQueueDispatchMode mode;
    TRACE( "iface %p, queue %p (effective %p), port %d, delayMs %d, callbackContext %p, callback %p\n", iface, queue, tq, port, delayMs, callbackContext, callback );

    if (!callback) return S_OK;

    if (port == XTaskQueuePort_Work && tq->work_parent) tq = tq->work_parent;
    if (port == XTaskQueuePort_Completion && tq->comp_parent) tq = tq->comp_parent;

    mode = (port == XTaskQueuePort_Work) ? tq->work_mode : tq->comp_mode;

    entry = malloc( sizeof(*entry) );
    if (!entry) return E_OUTOFMEMORY;
    entry->context = callbackContext;
    entry->callback = callback;
    entry->delay_ms = delayMs;
    entry->next = NULL;

    if (mode == XTaskQueueDispatchMode_ThreadPool || mode == XTaskQueueDispatchMode_SerializedThreadPool || mode == XTaskQueueDispatchMode_Immediate)
    {
        HANDLE hThread = CreateThread(NULL, 0, threadpool_callback_runner, entry, 0, NULL);
        if (hThread)
        {
            CloseHandle(hThread);
            return S_OK;
        }
        if (delayMs > 0) Sleep(delayMs);
        callback(callbackContext, FALSE);
        free(entry);
        return S_OK;
    }

    init_queue_cs(tq);
    EnterCriticalSection(&tq->cs);

    if (port == XTaskQueuePort_Work)
    {
        if (tq->work_tail)
            tq->work_tail->next = entry;
        else
            tq->work_head = entry;
        tq->work_tail = entry;
    }
    else
    {
        if (tq->comp_tail)
            tq->comp_tail->next = entry;
        else
            tq->comp_head = entry;
        tq->comp_tail = entry;
    }

    LeaveCriticalSection(&tq->cs);

    if (tq->monitor_callback)
    {
        tq->monitor_callback(tq->monitor_context, (XTaskQueueHandle)tq, port);
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
    struct task_queue *tq = get_effective_queue_ptr(queue);
    TRACE( "iface %p, queue %p, callbackContext %p, callback %p, token %p\n", iface, queue, callbackContext, callback, token );
    init_queue_cs(tq);
    EnterCriticalSection(&tq->cs);
    tq->monitor_callback = callback;
    tq->monitor_context = callbackContext;
    LeaveCriticalSection(&tq->cs);
    if (token) token->token = 1;
    return S_OK;
}

static void WINAPI x_threading_XTaskQueueUnregisterMonitor( IXThreadingImpl *iface, XTaskQueueHandle queue, XTaskQueueRegistrationToken token )
{
    struct task_queue *tq = get_effective_queue_ptr(queue);
    TRACE( "iface %p, queue %p, token %p\n", iface, queue, &token );
    init_queue_cs(tq);
    EnterCriticalSection(&tq->cs);
    tq->monitor_callback = NULL;
    tq->monitor_context = NULL;
    LeaveCriticalSection(&tq->cs);
}

static BOOLEAN WINAPI x_threading_XTaskQueueGetCurrentProcessTaskQueue( IXThreadingImpl *iface, XTaskQueueHandle *queue )
{
    TRACE( "iface %p, queue %p\n", iface, queue );
    if (!queue) return FALSE;
    *queue = g_current_process_queue ? g_current_process_queue : (XTaskQueueHandle)&default_process_queue;
    return TRUE;
}

static void WINAPI x_threading_XTaskQueueSetCurrentProcessTaskQueue( IXThreadingImpl *iface, XTaskQueueHandle queue )
{
    TRACE( "iface %p, queue %p\n", iface, queue );
    g_current_process_queue = queue;
}

static void CALLBACK async_callback_thunk(void *context, BOOLEAN canceled)
{
    XAsyncBlock *async = (XAsyncBlock *)context;
    if (async && async->callback)
    {
        TRACE( "Invoking async callback %p on async block %p\n", async->callback, async );
        async->callback(async);
    }
}

void complete_async(XAsyncBlock *async)
{
    struct async_internal *state;
    if (!async) return;
    state = *(struct async_internal**)&async->internal[0];
    if (!state)
    {
        state = calloc(1, sizeof(*state));
        *(struct async_internal**)&async->internal[0] = state;
    }
    state->completed = TRUE;
    state->result = S_OK;

    XTaskQueueHandle queue = async->queue ? async->queue : (g_current_process_queue ? g_current_process_queue : (XTaskQueueHandle)&default_process_queue);
    x_threading_XTaskQueueSubmitCallback(x_threading_impl, queue, XTaskQueuePort_Completion, async, async_callback_thunk);
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
