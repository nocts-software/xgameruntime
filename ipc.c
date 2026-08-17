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

/**
 * Connects to the active `xodus-service` background daemon via Unix domain socket.
 * 
 * Iterates through potential socket paths ($XDG_RUNTIME_DIR/xodus.sock, /tmp/xodus.sock, /run/user/$UID/xodus.sock)
 * and establishes a stream connection to service title authentication, licensing, and save sync requests.
 * 
 * @return Active file descriptor on success, or -1 if the daemon is unreachable.
 */
static int connect_to_daemon(void)
{
    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    const char *candidates[8];
    int cand_count = 0;
    char custom_path[256];
    int fd, i;

    if (runtime_dir && runtime_dir[0] != '\0')
    {
        snprintf(custom_path, sizeof(custom_path), "%s/xodus.sock", runtime_dir);
        candidates[cand_count++] = custom_path;
    }
    candidates[cand_count++] = "/tmp/xodus.sock";
    candidates[cand_count++] = "/run/user/1000/xodus.sock";
    candidates[cand_count++] = "/run/user/1001/xodus.sock";

    for (i = 0; i < cand_count; i++)
    {
        struct sockaddr_un addr;
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) continue;

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", candidates[i]);

        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
        {
            fprintf(stderr, "[GDK IPC] Connected to xodus-service daemon at %s (fd %d)\n", candidates[i], fd);
            return fd;
        }
        close(fd);
    }

    fprintf(stderr, "[GDK IPC] ERROR: Failed to connect to xodus-service daemon on all socket candidates\n");
    return -1;
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

static const BYTE *pb_find_field(const BYTE *buf, SIZE_T buf_len, UINT32 target_field, UINT32 expected_wire_type, SIZE_T *out_field_len)
{
    SIZE_T i = 0;
    while (i < buf_len)
    {
        UINT64 key = 0;
        size_t key_len = decode_varint(&buf[i], buf_len - i, &key);
        if (!key_len) break;
        i += key_len;

        UINT32 field_num = (UINT32)(key >> 3);
        UINT32 wire_type = (UINT32)(key & 7);

        if (wire_type == 0) /* Varint */
        {
            UINT64 val = 0;
            size_t val_len = decode_varint(&buf[i], buf_len - i, &val);
            if (!val_len) break;
            if (field_num == target_field && wire_type == expected_wire_type)
            {
                if (out_field_len) *out_field_len = (SIZE_T)val;
                return &buf[i];
            }
            i += val_len;
        }
        else if (wire_type == 2) /* Length-delimited */
        {
            UINT64 len = 0;
            size_t len_bytes = decode_varint(&buf[i], buf_len - i, &len);
            if (!len_bytes) break;
            i += len_bytes;
            if (i + len > buf_len) break;

            if (field_num == target_field && wire_type == expected_wire_type)
            {
                if (out_field_len) *out_field_len = (SIZE_T)len;
                return &buf[i];
            }
            i += (SIZE_T)len;
        }
        else if (wire_type == 1) /* 64-bit */
        {
            i += 8;
        }
        else if (wire_type == 5) /* 32-bit */
        {
            i += 4;
        }
        else
        {
            break;
        }
    }
    return NULL;
}

static const BYTE *pb_get_xodus_payload(const BYTE *msg_buf, SIZE_T msg_len, SIZE_T *out_payload_len)
{
    return pb_find_field(msg_buf, msg_len, 3, 2, out_payload_len);
}

