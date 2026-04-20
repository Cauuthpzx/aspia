# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Aspia is a C++20/Qt6 remote desktop, file transfer and system information suite. The distribution is broken into several separately installable components (Client, Host, Console, Router, Relay) that communicate over TCP/UDP using Protocol Buffers messages (`source/proto/*.proto`) with SRP-based authentication and an AES/ChaCha20 encryption layer. NAT traversal is supported via the Router + Relay infrastructure; direct peer-to-peer connections are also available.

Target platforms: Windows 7/2008R2+ (x86, x64, arm64), Debian 11/Ubuntu 20.04+ (x64, arm64), macOS (x64, arm64). Host is Windows/Linux only; Router and Relay are Windows/Linux only; Client and Console run on all three. The `drivers/` tree (mirror display + virtual display) is Windows-only.

## Build System

The project uses **CMake (≥3.30) with Ninja Multi-Config** and **vcpkg** (vendored under `vcpkg4aspia/`) for all third-party dependencies. Configuration is driven by platform-specific presets in `CMakePresets.json` — one preset per `(OS, arch)` pair.

### Common commands

```bash
# Configure (pick the preset matching your host)
cmake --preset local-linux-x64      # or local-mac-arm64, local-win-x64, ...

# Build (Release or Debug)
cmake --build --preset ci-release   # or ci-debug
# Equivalent: cmake --build builds/local-linux-x64 --config Release

# Run the full test suite
ctest --preset ci-release           # or ci-debug
# Equivalent: ctest --test-dir builds/local-linux-x64 -C Release

# Run a single test binary (each module with *_unittest.cc links into a test exe)
cd builds/local-linux-x64/Release && ./aspia_base_tests --gtest_filter=BitsetTest.*

# Build installer/package (uses CPack; .deb on Linux, .dmg on macOS, WiX on Windows)
cpack --config builds/local-linux-x64/CPackConfig.cmake -C Release

# Regenerate Qt .ts translation files from source
cmake --build builds/local-linux-x64 --target update_translations
```

On first configure, vcpkg will compile all dependencies (Qt6, OpenSSL, libvpx, libyuv, Opus, protobuf, enet, curl, asio, zstd, gtest, …) into `builds/<preset>/vcpkg_installed/`. This takes a long time; the CI caches `VCPKG_DEFAULT_BINARY_CACHE`. Do not expect a fresh configure to finish in seconds.

Windows uses static CRT (`x64-windows-static` triplet + `MultiThreaded[Debug]` runtime); Windows-only code additionally links against the WDK (see `FindWDK.cmake`).

## High-Level Architecture

### Component layout (`source/`)

- **`base/`** — Platform abstraction and shared runtime: threading, logging, XML settings, SMBIOS parsing, service control (systemd / Windows SCM), filesystem/IPC helpers, and subtrees used across every binary:
  - `base/crypto/` — SRP handshake (`srp_math`), BigNum, key pairs, symmetric stream/datagram cryptors, password hashing, OS keystore (`os_crypt_*`).
  - `base/net/` — Network primitives: adapter enumeration, `Address`, anti-replay window, connect enumerators; built on standalone Asio (`-DASIO_STANDALONE`).
  - `base/peer/` — Authenticated peer-to-peer channel built on top of `base/net` + `base/crypto` (the core session transport).
  - `base/ipc/` — Local IPC server/channel used between Host service and per-user agents.
  - `base/codec/` — Video (VP8/VP9 via libvpx, scale reducer), audio (Opus + resampler), cursor encoders/decoders, WebM muxing, zstd streams.
  - `base/desktop/`, `base/files/`, `base/audio/` — Platform capture / filesystem / audio abstractions with `_win`, `_mac`, `_linux`, `_x11` variants selected by `CMakeLists.txt`.
- **`proto/`** — All wire formats (`.proto` files). Protos are compiled via the custom `qtprotoc` tool (`source/tools/qtprotoc/`) rather than stock `protoc`, so that generated types interop cleanly with Qt containers/strings. Anything crossing a process or network boundary lives here.
- **`common/`** — Protocol-level helpers shared between Client and Host that are not part of `base`: clipboard (per-platform), file packet (de)packetizer, file task worker, keycode converter, system-info constants, auto-updater.
- **`client/`** — Cross-platform Qt GUI application and the per-feature session clients (`client_desktop`, `client_file_transfer`, `client_system_info`, `client_text_chat`). `client/ui/` holds the `.ui` / Widgets code for main window, desktop viewer, file manager, chat, sys-info, router manager, online checker. `client/online_checker/` is a separate component for probing host reachability.
- **`host/`** — Windows/Linux only. A multi-process design:
  - `service` — Long-running system service (SCM / systemd). Accepts incoming network connections.
  - `desktop_agent` — Per-user-session process launched by the service; owns the display/input/audio capture for that session. Communicates with `service` over `base/ipc`.
  - `file_agent` — Per-user-session process for file transfers (runs in user context so ACLs are correct).
  - `user_session`, `user_session_agent` — Glue that maps OS session IDs to agents.
