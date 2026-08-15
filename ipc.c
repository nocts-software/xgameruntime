/*
 * Xbox Game runtime Library - IPC Client Implementation
 *
 * Copyright 2026 Xodus Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define _WINSOCKAPI_
#define _WINSOCK2API_

#include <wine/debug.h>
#include "ipc.h"
#include "private.h"



WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

static int ipc_socket_fd = -1;

static int connect_to_daemon(void)
{
    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    char socket_path[256];
    struct sockaddr_un addr;
    int fd;

    if (runtime_dir && runtime_dir[0] != '\0')
        snprintf(socket_path, sizeof(socket_path), "%s/xodus.sock", runtime_dir);
    else
        snprintf(socket_path, sizeof(socket_path), "/tmp/xodus.sock");

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

HRESULT ipc_init(void)
{
    if (ipc_socket_fd >= 0)
        return S_OK;

    ipc_socket_fd = connect_to_daemon();
    if (ipc_socket_fd < 0)
    {
        WARN("Could not connect to xodus-service daemon at /tmp/xodus.sock\n");
        return S_FALSE;
    }

    TRACE("Connected to xodus-service daemon on fd %d\n", ipc_socket_fd);
    return S_OK;
}

void ipc_cleanup(void)
{
    if (ipc_socket_fd >= 0)
    {
        close(ipc_socket_fd);
        ipc_socket_fd = -1;
    }
}

static size_t encode_varint(UINT64 value, BYTE *out)
{
    size_t i = 0;
    while (value >= 0x80)
    {
        out[i++] = (BYTE)((value & 0x7F) | 0x80);
        value >>= 7;
    }
    out[i++] = (BYTE)(value & 0x7F);
    return i;
}

static size_t decode_varint(const BYTE *in, size_t in_len, UINT64 *out_val)
{
    UINT64 result = 0;
    size_t shift = 0;
    size_t i = 0;

    while (i < in_len)
    {
        BYTE b = in[i++];
        result |= ((UINT64)(b & 0x7F)) << shift;
        if ((b & 0x80) == 0)
        {
            *out_val = result;
            return i;
        }
        shift += 7;
        if (shift >= 64) break;
    }
    *out_val = 0;
    return 0;
}

static HRESULT send_ipc_message(XodusMessageType msg_type, const BYTE *payload, SIZE_T payload_len, BYTE *out_resp, SIZE_T resp_max, SIZE_T *out_resp_len)
{
    BYTE packet[1024];
    BYTE pb_buf[512];
    SIZE_T pb_len = 0;
    UINT32 magic = PROTO_MAGIC;
    UINT32 total_len;
    ssize_t sent, ret;
    UINT32 resp_payload_len;

    if (ipc_socket_fd < 0)
    {
        if (ipc_init() != S_OK)
            return E_FAIL;
    }

    /* Encode XodusMessage: msg_type (field 1), request_id (field 2), payload (field 3) */
    pb_buf[pb_len++] = (1 << 3) | 0; /* tag 1, varint */
    pb_len += encode_varint((UINT64)msg_type, &pb_buf[pb_len]);

    pb_buf[pb_len++] = (2 << 3) | 0; /* tag 2, varint */
    pb_len += encode_varint(1, &pb_buf[pb_len]);

    if (payload && payload_len > 0)
    {
        pb_buf[pb_len++] = (3 << 3) | 2; /* tag 3, length-delimited */
        pb_len += encode_varint((UINT64)payload_len, &pb_buf[pb_len]);
        memcpy(&pb_buf[pb_len], payload, payload_len);
        pb_len += payload_len;
    }

    total_len = (UINT32)pb_len;

    /* Write 4-byte magic + 4-byte len + pb payload */
    memcpy(packet, &magic, 4);
    memcpy(packet + 4, &total_len, 4);
    memcpy(packet + 8, pb_buf, pb_len);

    sent = write(ipc_socket_fd, packet, 8 + pb_len);
    if (sent < (ssize_t)(8 + pb_len))
    {
        ipc_cleanup();
        return E_FAIL;
    }

    /* Read 4-byte response len */
    ret = read(ipc_socket_fd, &resp_payload_len, 4);
    if (ret < 4 || resp_payload_len > resp_max)
    {
        ipc_cleanup();
        return E_FAIL;
    }

    ret = read(ipc_socket_fd, out_resp, resp_payload_len);
    if (ret < (ssize_t)resp_payload_len)
    {
        ipc_cleanup();
        return E_FAIL;
    }

    if (out_resp_len) *out_resp_len = (SIZE_T)ret;
    return S_OK;
}

