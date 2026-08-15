#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Building xgameruntime.dll IDL headers with widl ==="
IDLS=(
    xaccessibility.idl
    xappcapture.idl
    xasyncprovider.idl
    xdisplay.idl
    xerror.idl
    xgame.idl
    xgameactivation.idl
    xgameevent.idl
    xgameinvite.idl
    xgameprotocol.idl
    xgameruntimefeature.idl
    xgamesave.idl
    xgamestreaming.idl
    xgameui.idl
    xnetworking.idl
    xpackage.idl
    xpersistentlocalstorage.idl
    xstore.idl
    xsystem.idl
    xuser.idl
)

for idl in "${IDLS[@]}"; do
    header="${idl%.idl}.h"
    echo "  Generating $header from $idl..."
    widl -D__WINESRC__ -I. -h -o "$header" "$idl"
done



echo "=== Compiling xgameruntime.dll PE library with winegcc ==="
winegcc -shared -m64 -I. -D__WINESRC__ xgameruntime.spec \
    ipc.c \
    main.c \
    xaccessibility.c \
    xappcapture.c \
    xdisplay.c \
    xerror.c \
    xgame.c \
    xgameactivation.c \
    xgameevent.c \
    xgameinvite.c \
    xgameprotocol.c \
    xgameruntimefeature.c \
    xgamesave.c \
    xgamestreaming.c \
    xgameui.c \
    xnetworking.c \
    xpackage.c \
    xpersistentlocalstorage.c \
    xstore.c \
    xsystem.c \
    xsystemanalytics.c \
    xthreading.c \
    xuser.c \
    xaml.c \
    -lgdi32 -luser32 -lcombase \
    -o xgameruntime.dll

echo "=== Compiling twinapi.appcore.dll with winegcc ==="
winegcc -shared -m64 -I. -D__WINESRC__ twinapi.appcore.spec \
    twinapi_appcore.c \
    -lgdi32 -luser32 -lcombase \
    -o twinapi.appcore.dll

echo "=== Build successful! Generated xgameruntime.dll and twinapi.appcore.dll ==="

