/*
 * Xbox Game runtime Library
 *  GDK Component: System API -> XGameSave and XGameSaveFiles
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
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct x_game_save
{
    IXGameSaveImpl3 IXGameSaveImpl3_iface;
    LONG ref;
};

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

static inline struct x_game_save *impl_from_IXGameSaveImpl3( IXGameSaveImpl3 *iface )
{
    return CONTAINING_RECORD( iface, struct x_game_save, IXGameSaveImpl3_iface );
}

static HRESULT WINAPI x_game_save_QueryInterface( IXGameSaveImpl3 *iface, REFIID iid, void **out )
{
    struct x_game_save *impl = impl_from_IXGameSaveImpl3( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    IXGameSaveImpl_AddRef( *out = &impl->IXGameSaveImpl3_iface );
    return S_OK;
}

static ULONG WINAPI x_game_save_AddRef( IXGameSaveImpl3 *iface )
{
    struct x_game_save *impl = impl_from_IXGameSaveImpl3( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI x_game_save_Release( IXGameSaveImpl3 *iface )
{
    struct x_game_save *impl = impl_from_IXGameSaveImpl3( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI dummy_save_func(void *self) { return S_OK; }
static void *dummy_save_vtable[64] = {
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
    dummy_save_func, dummy_save_func, dummy_save_func, dummy_save_func,
};

struct save_handle_data {
    void **vtable;
    UINT64 id;
    void *extra[32];
};

static struct save_handle_data default_save_obj = {
    dummy_save_vtable,
    1
};

struct x_game_save_container_data {
    void **vtable;
    char container_name[256];
    char scid[64];
};

struct x_game_save_blob_write_item {
    char blob_name[256];
    UINT8 *data;
    SIZE_T size;
};

struct x_game_save_update_data {
    void **vtable;
    char container_name[256];
    char scid[64];
    struct x_game_save_blob_write_item writes[64];
    UINT32 write_count;
    char deletes[64][256];
    UINT32 delete_count;
};

static void get_scid(char *out_scid, size_t max_len)
{
    const char *title_id = getenv("XODUS_TITLE_ID");
    if (!title_id || !title_id[0]) title_id = "6564dc7b";
    snprintf(out_scid, max_len, "00000000-0000-0000-0000-0000%s", title_id);
    for (int i = 0; out_scid[i]; i++) {
        out_scid[i] = tolower(out_scid[i]);
    }
}

static void get_container_dir(const char *container_name, char *out_dir, size_t max_len)
{
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    char scid[64];
    get_scid(scid, sizeof(scid));
    snprintf(out_dir, max_len, "%s/.local/share/xodus/saves/%s/1/%s",
             home, scid, (container_name && container_name[0]) ? container_name : "default");
}

static HRESULT WINAPI x_game_save_XGameSaveInitializeProvider( IXGameSaveImpl3 *iface, XUserHandle requestingUser, const char *configurationId, BOOLEAN syncOnDemand, XGameSaveProviderHandle *provider )
{
    TRACE( "iface %p, requestingUser %p, configurationId %s, syncOnDemand %d, provider %p\n", iface, requestingUser, debugstr_a( configurationId ), syncOnDemand, provider );
    if (provider)
    {
        *provider = (XGameSaveProviderHandle)&default_save_obj;
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_game_save_XGameSaveInitializeProviderAsync( IXGameSaveImpl3 *iface, XUserHandle requestingUser, const char *configurationId, BOOLEAN syncOnDemand, XAsyncBlock *async )
{
    TRACE( "iface %p, requestingUser %p, configurationId %s, syncOnDemand %d, async %p.\n", iface, requestingUser, debugstr_a( configurationId ), syncOnDemand, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveInitializeProviderResult( IXGameSaveImpl3 *iface, XAsyncBlock *async, XGameSaveProviderHandle *provider )
{
    TRACE( "iface %p, async %p, provider %p.\n", iface, async, provider );
    if (provider)
    {
        *provider = (XGameSaveProviderHandle)&default_save_obj;
        return S_OK;
    }
    return E_POINTER;
}

static void WINAPI x_game_save_XGameSaveCloseProvider( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider )
{
    TRACE( "iface %p, provider %p.\n", iface, provider );
}

static HRESULT WINAPI x_game_save_XGameSaveGetRemainingQuota( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, INT64 *remainingQuota )
{
    TRACE( "iface %p, provider %p, remainingQuota %p.\n", iface, provider, remainingQuota );
    if (remainingQuota)
    {
        *remainingQuota = (INT64)1024 * 1024 * 1024; /* 1 GB remaining quota */
        return S_OK;
    }
    return E_POINTER;
}

