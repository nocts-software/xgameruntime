<h1 align="center">xgameruntime</h1>
<p align="center"><strong>Open-Source Wine & Proton Implementation of the Microsoft Gaming Runtime (GDK)</strong></p>
<p align="center">
    <a href="https://github.com/nocts-software/xgameruntime/releases">
        <img src="https://img.shields.io/github/v/release/nocts-software/xgameruntime?style=for-the-badge&color=blue" alt="Latest Release" />
    </a>
    <a href="https://discord.gg/ZG774FK4tq">
        <img src="https://img.shields.io/discord/1123890623586504714?logo=discord&style=for-the-badge&color=green&label=Discord" alt="Discord" />
    </a>
    <a href="https://ko-fi.com/noct1">
        <img src="https://img.shields.io/badge/Ko--fi-F16061?logo=ko-fi&logoColor=white&style=for-the-badge" alt="Support on Ko-fi" />
    </a>
    <a href="https://www.patreon.com/cw/nocatix">
        <img src="https://img.shields.io/badge/Patreon-FF424D?logo=patreon&logoColor=white&style=for-the-badge" alt="Support on Patreon" />
    </a>
    <img src="https://img.shields.io/badge/Platform-Wine%20%7C%20Proton-red?style=for-the-badge" alt="Platform" />
    <img src="https://img.shields.io/badge/License-LGPL--2.1-purple?style=for-the-badge" alt="License" />
</p>

