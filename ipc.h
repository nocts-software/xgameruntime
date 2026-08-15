/*
 * Xbox Game runtime Library - IPC Client
 *
 * Copyright 2026 Xodus Project
 */

#ifndef XGAMERUNTIME_IPC_H
#define XGAMERUNTIME_IPC_H

#include <windows.h>
#include <winerror.h>

#define PROTO_MAGIC 0x58445350 /* "XDSP" */

typedef enum {
    XODUS_MSG_UNKNOWN = 0,
    XODUS_MSG_PING = 1,
    XODUS_MSG_PONG = 2,
    XODUS_MSG_MSA_TOKEN_REQUEST = 3,
    XODUS_MSG_MSA_TOKEN_RESPONSE = 4,
    XODUS_MSG_XUSER_ADD_REQUEST = 5,
    XODUS_MSG_XUSER_ADD_RESPONSE = 6,
    XODUS_MSG_XUSER_GET_GAMERTAG_REQUEST = 7,
    XODUS_MSG_XUSER_GET_GAMERTAG_RESPONSE = 8,
    XODUS_MSG_XUSER_GET_TOKEN_REQUEST = 9,
    XODUS_MSG_XUSER_GET_TOKEN_RESPONSE = 10,
    XODUS_MSG_XSTORE_QUERY_LICENSE_REQUEST = 11,
    XODUS_MSG_XSTORE_QUERY_LICENSE_RESPONSE = 12,
    XODUS_MSG_XGAMESAVE_READ_BLOB_REQUEST = 13,
    XODUS_MSG_XGAMESAVE_READ_BLOB_RESPONSE = 14,
    XODUS_MSG_XGAMESAVE_WRITE_BLOB_REQUEST = 15,
    XODUS_MSG_XGAMESAVE_WRITE_BLOB_RESPONSE = 16,
} XodusMessageType;

typedef struct {
    UINT64 user_id;
    char xuid[32];
    char gamertag[64];
} XodusUserInfo;

HRESULT ipc_init(void);
void ipc_cleanup(void);
HRESULT ipc_xuser_add(UINT32 options, XodusUserInfo *out_user);
HRESULT ipc_xuser_get_gamertag(UINT64 user_id, char *out_gamertag, SIZE_T max_len);
HRESULT ipc_xuser_get_token(UINT64 user_id, const char *relying_party, char *out_token, SIZE_T token_max_len, char *out_sig, SIZE_T sig_max_len);
HRESULT ipc_xuser_get_gamer_picture(UINT64 user_id, DWORD picture_size, void *out_buf, SIZE_T buf_len, SIZE_T *out_used);
HRESULT ipc_xuser_check_privilege(UINT64 user_id, UINT32 privilege, BOOL *out_has_privilege, UINT32 *out_deny_reason);
HRESULT ipc_xstore_query_license(UINT64 user_id, const char *product_id, BOOL *out_is_licensed);
HRESULT ipc_xstore_query_products(UINT64 user_id, UINT32 product_kinds, const char *store_ids, char *out_products_json, SIZE_T max_len, SIZE_T *out_count);
HRESULT ipc_xstore_acquire_license(UINT64 user_id, const char *package_id, BOOL *out_is_licensed, void *out_blob, SIZE_T max_len, SIZE_T *out_used);


HRESULT ipc_xgamesave_read_blob(UINT64 user_id, const char *scid, const char *container, const char *blob, void *out_buf, SIZE_T buf_len, SIZE_T *out_read);
HRESULT ipc_xgamesave_write_blob(UINT64 user_id, const char *scid, const char *container, const char *blob, const void *buf, SIZE_T buf_len);

#endif /* XGAMERUNTIME_IPC_H */
