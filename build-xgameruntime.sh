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
    flat_exports.c \
    xaml.c \
    -lgdi32 -luser32 -lcombase -lshell32 \
    -o xgameruntime.dll

echo "=== Compiling twinapi.appcore.dll with winegcc ==="
winegcc -shared -m64 -I. -D__WINESRC__ twinapi.appcore.spec \
    twinapi_appcore.c \
    -lgdi32 -luser32 -lcombase -lshell32 \
    -o twinapi.appcore.dll

echo "=== Compiling api-ms-win-core-psm-appnotify-l1-1-0.dll with winegcc ==="
winegcc -shared -m64 api-ms-win-core-psm-appnotify-l1-1-0.spec \
    -o api-ms-win-core-psm-appnotify-l1-1-0.dll

echo "=== Generating Wine PE fake modules and Unix Winelib SOs ==="
mkdir -p x86_64-unix
cp xgameruntime.dll.so x86_64-unix/xgameruntime.so
cp twinapi.appcore.dll.so x86_64-unix/twinapi.appcore.so
cp api-ms-win-core-psm-appnotify-l1-1-0.dll.so x86_64-unix/api-ms-win-core-psm-appnotify-l1-1-0.so

# Keep wintypes and windows.ui.core.textinput in sync with xgameruntime
cp xgameruntime.dll.so wintypes.dll.so
cp xgameruntime.dll.so windows.ui.core.textinput.dll.so
cp xgameruntime.dll.so x86_64-unix/wintypes.so
cp xgameruntime.dll.so x86_64-unix/windows.ui.core.textinput.so

winebuild --fake-module --dll -m64 -E xgameruntime.spec -o xgameruntime.dll
winebuild --fake-module --dll -m64 -E twinapi.appcore.spec -o twinapi.appcore.dll
winebuild --fake-module --dll -m64 -E api-ms-win-core-psm-appnotify-l1-1-0.spec -o api-ms-win-core-psm-appnotify-l1-1-0.dll
winebuild --fake-module --dll -m64 -E xgameruntime.spec -o wintypes.dll
winebuild --fake-module --dll -m64 -E xgameruntime.spec -o windows.ui.core.textinput.dll

echo "=== Build successful! Generated xgameruntime.dll, twinapi.appcore.dll, and forwarders ==="