> [!CAUTION]
> **Clean-Room Reverse Engineering**: This project is developed adhering strictly to [Wine's clean-room reverse engineering guidelines](https://gitlab.winehq.org/wine/wine/-/wikis/Clean-Room-Guidelines). It is not affiliated with, endorsed by, or sponsored by Microsoft Corporation or Xbox.

---

## 🌟 Overview

`xgameruntime` is an open-source Winelib implementation of the Microsoft Game Development Kit (GDK) runtime libraries (`xgameruntime.dll`, `twinapi.appcore.dll`, and `api-ms-win-core-psm-appnotify-l1-1-0.dll`).

It enables native execution of Windows GDK games (including Microsoft Store and Xbox Game Pass titles) under **Wine** and **Proton** without requiring proprietary Windows runtime binaries.

This solution provides comprehensive inter-process communication (IPC) wiring with [`xodus-service`](https://github.com/nocts-software/xodus) and [`xodus-gui`](https://github.com/nocts-software/xodus), delivering seamless user authentication, cloud save synchronization, and automated Proton prefix DLL override setups.

---

## ✨ Features & Implemented Subsystems

- **🧵 Task Queues & Asynchronous Dispatching (`XThreading`)**:
  - Full implementation of `XTaskQueue` supporting FIFO Work and Completion callback ports.
  - Thread-safe callback registration, dispatching (`XTaskQueueDispatch`), and asynchronous notification helpers (`complete_async`).
  - Automatic integration with game main loops and frame dispatchers.

- **💾 Persistent Local Storage (PLS) (`XPersistentLocalStorage`)**:
  - Implements `XPersistentLocalStorageGetPathSize` and `XPersistentLocalStorageGetPath`.
  - Maps persistent title storage directories into user application state folders inside the Wine prefix.

- **👤 User Management & Xbox Live Authentication (`XUser`)**:
  - Live resolution of user Gamertags, modern Gamertags with suffix numbers, PUIDs, and XUIDs.
  - Asynchronous token acquisition (`XUserGetTokenAndSignatureAsync`, `XUserGetMsaTokenSilentlyAsync`) interfaced via Unix IPC with `xodus-service`.

- **📦 Package Management & Licensing (`XPackage`, `XStore`)**:
  - Package identity querying, package installation state, and mount handles.
  - Store license querying and entitlement verification.

- **🌐 Networking & Social APIs (`XNetworking`, `XGameInvite`, `XGameUI`)**:
  - Title networking connectivity status and security state queries.
  - Game invitation handlers and user profile UI invocations.

- **🧩 Modern Wine Architecture**:
  - Provides Wine PE fake module stubs (`winebuild --fake-module`) paired with Linux ELF Winelib shared objects (`x86_64-unix/`).
  - Compatible with Wine 8.0+, Wine 9.0+, Wine 10.0+, GE-Proton, and Proton CachyOS.

---

## 🏗️ Architecture & Module Structure

```
xgameruntime/
├── xgameruntime.spec             # Export specifications for xgameruntime.dll
├── twinapi.appcore.spec          # Export specifications for twinapi.appcore.dll
├── api-ms-win-core-psm-appnotify-l1-1-0.spec # Forwarder to twinapi.appcore.dll
├── main.c                        # DllMain, InitializeApiImpl, QueryApiImpl, UninitializeApiImpl
├── ipc.c / ipc.h                 # Unix domain socket IPC client for xodus-service
├── xthreading.c                  # XTaskQueue and async callback queue dispatching
├── xpersistentlocalstorage.c     # Persistent local storage directory management
├── xuser.c                       # User identity and MSA token async operations
├── xstore.c                      # Store entitlements and licensing
├── xpackage.c                    # Package inspection and identity
├── xnetworking.c                 # Network connectivity and socket initialization
├── xgameui.c / xgamesave.c       # Game UI dialogs and Connected Storage saves
└── build-xgameruntime.sh         # Complete WIDL header and Wine library compilation script
```

---

## 🛠️ Building

### Prerequisites

- **Wine development tools**: `widl`, `winegcc`, `winebuild`, and Wine development C headers (`wine-devel` / `wine-cachyos-devel`).
- Standard C build tools (`gcc`, `make`).

### Build Script

To generate all IDL headers, compile the Winelib `.so` libraries, and create the Wine PE stub DLLs, simply run:

```bash
chmod +x build-xgameruntime.sh
./build-xgameruntime.sh
```

This will produce:
- `xgameruntime.dll` (PE stub) & `x86_64-unix/xgameruntime.so` (Winelib ELF)
- `twinapi.appcore.dll` (PE stub) & `x86_64-unix/twinapi.appcore.so` (Winelib ELF)
- `api-ms-win-core-psm-appnotify-l1-1-0.dll` (PE stub forwarder)

---

## 🚀 Usage with Proton & Wine

To use `xgameruntime` with a game in Proton or Wine:

1. **Install PE Stubs into Prefix**:
   Copy `xgameruntime.dll`, `twinapi.appcore.dll`, and `api-ms-win-core-psm-appnotify-l1-1-0.dll` to your Proton prefix's `drive_c/windows/system32/` directory (or place them alongside the game executable).

2. **Configure Environment Variables**:
   ```bash
   export WINEDLLPATH="/path/to/xgameruntime:${WINEDLLPATH}"
   export WINEDLLOVERRIDES="xgameruntime=n,b;twinapi.appcore=n,b;api-ms-win-core-psm-appnotify-l1-1-0=n,b"
   ```

3. **Launch the Game**:
   ```bash
   wine GameExecutable.exe
   # or with Proton
   proton run GameExecutable.exe
   ```

## 🤝 Related Projects & Upstream
 
 - **[Xodus](https://github.com/nocts-software/xodus)** ([upstream](https://github.com/xodus-gaming/xodus)): Native Linux client and package downloader for Xbox Game Pass and Microsoft Store games.
 - **[xgameruntime-docs](https://github.com/nocts-software/xgameruntime-docs)** ([upstream](https://github.com/xodus-gaming/xgameruntime-docs)): Deep technical documentation and reverse-engineered specifications of GDK COM interfaces.
 - **Original Repositories**:
   - [xodus-gaming/xgameruntime](https://github.com/xodus-gaming/xgameruntime)
   - [xodus-gaming/xodus](https://github.com/xodus-gaming/xodus)
   - [xodus-gaming/xgameruntime-docs](https://github.com/xodus-gaming/xgameruntime-docs)

## 🔒 Clean-Room Interoperability & Anti-Piracy Notice

`xgameruntime` is an open-source clean-room reimplementation of Microsoft Gaming Runtime (GDK) APIs designed exclusively to provide compatibility for running Windows games on Linux under Wine and Proton:

- **No DRM Bypass**: `xgameruntime` does not bypass, strip, or alter digital rights management. It provides standard Win32/COM function shims that route store, entitlement, and user queries to legitimate platform APIs.
- **Authentic Token Verification**: Online authentication (`XUserGetTokenAndSignatureUtf16Async`) requires genuine cryptographic tokens issued by Xbox Live servers for the signed-in user.

---

## 💖 Support the Project

If you find `xgameruntime` valuable for gaming on Linux with Wine and Proton, please consider supporting development:

- ☕ **Ko-fi**: [ko-fi.com/noct1](https://ko-fi.com/noct1)
- 🧡 **Patreon**: [patreon.com/cw/nocatix](https://www.patreon.com/cw/nocatix)

---

## 📜 License & TL;DR

This library is distributed under the **GNU Lesser General Public License v2.1 (LGPL-2.1)**. See [LICENSE](LICENSE) for details.

### 📋 License Summary (TL;DR)

| ✅ What you CAN do | ❌ What you CANNOT do | ⚠️ What you MUST do |
|---|---|---|
| • **Commercial Use**: Use and bundle in commercial software | • **Hold Liable**: Software provided without warranty | • **Disclose Library Changes**: Release modifications to `xgameruntime` |
| • **Modification**: Freely modify the library's code | • **Close-Source the Library**: Re-license library as proprietary | • **Same License for Library**: Modified library stays LGPL-2.1 |
| • **Distribution**: Distribute original or modified binaries | | • **Allow Relinking**: Users must be able to replace/update library |
| • **Dynamic Linking**: Link with proprietary/open-source apps | | • **Include Notice**: Retain copyright and license notices |

---

### Acknowledgments
Special thanks to the original [xodus-gaming](https://github.com/xodus-gaming) team for founding the project and pioneering open-source GDK runtime development for Wine.