static HRESULT WINAPI x_game_save_XGameSaveGetRemainingQuotaAsync( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, XAsyncBlock *async )
{
    TRACE( "iface %p, provider %p, async %p.\n", iface, provider, async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveGetRemainingQuotaResult( IXGameSaveImpl3 *iface, XAsyncBlock *async, INT64 *remainingQuota )
{
    TRACE( "iface %p, async %p, remainingQuota %p\n", iface, async, remainingQuota );
    if (remainingQuota) *remainingQuota = (INT64)1024 * 1024 * 1024;
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveDeleteContainer( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, const char *containerName )
{
    TRACE( "iface %p, provider %p, containerName %s.\n", iface, provider, debugstr_a( containerName ) );
    char dir[MAX_PATH];
    get_container_dir(containerName, dir, sizeof(dir));
    char rm_cmd[MAX_PATH + 32];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf \"%s\"", dir);
    system(rm_cmd);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveDeleteContainerAsync( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, const char *containerName, XAsyncBlock *async )
{
    TRACE( "iface %p, provider %p, containerName %s, async %p.\n", iface, provider, debugstr_a( containerName ), async );
    x_game_save_XGameSaveDeleteContainer(iface, provider, containerName);
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveDeleteContainerResult( IXGameSaveImpl3 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p\n", iface, async );
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveGetContainerInfo( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, const char *containerName, void *context, XGameSaveContainerInfoCallback *callback )
{
    TRACE( "iface %p, provider %p, containerName %s, context %p, callback %p\n", iface, provider, debugstr_a( containerName ), context, callback );
    if (!callback) return E_POINTER;
    char dir[MAX_PATH];
    get_container_dir(containerName ? containerName : "default", dir, sizeof(dir));
    DIR *d = opendir(dir);
    if (!d) return 0x80830002; /* E_GS_CONTAINER_NOT_FOUND */

    UINT32 blobCount = 0;
    UINT64 totalSize = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL)
    {
        if (ent->d_name[0] == '.') continue;
        char file_path[MAX_PATH];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir, ent->d_name);
        struct stat st;
        if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode))
        {
            blobCount++;
            totalSize += st.st_size;
        }
    }
    closedir(d);

    XGameSaveContainerInfo info;
    info.name = containerName ? containerName : "default";
    info.displayName = containerName ? containerName : "default";
    info.blobCount = blobCount;
    info.totalSize = totalSize;
    info.lastModifiedTime = 0;
    info.needsSync = FALSE;
    callback(&info, context);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveEnumerateContainerInfo( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, void *context, XGameSaveContainerInfoCallback *callback )
{
    TRACE( "iface %p, provider %p, context %p, callback %p\n", iface, provider, context, callback );
    if (!callback) return E_POINTER;
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    char scid[64];
    get_scid(scid, sizeof(scid));
    char base_dir[MAX_PATH];
    snprintf(base_dir, sizeof(base_dir), "%s/.local/share/xodus/saves/%s/1", home, scid);
    DIR *d = opendir(base_dir);
    if (!d) return S_OK;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL)
    {
        if (ent->d_name[0] == '.') continue;
        char cdir[MAX_PATH];
        snprintf(cdir, sizeof(cdir), "%s/%s", base_dir, ent->d_name);
        struct stat st;
        if (stat(cdir, &st) == 0 && S_ISDIR(st.st_mode))
        {
            XGameSaveContainerInfo info;
            info.name = ent->d_name;
            info.displayName = ent->d_name;
            info.blobCount = 1;
            info.totalSize = 1024;
            info.lastModifiedTime = 0;
            info.needsSync = FALSE;
            if (!callback(&info, context)) break;
        }
    }
    closedir(d);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveEnumerateContainerInfoByName( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, const char *containerNamePrefix, void *context, XGameSaveContainerInfoCallback *callback )
{
    TRACE( "iface %p, provider %p, containerNamePrefix %s, context %p, callback %p\n", iface, provider, debugstr_a( containerNamePrefix ), context, callback );
    return x_game_save_XGameSaveEnumerateContainerInfo(iface, provider, context, callback);
}

static HRESULT WINAPI x_game_save_XGameSaveCreateContainer( IXGameSaveImpl3 *iface, XGameSaveProviderHandle provider, const char *containerName, XGameSaveContainerHandle *containerContext )
{
    TRACE( "iface %p, provider %p, containerName %s, containerContext %p.\n", iface, provider, debugstr_a( containerName ), containerContext );
    if (!containerContext) return E_POINTER;

    struct x_game_save_container_data *c = malloc(sizeof(struct x_game_save_container_data));
    if (!c) return E_OUTOFMEMORY;
    memset(c, 0, sizeof(*c));
    c->vtable = dummy_save_vtable;
    snprintf(c->container_name, sizeof(c->container_name), "%s", (containerName && containerName[0]) ? containerName : "default");
    get_scid(c->scid, sizeof(c->scid));

    char dir[MAX_PATH];
    get_container_dir(c->container_name, dir, sizeof(dir));
    char mkdir_cmd[MAX_PATH + 32];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", dir);
    system(mkdir_cmd);

    *containerContext = (XGameSaveContainerHandle)c;
    return S_OK;
}

static void WINAPI x_game_save_XGameSaveCloseContainer( IXGameSaveImpl3 *iface, XGameSaveContainerHandle context )
{
    TRACE( "iface %p, context %p.\n", iface, context );
    if (context && context != (XGameSaveContainerHandle)&default_save_obj)
    {
        free(context);
    }
}

static HRESULT WINAPI x_game_save_XGameSaveEnumerateBlobInfoByName( IXGameSaveImpl3 *iface, XGameSaveContainerHandle container, const char *blobNamePrefix, void *context, XGameSaveBlobInfoCallback *callback )
{
    TRACE( "iface %p, container %p, blobNamePrefix %s, context %p, callback %p.\n", iface, container, debugstr_a( blobNamePrefix ), context, callback );
    if (!callback) return E_POINTER;

    const char *cname = "default";
    if (container && container != (XGameSaveContainerHandle)&default_save_obj)
    {
        struct x_game_save_container_data *c = (struct x_game_save_container_data *)container;
        cname = c->container_name;
    }

    char dir[MAX_PATH];
    get_container_dir(cname, dir, sizeof(dir));

    DIR *d = opendir(dir);
    if (!d) return S_OK;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL)
    {
        if (ent->d_name[0] == '.') continue;
        if (blobNamePrefix && blobNamePrefix[0] && strncmp(ent->d_name, blobNamePrefix, strlen(blobNamePrefix)) != 0)
        {
            continue;
        }

        char full_file[MAX_PATH];
        snprintf(full_file, sizeof(full_file), "%s/%s", dir, ent->d_name);
        struct stat st;
        if (stat(full_file, &st) == 0 && S_ISREG(st.st_mode))
        {
            XGameSaveBlobInfo info;
            info.name = ent->d_name;
            info.size = (UINT32)st.st_size;
            if (!callback(&info, context))
            {
                break;
            }
        }
    }
    closedir(d);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveEnumerateBlobInfo( IXGameSaveImpl3 *iface, XGameSaveContainerHandle container, void *context, XGameSaveBlobInfoCallback *callback )
{
    TRACE( "iface %p, container %p, context %p, callback %p.\n", iface, container, context, callback );
    return x_game_save_XGameSaveEnumerateBlobInfoByName(iface, container, NULL, context, callback);
}

static HRESULT WINAPI x_game_save_XGameSaveReadBlobData( IXGameSaveImpl3 *iface, XGameSaveContainerHandle container, const char **blobNames, UINT32 *countOfBlobs, SIZE_T blobsSize, XGameSaveBlob *blobData )
{
    TRACE( "iface %p, container %p, blobNames %p, countOfBlobs %p, blobsSize %Iu, blobData %p.\n", iface, container, blobNames, countOfBlobs, blobsSize, blobData );

    UINT32 requested_count = 0;
    if ((UINT_PTR)countOfBlobs < 0x10000)
    {
        requested_count = (UINT32)(UINT_PTR)countOfBlobs;
    }
    else if (countOfBlobs)
    {
        requested_count = *countOfBlobs;
    }

    if (requested_count == 0 || !blobNames)
    {
        if ((UINT_PTR)countOfBlobs >= 0x10000 && countOfBlobs) *countOfBlobs = 0;
        return S_OK;
    }

    const char *cname = "default";
    if (container && container != (XGameSaveContainerHandle)&default_save_obj)
    {
        struct x_game_save_container_data *c = (struct x_game_save_container_data *)container;
        cname = c->container_name;
    }

    char dir[MAX_PATH];
    get_container_dir(cname, dir, sizeof(dir));

    SIZE_T total_data_size = 0;
    for (UINT32 i = 0; i < requested_count; i++)
    {
        if (blobNames[i])
        {
            char file_path[MAX_PATH];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, blobNames[i]);
            struct stat st;
            if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode))
            {
                total_data_size += st.st_size;
            }
        }
    }

    SIZE_T headers_size = requested_count * sizeof(XGameSaveBlob);
    if (!blobData || blobsSize == 0)
    {
        if ((UINT_PTR)countOfBlobs >= 0x10000 && countOfBlobs) *countOfBlobs = requested_count;
        return S_OK;
    }

    if (blobsSize < headers_size)
    {
        if ((UINT_PTR)countOfBlobs >= 0x10000 && countOfBlobs) *countOfBlobs = requested_count;
        return 0x8007007A; /* ERROR_INSUFFICIENT_BUFFER */
    }

    memset(blobData, 0, blobsSize);
    UINT8 *data_cursor = (UINT8 *)blobData + headers_size;
    SIZE_T remaining_bytes = blobsSize > headers_size ? (blobsSize - headers_size) : 0;

    for (UINT32 i = 0; i < requested_count; i++)
    {
        const char *bname = blobNames[i] ? blobNames[i] : "default";
        blobData[i].info.name = bname;
        blobData[i].info.size = 0;
        blobData[i].data = data_cursor;

        char file_path[MAX_PATH];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir, bname);
        FILE *f = fopen(file_path, "rb");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            if (fsize > 0 && (SIZE_T)fsize <= remaining_bytes)
            {
                size_t read_bytes = fread(data_cursor, 1, fsize, f);
                blobData[i].info.size = (UINT32)read_bytes;
                data_cursor += read_bytes;
                remaining_bytes -= read_bytes;
            }
            fclose(f);
        }
    }

    if ((UINT_PTR)countOfBlobs >= 0x10000 && countOfBlobs) *countOfBlobs = requested_count;
    return S_OK;
}

