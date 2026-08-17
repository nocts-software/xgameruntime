/*
 * Xbox Game runtime Library
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

#define COBJMACROS

#include <roapi.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <wine/debug.h>
#include <winstring.h>

static inline void xgdk_log(const char *level, const char *func, int line, const char *fmt, ...)
{
    char time_str[64];
    struct timeval tv;
    struct tm *tm_info;
    va_list args;
    FILE *log_f;
    time_t sec;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;
    tm_info = localtime(&sec);
    if (tm_info)
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    else
        snprintf(time_str, sizeof(time_str), "??:??:??");

    log_f = fopen("/tmp/xodus-gdk.log", "a");
    if (log_f) {
        fprintf(log_f, "[XGDK %s][%s.%03ld][TID %04lx][%s:%d] ",
                level, time_str, (long)(tv.tv_usec / 1000), (unsigned long)GetCurrentThreadId(), func, line);
        va_start(args, fmt);
        vfprintf(log_f, fmt, args);
        va_end(args);
        fclose(log_f);
    }

    fprintf(stderr, "[XGDK %s][%s.%03ld][TID %04lx][%s:%d] ",
            level, time_str, (long)(tv.tv_usec / 1000), (unsigned long)GetCurrentThreadId(), func, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

#undef TRACE
#define TRACE(fmt, ...) xgdk_log("TRACE", __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#undef FIXME
#define FIXME(fmt, ...) xgdk_log("FIXME", __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#undef WARN
#define WARN(fmt, ...) xgdk_log("WARN", __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#undef ERR
#define ERR(fmt, ...) xgdk_log("ERR", __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#include <xaccessibility.h>
#include <xappcapture.h>
#include <xasync.h>
#include <xasyncprovider.h>
#include <xdisplay.h>
#include <xerror.h>
#include <xgame.h>
#include <xgameactivation.h>
#include <xgameerr.h>
#include <xgameevent.h>
#include <xgameinvite.h>
#include <xgameprotocol.h>
#include <xgameruntimefeature.h>
#include <xgameruntimetypes.h>
#include <xgamesave.h>
#include <xgamestreaming.h>
#include <xgameui.h>
#include <xnetworking.h>
#include <xpackage.h>
#include <xpersistentlocalstorage.h>
#include <xspeechsynthesizer.h>
#include <xstore.h>
#include <xsystem.h>
#include <xtaskqueue.h>
#include <xuser.h>
#include "ipc.h"


/* April 2026 Update 3 Release of GDK */
#define GDKC_VERSION 10002L
#define GAMING_SERVICES_VERSION 7874L

#define WIDL_using_Windows_Foundation
#define WIDL_using_Windows_Foundation_Collections
#include <windows.foundation.h>
#define WIDL_using_Windows_Globalization
#include <windows.globalization.h>
#define WIDL_using_Windows_System_Profile
#include <windows.system.profile.h>

extern DWORD tlsIndex;

extern IXAccessibilityImpl *x_accessibility_impl;
extern IXAppCaptureImpl *x_app_capture_impl;
extern IXAppCaptureMetadataImpl *x_app_capture_metadata_impl;
extern IXDisplayImpl *x_display_impl;
extern IXErrorImpl *x_error_impl;
extern IXGameImpl *x_game_impl;
extern IXGameActivationImpl *x_game_activation_impl;
extern IXGameEventImpl *x_game_event_impl;
extern IXGameInviteImpl *x_game_invite_impl;
extern IXGameProtocolImpl *x_game_protocol_impl;
extern IXGameRuntimeFeatureImpl *x_game_runtime_feature_impl;
extern IXGameSaveImpl *x_game_save_impl;
extern IXGameStreamingImpl *x_game_streaming_impl;
extern IXGameUiImpl *x_game_ui_impl;
extern IXLauncherImpl *x_launcher_impl;
extern IXNetworkingImpl *x_networking_impl;
extern IXPackageImpl *x_package_impl;
extern IXPersistentLocalStorageImpl *x_persistent_local_storage_impl;
extern IXStoreImpl *x_store_impl;
extern IXSystemImpl *x_system_impl;
extern IXSystemAnalyticsImpl *x_system_analytics_impl;
extern IXThreadingImpl *x_threading_impl;
extern IXUserImpl *x_user_impl;
extern IXUserDeviceImpl *x_user_device_impl;

extern void *get_winrt_package_factory(void);
extern HWND create_core_window_host(void);
void complete_async(XAsyncBlock *async);
void complete_async_with_size(XAsyncBlock *async, SIZE_T required_size);

struct initialize_options
{
    UINT32 unk;
    BOOL isInline;
    const char *gameConfig;
};

HRESULT WINAPI InitializeApiImpl( ULONG gdkVer, ULONG gsVer );
HRESULT WINAPI InitializeApiImplEx2( ULONG gdkVer, ULONG gsVer, char mode, const struct initialize_options *options );
HRESULT WINAPI UninitializeApiImpl(void);
HRESULT WINAPI QueryApiImpl( const GUID *classId, REFIID interfaceId, void **out );