static HRESULT send_ipc_message(XodusMessageType msg_type, const BYTE *payload, SIZE_T payload_len, BYTE *out_resp, SIZE_T resp_max, SIZE_T *out_resp_len)
{
    BYTE packet[16384 + 32];
    BYTE pb_buf[16384];
    SIZE_T pb_len = 0;
    UINT32 magic = PROTO_MAGIC;
    UINT32 total_len;
    ssize_t sent, ret;
    UINT32 resp_payload_len;

    if (ipc_socket_fd < 0)
    {
        if (ipc_init() != S_OK)
        {
            fprintf(stderr, "[GDK IPC] ERROR: Cannot connect to xodus-service daemon!\n");
            return E_FAIL;
        }
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
        if (pb_len + payload_len > sizeof(pb_buf))
            return E_OUTOFMEMORY;
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
        fprintf(stderr, "[GDK IPC] ERROR: Failed to write to daemon socket\n");
        ipc_cleanup();
        return E_FAIL;
    }

    /* Read 4-byte response len */
    {
        SIZE_T len_read = 0;
        while (len_read < 4)
        {
            ret = read(ipc_socket_fd, ((BYTE *)&resp_payload_len) + len_read, 4 - len_read);
            if (ret <= 0)
            {
                fprintf(stderr, "[GDK IPC] ERROR: Connection lost reading response length\n");
                ipc_cleanup();
                return E_FAIL;
            }
            len_read += (SIZE_T)ret;
        }
    }

    if (resp_payload_len > resp_max)
    {
        fprintf(stderr, "[GDK IPC] ERROR: Response len %u exceeds maximum buffer %zu\n", resp_payload_len, resp_max);
        ipc_cleanup();
        return E_FAIL;
    }

    {
        SIZE_T total_payload_read = 0;
        while (total_payload_read < resp_payload_len)
        {
            ret = read(ipc_socket_fd, out_resp + total_payload_read, resp_payload_len - total_payload_read);
            if (ret <= 0)
            {
                fprintf(stderr, "[GDK IPC] ERROR: Connection lost reading response payload\n");
                ipc_cleanup();
                return E_FAIL;
            }
            total_payload_read += (SIZE_T)ret;
        }
        if (out_resp_len) *out_resp_len = total_payload_read;
    }

    return S_OK;
}

HRESULT ipc_xuser_add(UINT32 options, XodusUserInfo *out_user)
{
    BYTE req[32];
    SIZE_T req_len = 0;
    BYTE resp[2048];
    SIZE_T resp_len = 0;
    HRESULT hr;

    if (!out_user) return E_POINTER;

    /* Fallback default values */
    out_user->user_id = 1;
    snprintf(out_user->xuid, sizeof(out_user->xuid), "%s", "2533274839201029");
    snprintf(out_user->gamertag, sizeof(out_user->gamertag), "%s", "XodusUser");

    req[req_len++] = (1 << 3) | 0;
    req_len += encode_varint((UINT64)options, &req[req_len]);

    hr = send_ipc_message(XODUS_MSG_XUSER_ADD_REQUEST, req, req_len, resp, sizeof(resp), &resp_len);
    if (SUCCEEDED(hr))
    {
        SIZE_T payload_len = 0;
        const BYTE *payload = pb_get_xodus_payload(resp, resp_len, &payload_len);
        if (payload)
        {
            SIZE_T str_len = 0;
            const BYTE *xuid_ptr = pb_find_field(payload, payload_len, 3, 2, &str_len);
            if (xuid_ptr && str_len > 0)
            {
                SIZE_T copy_len = str_len < sizeof(out_user->xuid) - 1 ? str_len : sizeof(out_user->xuid) - 1;
                memcpy(out_user->xuid, xuid_ptr, copy_len);
                out_user->xuid[copy_len] = '\0';
            }
            const BYTE *gt_ptr = pb_find_field(payload, payload_len, 4, 2, &str_len);
            if (gt_ptr && str_len > 0)
            {
                SIZE_T copy_len = str_len < sizeof(out_user->gamertag) - 1 ? str_len : sizeof(out_user->gamertag) - 1;
                memcpy(out_user->gamertag, gt_ptr, copy_len);
                out_user->gamertag[copy_len] = '\0';
            }
        }
    }
    return S_OK;
}