struct x_game_save_read_async_ctx {
    char container_name[MAX_PATH];
    UINT32 count;
    char blob_names[32][MAX_PATH];
};

static struct x_game_save_read_async_ctx last_read_blob_ctx;

static HRESULT WINAPI x_game_save_XGameSaveReadBlobDataAsync( IXGameSaveImpl3 *iface, XGameSaveContainerHandle container, const char **blobNames, UINT32 countOfBlobs, XAsyncBlock *async )
{
    TRACE( "iface %p, container %p, blobNames %p, countOfBlobs %u, async %p.\n", iface, container, blobNames, countOfBlobs, async );
    const char *cname = "default";
    if (container && container != (XGameSaveContainerHandle)&default_save_obj)
    {
        struct x_game_save_container_data *c = (struct x_game_save_container_data *)container;
        cname = c->container_name;
    }
    char dir[MAX_PATH];
    get_container_dir(cname, dir, sizeof(dir));

    memset(&last_read_blob_ctx, 0, sizeof(last_read_blob_ctx));
    snprintf(last_read_blob_ctx.container_name, sizeof(last_read_blob_ctx.container_name), "%s", cname);
    last_read_blob_ctx.count = countOfBlobs > 32 ? 32 : countOfBlobs;
    
    SIZE_T total_data_size = 0;
    for (UINT32 i = 0; i < last_read_blob_ctx.count; i++)
    {
        if (blobNames && blobNames[i])
        {
            snprintf(last_read_blob_ctx.blob_names[i], sizeof(last_read_blob_ctx.blob_names[i]), "%s", blobNames[i]);
            char file_path[MAX_PATH];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, blobNames[i]);
            struct stat st;
            if (stat(file_path, &st) == 0 && S_ISREG(st.st_mode))
            {
                total_data_size += st.st_size;
            }
        }
    }

    SIZE_T required_size = last_read_blob_ctx.count * sizeof(XGameSaveBlob) + total_data_size;
    if (required_size == 0) required_size = sizeof(XGameSaveBlob);
    complete_async_with_size(async, required_size);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveReadBlobDataResult( IXGameSaveImpl3 *iface, XAsyncBlock *async, SIZE_T blobsSize, XGameSaveBlob *blobData, UINT32 *countOfBlobs )
{
    TRACE( "iface %p, async %p, blobsSize %Iu, blobData %p, countOfBlobs %p.\n", iface, async, blobsSize, blobData, countOfBlobs );
    if (!countOfBlobs) return E_POINTER;

    const char *bnames[32];
    for (UINT32 i = 0; i < last_read_blob_ctx.count; i++)
    {
        bnames[i] = last_read_blob_ctx.blob_names[i];
    }
    UINT32 cnt = last_read_blob_ctx.count;

    struct x_game_save_container_data cdata;
    memset(&cdata, 0, sizeof(cdata));
    snprintf(cdata.container_name, sizeof(cdata.container_name), "%s", last_read_blob_ctx.container_name);

    HRESULT hr = x_game_save_XGameSaveReadBlobData(iface, (XGameSaveContainerHandle)&cdata, bnames, &cnt, blobsSize, blobData);
    *countOfBlobs = cnt;
    return hr;
}