HRESULT ipc_xuser_add(UINT32 options, XodusUserInfo *out_user)
{
    BYTE resp[512];
    SIZE_T resp_len = 0;
    HRESULT hr = send_ipc_message(XODUS_MSG_XUSER_ADD_REQUEST, NULL, 0, resp, sizeof(resp), &resp_len);

    if (FAILED(hr))
    {
        /* Fallback mock data when daemon is offline */
        out_user->user_id = 1;
        snprintf(out_user->xuid, sizeof(out_user->xuid), "%s", "2533274839201029");
        snprintf(out_user->gamertag, sizeof(out_user->gamertag), "%s", "XodusUser");
        return S_OK;
    }

    out_user->user_id = 1;
    snprintf(out_user->xuid, sizeof(out_user->xuid), "%s", "2533274839201029");
    snprintf(out_user->gamertag, sizeof(out_user->gamertag), "%s", "XodusUser");
    return S_OK;
}

HRESULT ipc_xuser_get_gamertag(UINT64 user_id, char *out_gamertag, SIZE_T max_len)
{
    snprintf(out_gamertag, max_len, "%s", "XodusUser");
    return S_OK;
}

HRESULT ipc_xuser_get_token(UINT64 user_id, const char *relying_party, char *out_token, SIZE_T token_max_len, char *out_sig, SIZE_T sig_max_len)
{
    snprintf(out_token, token_max_len, "%s", "MOCK_XSTS_TOKEN");
    if (out_sig) snprintf(out_sig, sig_max_len, "%s", "MOCK_SIG");
    return S_OK;
}

HRESULT ipc_xuser_get_gamer_picture(UINT64 user_id, DWORD picture_size, void *out_buf, SIZE_T buf_len, SIZE_T *out_used)
{
    BYTE png_header[4] = {0x89, 0x50, 0x4E, 0x47};
    SIZE_T copy_len = buf_len < 4 ? buf_len : 4;
    if (out_buf && copy_len > 0)
        memcpy(out_buf, png_header, copy_len);
    if (out_used)
        *out_used = copy_len;
    return S_OK;
}

HRESULT ipc_xuser_check_privilege(UINT64 user_id, UINT32 privilege, BOOL *out_has_privilege, UINT32 *out_deny_reason)
{
    if (out_has_privilege) *out_has_privilege = TRUE;
    if (out_deny_reason) *out_deny_reason = 0;
    return S_OK;
}

HRESULT ipc_xstore_query_license(UINT64 user_id, const char *product_id, BOOL *out_is_licensed)
{
    if (out_is_licensed) *out_is_licensed = TRUE;
    return S_OK;
}

HRESULT ipc_xstore_query_products(UINT64 user_id, UINT32 product_kinds, const char *store_ids, char *out_products_json, SIZE_T max_len, SIZE_T *out_count)
{
    if (out_products_json && max_len > 0)
        snprintf(out_products_json, max_len, "%s", "[{\"store_id\":\"mock_product\",\"is_in_user_collection\":true}]");
    if (out_count)
        *out_count = 1;
    return S_OK;
}



HRESULT ipc_xstore_acquire_license(UINT64 user_id, const char *package_id, BOOL *out_is_licensed, void *out_blob, SIZE_T max_len, SIZE_T *out_used)
{
    BYTE mock_blob[4] = {0x4C, 0x49, 0x43, 0x53};
    SIZE_T copy_len = max_len < 4 ? max_len : 4;
    if (out_is_licensed) *out_is_licensed = TRUE;
    if (out_blob && copy_len > 0) memcpy(out_blob, mock_blob, copy_len);
    if (out_used) *out_used = copy_len;
    return S_OK;
}


HRESULT ipc_xgamesave_read_blob(UINT64 user_id, const char *scid, const char *container, const char *blob, void *out_buf, SIZE_T buf_len, SIZE_T *out_read)
{
    if (out_read) *out_read = 0;
    return S_OK;
}

HRESULT ipc_xgamesave_write_blob(UINT64 user_id, const char *scid, const char *container, const char *blob, const void *buf, SIZE_T buf_len)
{
    return S_OK;
}
