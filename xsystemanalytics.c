/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XSystem
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

#include <wchar.h>
#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);


struct x_system_analytics
{
    IXSystemAnalyticsImpl IXSystemAnalyticsImpl_iface;
    LONG ref;
};

static inline struct x_system_analytics *impl_from_IXSystemAnalyticsImpl( IXSystemAnalyticsImpl *iface )
{
    return CONTAINING_RECORD( iface, struct x_system_analytics, IXSystemAnalyticsImpl_iface );
}

static HRESULT WINAPI x_system_analytics_QueryInterface( IXSystemAnalyticsImpl *iface, REFIID iid, void **out )
{
    struct x_system_analytics *impl = impl_from_IXSystemAnalyticsImpl( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown              ) ||
        IsEqualGUID( iid, &IID_IXSystemAnalyticsImpl ))
    {
        IXSystemAnalyticsImpl_AddRef( *out = &impl->IXSystemAnalyticsImpl_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_system_analytics_AddRef( IXSystemAnalyticsImpl *iface )
{
    struct x_system_analytics *impl = impl_from_IXSystemAnalyticsImpl( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_system_analytics_Release( IXSystemAnalyticsImpl *iface )
{
    struct x_system_analytics *impl = impl_from_IXSystemAnalyticsImpl( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static XSystemAnalyticsInfo* WINAPI x_system_analytics_XSystemGetAnalyticsInfo( IXSystemAnalyticsImpl *iface, XSystemAnalyticsInfo *__ret )
{
    /* For Windows, XSystemAnalyticsInfo->form is always "Desktop" */
    XSystemAnalyticsInfo info;

    TRACE( "iface %p.\n", iface );

    if (!__ret) return NULL;

    memset( &info, 0, sizeof(info) );
    strcpy( info.family, "Windows" );
    strcpy( info.form, "Desktop" );
    info.osVersion.major = 10;
    info.osVersion.minor = 0;
    info.osVersion.build = 22631;
    info.osVersion.revision = 3880;
    info.hostingOsVersion = info.osVersion;

    *__ret = info;
    return __ret;
}

static const struct IXSystemAnalyticsImplVtbl x_system_analytics_vtbl =
{
    x_system_analytics_QueryInterface,
    x_system_analytics_AddRef,
    x_system_analytics_Release,
    /* IXSystemAnalyticsImpl methods */
    x_system_analytics_XSystemGetAnalyticsInfo,
};

static struct x_system_analytics x_system_analytics =
{
    {&x_system_analytics_vtbl},
    0,
};

IXSystemAnalyticsImpl *x_system_analytics_impl = &x_system_analytics.IXSystemAnalyticsImpl_iface;