static HRESULT WINAPI x_game_save_XGameSaveCreateUpdate( IXGameSaveImpl3 *iface, XGameSaveContainerHandle container, const char *containerDisplayName, XGameSaveUpdateHandle *updateContext )
{
    TRACE( "iface %p, container %p, containerDisplayName %s, updateContext %p.\n", iface, container, debugstr_a( containerDisplayName ), updateContext );
    if (!updateContext) return E_POINTER;

    struct x_game_save_update_data *u = malloc(sizeof(struct x_game_save_update_data));
    if (!u) return E_OUTOFMEMORY;
    memset(u, 0, sizeof(*u));
    u->vtable = dummy_save_vtable;

    const char *cname = "default";
    if (container && container != (XGameSaveContainerHandle)&default_save_obj)
    {
        struct x_game_save_container_data *c = (struct x_game_save_container_data *)container;
        cname = c->container_name;
    }
    snprintf(u->container_name, sizeof(u->container_name), "%s", cname);
    get_scid(u->scid, sizeof(u->scid));

    *updateContext = (XGameSaveUpdateHandle)u;
    return S_OK;
}

static void WINAPI x_game_save_XGameSaveCloseUpdate( IXGameSaveImpl3 *iface, XGameSaveUpdateHandle context )
{
    TRACE( "iface %p, context %p.\n", iface, context );
    if (context && context != (XGameSaveUpdateHandle)&default_save_obj)
    {
        struct x_game_save_update_data *u = (struct x_game_save_update_data *)context;
        for (UINT32 i = 0; i < u->write_count; i++)
        {
            if (u->writes[i].data) free(u->writes[i].data);
        }
        free(u);
    }
}

