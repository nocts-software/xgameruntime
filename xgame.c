/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XGame
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

struct x_game
{
    IXGameImpl3 IXGameImpl3_iface;
    LONG ref;
};

static inline struct x_game *impl_from_IXGameImpl3( IXGameImpl3 *iface )
{
    return CONTAINING_RECORD( iface, struct x_game, IXGameImpl3_iface );
}

static HRESULT WINAPI x_game_QueryInterface( IXGameImpl3 *iface, REFIID iid, void **out )
{
    struct x_game *impl = impl_from_IXGameImpl3( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (IsEqualGUID( iid, &IID_IUnknown    ) ||
        IsEqualGUID( iid, &IID_IXGameImpl  ) ||
        IsEqualGUID( iid, &IID_IXGameImpl2 ) ||
        IsEqualGUID( iid, &IID_IXGameImpl3 ))
    {
        IXGameImpl3_AddRef( *out = &impl->IXGameImpl3_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI x_game_AddRef( IXGameImpl3 *iface )
{
    struct x_game *impl = impl_from_IXGameImpl3( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_game_Release( IXGameImpl3 *iface )
{
    struct x_game *impl = impl_from_IXGameImpl3( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI x_game_XGameGetXboxTitleId( IXGameImpl3 *iface, UINT32 *titleId )
{
    const char *env_title_id;
    FILE *f;
    TRACE( "iface %p, titleId %p\n", iface, titleId );
    if (!titleId) return E_POINTER;

    env_title_id = getenv("XODUS_TITLE_ID");
    if (env_title_id && *env_title_id)
    {
        *titleId = (UINT32)strtoul(env_title_id, NULL, 16);
        fprintf(stderr, "[GDK XGame] XGameGetXboxTitleId: returning TitleId 0x%08X (%u) from env\n", *titleId, *titleId);
        return S_OK;
    }

    f = fopen("appxmanifest.xml", "r");
    if (!f) f = fopen("AppxManifest.xml", "r");
    if (!f) f = fopen("../appxmanifest.xml", "r");
    if (!f) f = fopen("../AppxManifest.xml", "r");
    if (!f) f = fopen("../../appxmanifest.xml", "r");
    if (!f) f = fopen("../../AppxManifest.xml", "r");
    if (!f) f = fopen("../../../appxmanifest.xml", "r");
    if (!f) f = fopen("../../../AppxManifest.xml", "r");
    if (f)
    {
        char line[512];
        while (fgets(line, sizeof(line), f))
        {
            char *p = strstr(line, "ms-xbl-");
            if (p)
            {
                p += 7;
                *titleId = (UINT32)strtoul(p, NULL, 16);
                fclose(f);
                fprintf(stderr, "[GDK XGame] XGameGetXboxTitleId: detected TitleId 0x%08X (%u) from manifest\n", *titleId, *titleId);
                return S_OK;
            }
        }
        fclose(f);
    }

    *titleId = 0x66591171;
    fprintf(stderr, "[GDK XGame] XGameGetXboxTitleId: fallback TitleId 0x%08X (%u)\n", *titleId, *titleId);
    return S_OK;
}


static void WINAPI x_game_XLaunchNewGame( IXGameImpl3 *iface, const char *exePath, const char *args, XUserHandle defaultUser )
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdline[2048];
    BOOL ret;

    fprintf(stderr, "[GDK XGame] XLaunchNewGame: launching target '%s' with args '%s' (defaultUser %p)\n",
            exePath ? exePath : "(null)", args ? args : "", defaultUser);

    if (!exePath || !*exePath)
        return;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

    if (args && *args)
        snprintf(cmdline, sizeof(cmdline), "\"%s\" %s", exePath, args);
    else
        snprintf(cmdline, sizeof(cmdline), "\"%s\"", exePath);

    ret = CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!ret)
    {
        ret = CreateProcessA(exePath, (LPSTR)args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    }

    if (ret)
    {
        fprintf(stderr, "[GDK XGame] XLaunchNewGame: successfully launched PID %lu\n", (unsigned long)pi.dwProcessId);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else
    {
        fprintf(stderr, "[GDK XGame] XLaunchNewGame: CreateProcess failed with error %lu for '%s'\n",
                (unsigned long)GetLastError(), cmdline);
    }
}

static HRESULT WINAPI x_game_XLaunchRestartOnCrash( IXGameImpl3 *iface, const char *args, UINT32 reserved )
{
    FIXME( "iface %p, args %s, reserved %u stub!\n", iface, debugstr_a( args ), reserved );
    return E_NOTIMPL;
}

static const struct IXGameImpl3Vtbl x_game_vtbl =
{
    x_game_QueryInterface,
    x_game_AddRef,
    x_game_Release,
    /* IXGameImpl methods */
    x_game_XGameGetXboxTitleId,
    /* IXGameImpl2 methods */
    x_game_XLaunchNewGame,
    /* IXGameImpl3 methods */
    x_game_XLaunchRestartOnCrash,
};

static struct x_game x_game =
{
    {&x_game_vtbl},
    0,
};

IXGameImpl *x_game_impl = (IXGameImpl *)&x_game.IXGameImpl3_iface;