HRESULT ipc_xuser_get_gamertag(UINT64 user_id, char *out_gamertag, SIZE_T max_len)
{
    BYTE req[32];
    SIZE_T req_len = 0;
    BYTE resp[1024];
    SIZE_T resp_len = 0;
    HRESULT hr;

    if (!out_gamertag || max_len == 0) return E_POINTER;
    snprintf(out_gamertag, max_len, "%s", "XodusUser");

    req[req_len++] = (1 << 3) | 0;
    req_len += encode_varint(user_id, &req[req_len]);

    hr = send_ipc_message(XODUS_MSG_XUSER_GET_GAMERTAG_REQUEST, req, req_len, resp, sizeof(resp), &resp_len);
    if (SUCCEEDED(hr))
    {
        SIZE_T payload_len = 0;
        const BYTE *payload = pb_get_xodus_payload(resp, resp_len, &payload_len);
        if (payload)
        {
            SIZE_T str_len = 0;
            const BYTE *gt_ptr = pb_find_field(payload, payload_len, 2, 2, &str_len);
            if (gt_ptr && str_len > 0)
            {
                SIZE_T copy_len = str_len < max_len - 1 ? str_len : max_len - 1;
                memcpy(out_gamertag, gt_ptr, copy_len);
                out_gamertag[copy_len] = '\0';
            }
        }
    }
    return S_OK;
}