- **`console/`** — Windows/Linux/macOS Qt GUI for administrators: address book (encrypted with a master password), computer groups, router user management. Entry point for launching Client sessions. Uses SQLite (`Qt6::QSQLiteDriverPlugin`) for the address book.
- **`router/`** — NAT-traversal rendezvous service. Maintains a SQLite user database (`database.cc`), accepts registrations from hosts (`session_host`, `session_legacy_host`), admin connections (`session_admin`), client lookups (`session_client`), and coordinates relays (`session_relay`).
- **`relay/`** — Packet-forwarding service used when peers cannot connect directly. Sessions are keyed (`session_key`) and managed by `session_manager`.
- **`drivers/`** — Windows kernel-adjacent components: `mirror/` (legacy mirror display driver sources) and `virtual_display/` (IddCx virtual display). Built only on Windows.
- **`third_party/`** — Vendored sources not available via vcpkg: `libwebm` (video recording), `portaudio` (fallback audio backend), `xdg_user_dirs`.
- **`tools/`**
  - `qtprotoc/` — Wrapper around protoc that emits Qt-friendly C++; used by `proto/CMakeLists.txt`.
  - `sha256/` — Build-time hashing utility (used for signing/integrity in installers).
- **`translations/`** — Qt Linguist `.ts` files, one per locale. Top-level `CMakeLists.txt` wires up `update_translations` (runs `lupdate`) and `qt_add_translation` (produces `.qm`, bundled as a Qt resource under prefix `tr/` into the `aspia_common` target).

### Binaries produced

Each component links to `aspia_base`, `aspia_common`, `aspia_proto`, plus Qt platform/private modules. Installable targets: `aspia_client`, `aspia_console`, `aspia_host` + `aspia_host_core` + `aspia_host_service` + `aspia_desktop_agent` + `aspia_file_agent` (Win/Linux), `aspia_router`, `aspia_relay`. On Linux, CPack produces per-component `.deb` packages with systemd units (`aspia-host-service.service`, `aspia-router.service`, `aspia-relay.service`) and XDG desktop files under `*/linux/`. Windows installers are WiX-based, defined in `installer/*.wxs`.

### Cross-cutting patterns

- **Unit tests** live next to their subject as `*_unittest.cc`, share `base/tests_main.cc` as their GoogleTest entry point, and are registered via `add_test(...)` in the owning `CMakeLists.txt`. Run a single case with `--gtest_filter=Pattern`.
- **Platform splits** follow the `_win.cc`, `_mac.mm`, `_linux.cc`, `_x11.cc`, `_posix.cc` suffix convention. The `CMakeLists.txt` in each module picks the right list based on `WIN32` / `LINUX` / `APPLE`.
- **Qt conventions enforced via compile defs:** `QT_NO_CAST_TO_ASCII`, `QT_NO_CAST_FROM_BYTEARRAY`, `QT_USE_QSTRINGBUILDER`. Asio is standalone with `ASIO_NO_DEPRECATED`. C++ standard is fixed at **C++20**. Windows builds target `_WIN32_WINNT=0x0601` (Windows 7).
- **Git metadata** (`GIT_CURRENT_BRANCH`, `GIT_COMMIT_HASH`, `GIT_COMMIT_COUNT`) and version (`ASPIA_VERSION_{MAJOR,MINOR,PATCH}`, currently 3.0.0) are injected into every translation unit via `add_definitions` in `source/CMakeLists.txt` / the root `CMakeLists.txt`. Runtime reads them from `source/build/version.h` and `base/version_constants.cc`.
- **Adding a new proto message:** add the `.proto` to `source/proto/`, register it in `source/proto/CMakeLists.txt`, then consume the generated header from whichever component needs it. Don't hand-write wire formats.

## CI

GitHub Actions workflows live at `.github/workflows/{linux,macos,windows}.yml`. Each runs the `ci` configure preset with the matching `VCPKG_TRIPLET` environment variable, builds both Release and Debug, runs `ctest`, and packages with `cpack`. The vcpkg binary cache directory (`$VCPKG_DEFAULT_BINARY_CACHE`) and the `vcpkg` executable itself are reused across runs — mirror this locally if you need fast rebuilds.