static HRESULT WINAPI x_game_save_XGameSaveSubmitBlobWrite( IXGameSaveImpl3 *iface, XGameSaveUpdateHandle updateContext, const char *blobName, UINT8 *data, SIZE_T byteCount )
{
    TRACE( "iface %p, updateContext %p, blobName %s, data %p, byteCount %Iu.\n", iface, updateContext, debugstr_a( blobName ), data, byteCount );
    if (!blobName) return E_INVALIDARG;

    if (updateContext && updateContext != (XGameSaveUpdateHandle)&default_save_obj)
    {
        struct x_game_save_update_data *u = (struct x_game_save_update_data *)updateContext;
        if (u->write_count < 64)
        {
            snprintf(u->writes[u->write_count].blob_name, sizeof(u->writes[u->write_count].blob_name), "%s", blobName);
            u->writes[u->write_count].size = byteCount;
            if (byteCount > 0 && data)
            {
                u->writes[u->write_count].data = malloc(byteCount);
                if (u->writes[u->write_count].data)
                {
                    memcpy(u->writes[u->write_count].data, data, byteCount);
                }
            }
            else
            {
                u->writes[u->write_count].data = NULL;
            }
            u->write_count++;
        }
    }
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveSubmitBlobDelete( IXGameSaveImpl3 *iface, XGameSaveUpdateHandle updateContext, const char *blobName )
{
    TRACE( "iface %p, updateContext %p, blobName %s.\n", iface, updateContext, debugstr_a( blobName ) );
    if (!blobName) return E_INVALIDARG;

    if (updateContext && updateContext != (XGameSaveUpdateHandle)&default_save_obj)
    {
        struct x_game_save_update_data *u = (struct x_game_save_update_data *)updateContext;
        if (u->delete_count < 64)
        {
            snprintf(u->deletes[u->delete_count], sizeof(u->deletes[u->delete_count]), "%s", blobName);
            u->delete_count++;
        }
    }
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveSubmitUpdate( IXGameSaveImpl3 *iface, XGameSaveUpdateHandle updateContext )
{
    TRACE( "iface %p, updateContext %p.\n", iface, updateContext );
    if (updateContext && updateContext != (XGameSaveUpdateHandle)&default_save_obj)
    {
        struct x_game_save_update_data *u = (struct x_game_save_update_data *)updateContext;
        char dir[MAX_PATH];
        get_container_dir(u->container_name, dir, sizeof(dir));
        char mkdir_cmd[MAX_PATH + 32];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", dir);
        system(mkdir_cmd);

        for (UINT32 i = 0; i < u->write_count; i++)
        {
            char file_path[MAX_PATH];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, u->writes[i].blob_name);
            FILE *f = fopen(file_path, "wb");
            if (f)
            {
                if (u->writes[i].size > 0 && u->writes[i].data)
                {
                    fwrite(u->writes[i].data, 1, u->writes[i].size, f);
                }
                fclose(f);
                TRACE( "Wrote save blob %s (%Iu bytes)\n", file_path, u->writes[i].size );
            }
        }

        for (UINT32 i = 0; i < u->delete_count; i++)
        {
            char file_path[MAX_PATH];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, u->deletes[i]);
            unlink(file_path);
            TRACE( "Deleted save blob %s\n", file_path );
        }
    }
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveSubmitUpdateAsync( IXGameSaveImpl3 *iface, XGameSaveUpdateHandle updateContext, XAsyncBlock *async )
{
    TRACE( "iface %p, updateContext %p, async %p.\n", iface, updateContext, async );
    x_game_save_XGameSaveSubmitUpdate(iface, updateContext);
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveSubmitUpdateResult( IXGameSaveImpl3 *iface, XAsyncBlock *async )
{
    TRACE( "iface %p, async %p.\n", iface, async );
    return S_OK;
}

static char cached_save_path[MAX_PATH] = {0};

static void get_save_folder(char *out_path, size_t max_len)
{
    if (cached_save_path[0] != '\0')
    {
        snprintf(out_path, max_len, "%s", cached_save_path);
        return;
    }

    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    char scid[64];
    get_scid(scid, sizeof(scid));

    snprintf(cached_save_path, sizeof(cached_save_path), "Z:%s/.local/share/xodus/saves/%s/1", home, scid);
    
    for (int i = 0; cached_save_path[i]; i++) {
        if (cached_save_path[i] == '/') {
            cached_save_path[i] = '\\';
        }
    }
    
    snprintf(out_path, max_len, "%s", cached_save_path);
}

static HRESULT WINAPI x_game_save_XGameSaveFilesGetFolderWithUiAsync( IXGameSaveImpl3 *iface, XUserHandle requestingUser, const char *configurationId, XAsyncBlock *async )
{
    TRACE( "iface %p, requestingUser %p, configurationId %s, async %p.\n", iface, requestingUser, debugstr_a( configurationId ), async );
    complete_async(async);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveFilesGetFolderWithUiResult( IXGameSaveImpl3 *iface, XAsyncBlock *async, SIZE_T folderSize, char *folderResult )
{
    TRACE( "iface %p, async %p, folderSize %Iu, folderResult %p.\n", iface, async, folderSize, folderResult );
    if (!folderResult || folderSize == 0) return E_INVALIDARG;
    get_save_folder(folderResult, folderSize);
    return S_OK;
}

static HRESULT WINAPI x_game_save_XGameSaveFilesGetRemainingQuota( IXGameSaveImpl3 *iface, XUserHandle userContext, const char *configurationId, INT64 *remainingQuota )
{
    TRACE( "iface %p, userContext %p, configurationId %s, remainingQuota %p.\n", iface, userContext, debugstr_a( configurationId ), remainingQuota );
    if (remainingQuota) *remainingQuota = (INT64)1024 * 1024 * 1024;
    return S_OK;
}

static const struct IXGameSaveImpl3Vtbl x_game_save_vtbl =
{
    x_game_save_QueryInterface,
    x_game_save_AddRef,
    x_game_save_Release,
    /* IXGameSaveImpl methods */
    x_game_save_XGameSaveInitializeProvider,
    x_game_save_XGameSaveInitializeProviderAsync,
    x_game_save_XGameSaveInitializeProviderResult,
    x_game_save_XGameSaveCloseProvider,
    x_game_save_XGameSaveGetRemainingQuota,
    x_game_save_XGameSaveGetRemainingQuotaAsync,
    x_game_save_XGameSaveGetRemainingQuotaResult,
    x_game_save_XGameSaveDeleteContainer,
    x_game_save_XGameSaveDeleteContainerAsync,
    x_game_save_XGameSaveDeleteContainerResult,
    x_game_save_XGameSaveGetContainerInfo,
    x_game_save_XGameSaveEnumerateContainerInfo,
    x_game_save_XGameSaveEnumerateContainerInfoByName,
    x_game_save_XGameSaveCreateContainer,
    x_game_save_XGameSaveCloseContainer,
    x_game_save_XGameSaveEnumerateBlobInfo,
    x_game_save_XGameSaveEnumerateBlobInfoByName,
    x_game_save_XGameSaveReadBlobData,
    x_game_save_XGameSaveReadBlobDataAsync,
    x_game_save_XGameSaveReadBlobDataResult,
    x_game_save_XGameSaveCreateUpdate,
    x_game_save_XGameSaveCloseUpdate,
    x_game_save_XGameSaveSubmitBlobWrite,
    x_game_save_XGameSaveSubmitBlobDelete,
    x_game_save_XGameSaveSubmitUpdate,
    x_game_save_XGameSaveSubmitUpdateAsync,
    x_game_save_XGameSaveSubmitUpdateResult,
    /* IXGameSaveImpl2 methods */
    x_game_save_XGameSaveFilesGetFolderWithUiAsync,
    x_game_save_XGameSaveFilesGetFolderWithUiResult,
    x_game_save_XGameSaveFilesGetRemainingQuota,
};

static struct x_game_save x_game_save =
{
    {&x_game_save_vtbl},
    0,
};

IXGameSaveImpl *x_game_save_impl = (IXGameSaveImpl *)&x_game_save.IXGameSaveImpl3_iface;