HRESULT ipc_xuser_get_token(UINT64 user_id, const char *relying_party, const char *http_method, const char *url, const void *body, SIZE_T body_len, char *out_token, SIZE_T token_max_len, char *out_sig, SIZE_T sig_max_len)
{
    BYTE req[16384];
    SIZE_T req_len = 0;
    BYTE resp[16384];
    SIZE_T resp_len = 0;
    HRESULT hr;

    if (!out_token || token_max_len == 0) return E_POINTER;
    snprintf(out_token, token_max_len, "%s", "MOCK_XSTS_TOKEN");
    if (out_sig && sig_max_len > 0) snprintf(out_sig, sig_max_len, "%s", "");

    req[req_len++] = (1 << 3) | 0;
    req_len += encode_varint(user_id, &req[req_len]);

    if (relying_party && relying_party[0])
    {
        SIZE_T rp_len = strlen(relying_party);
        req[req_len++] = (2 << 3) | 2;
        req_len += encode_varint((UINT64)rp_len, &req[req_len]);
        memcpy(&req[req_len], relying_party, rp_len);
        req_len += rp_len;
    }

    if (http_method && http_method[0])
    {
        SIZE_T hm_len = strlen(http_method);
        req[req_len++] = (3 << 3) | 2;
        req_len += encode_varint((UINT64)hm_len, &req[req_len]);
        memcpy(&req[req_len], http_method, hm_len);
        req_len += hm_len;
    }

    if (url && url[0])
    {
        SIZE_T url_len = strlen(url);
        req[req_len++] = (4 << 3) | 2;
        req_len += encode_varint((UINT64)url_len, &req[req_len]);
        memcpy(&req[req_len], url, url_len);
        req_len += url_len;
    }

    if (body && body_len > 0 && (req_len + body_len + 10 < sizeof(req)))
    {
        req[req_len++] = (6 << 3) | 2;
        req_len += encode_varint((UINT64)body_len, &req[req_len]);
        memcpy(&req[req_len], body, body_len);
        req_len += body_len;
    }

    TRACE( "[GDK IPC] Requesting XSTS token for user %llu, relying_party '%s', method '%s', url '%s'...\n",
           (unsigned long long)user_id, relying_party ? relying_party : "<none>",
           http_method ? http_method : "<none>", url ? url : "<none>" );

    hr = send_ipc_message(XODUS_MSG_XUSER_GET_TOKEN_REQUEST, req, req_len, resp, sizeof(resp), &resp_len);
    if (SUCCEEDED(hr))
    {
        SIZE_T payload_len = 0;
        const BYTE *payload = pb_get_xodus_payload(resp, resp_len, &payload_len);
        if (payload)
        {
            SIZE_T tok_len = 0;
            const BYTE *tok_ptr = pb_find_field(payload, payload_len, 2, 2, &tok_len);
            if (tok_ptr && tok_len > 0)
            {
                SIZE_T copy_len = tok_len < token_max_len - 1 ? tok_len : token_max_len - 1;
                memcpy(out_token, tok_ptr, copy_len);
                out_token[copy_len] = '\0';
                TRACE( "[GDK IPC] Successfully received token (length: %zu bytes)\n", copy_len );
            }

            if (out_sig && sig_max_len > 0)
            {
                SIZE_T sig_len = 0;
                const BYTE *sig_ptr = pb_find_field(payload, payload_len, 3, 2, &sig_len);
                if (sig_ptr && sig_len > 0)
                {
                    SIZE_T copy_len = sig_len < sig_max_len - 1 ? sig_len : sig_max_len - 1;
                    memcpy(out_sig, sig_ptr, copy_len);
                    out_sig[copy_len] = '\0';
                    TRACE( "[GDK IPC] Successfully received signature (length: %zu bytes)\n", copy_len );
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "[GDK IPC] ERROR: send_ipc_message failed for XSTS token (hr=0x%08lx)\n", hr);
    }
    return S_OK;
}

HRESULT ipc_xuser_get_gamer_picture(UINT64 user_id, DWORD picture_size, void *out_buf, SIZE_T buf_len, SIZE_T *out_used)
{
    BYTE req[64];
    SIZE_T req_len = 0;
    BYTE resp[65536];
    SIZE_T resp_len = 0;
    HRESULT hr;

    req[req_len++] = (1 << 3) | 0;
    req_len += encode_varint(user_id, &req[req_len]);

    req[req_len++] = (2 << 3) | 0;
    req_len += encode_varint((UINT64)picture_size, &req[req_len]);

    hr = send_ipc_message(17 /* XODUS_MSG_XUSER_GET_GAMER_PICTURE_REQUEST */, req, req_len, resp, sizeof(resp), &resp_len);
    if (SUCCEEDED(hr))
    {
        SIZE_T payload_len = 0;
        const BYTE *payload = pb_get_xodus_payload(resp, resp_len, &payload_len);
        if (payload)
        {
            SIZE_T pic_len = 0;
            const BYTE *pic_ptr = pb_find_field(payload, payload_len, 2, 2, &pic_len);
            if (pic_ptr && pic_len > 0)
            {
                SIZE_T copy_len = pic_len < buf_len ? pic_len : buf_len;
                if (out_buf && copy_len > 0)
                    memcpy(out_buf, pic_ptr, copy_len);
                if (out_used)
                    *out_used = pic_len;
                TRACE( "[GDK IPC] Successfully received gamer picture (%zu bytes)\n", pic_len );
                return S_OK;
            }
        }
    }

    /* Fallback 4-byte PNG */
    {
        BYTE png_header[4] = {0x89, 0x50, 0x4E, 0x47};
        SIZE_T copy_len = buf_len < 4 ? buf_len : 4;
        if (out_buf && copy_len > 0)
            memcpy(out_buf, png_header, copy_len);
        if (out_used)
            *out_used = copy_len;
    }
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
HRESULT ipc_xstore_query_license_token(UINT64 user_id, const char *product_id, char *out_token, SIZE_T token_max_len)
{
    BYTE req[1024];
    SIZE_T req_len = 0;
    BYTE resp[16384];
    SIZE_T resp_len = 0;
    HRESULT hr;

    if (!out_token || token_max_len == 0) return E_POINTER;
    snprintf(out_token, token_max_len, "%s", "MOCK_LIC_TOKEN");

    req[req_len++] = (1 << 3) | 0;
    req_len += encode_varint(user_id, &req[req_len]);

    if (product_id && product_id[0])
    {
        SIZE_T pid_len = strlen(product_id);
        req[req_len++] = (2 << 3) | 2;
        req_len += encode_varint((UINT64)pid_len, &req[req_len]);
        memcpy(&req[req_len], product_id, pid_len);
        req_len += pid_len;
    }

    hr = send_ipc_message(XODUS_MSG_XSTORE_QUERY_LICENSE_TOKEN_REQUEST, req, req_len, resp, sizeof(resp), &resp_len);
    if (SUCCEEDED(hr))
    {
        SIZE_T payload_len = 0;
        const BYTE *payload = pb_get_xodus_payload(resp, resp_len, &payload_len);
        if (payload)
        {
            SIZE_T str_len = 0;
            const BYTE *tok_ptr = pb_find_field(payload, payload_len, 2, 2, &str_len);
            if (tok_ptr && str_len > 0)
            {
                SIZE_T copy_len = str_len < token_max_len - 1 ? str_len : token_max_len - 1;
                memcpy(out_token, tok_ptr, copy_len);
                out_token[copy_len] = 0;
                TRACE( "[GDK IPC] Successfully received license token (length: %zu bytes)\n", copy_len );
            }
        }
    }
    return S_OK;
}
