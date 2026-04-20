# Phase 2 — Qt → Win32 Rewrite Plan

**Status:** In progress. Phase 1 (drop non-Windows) is complete. This document
tracks the ongoing removal of Qt from every Aspia binary.

## Scope measured at start of Phase 2

| Metric | Count |
|---|---|
| `.ui` files (Qt Designer forms) | **72** |
| Files with `Q_OBJECT` (signals/slots, require MOC) | **183** |
| Files using `QWidget`/`QDialog`/... (Qt UI) | **186** |
| Files touching `QString`/`QByteArray` (Qt data types) | **513** |
| Files issuing `connect(...)` | **135** |
| Files using `QSqlDatabase`/`QSqlQuery` | 2 (`console/`, `router/database.cc`) |
| Files using `QXmlStream`/`QDomDocument` | 3 (incl. `base/xml_settings.cc`) |
| Qt Linguist `.ts` files | 22 |
| Total `.cc`/`.h` | 861 |

**~60% of the C++ files touch Qt directly.** This is a multi-month rewrite.

## Target architecture

- **UI**: Win32 API + Common Controls v6 (`comctl32.dll` side-by-side manifest).
  Custom controls drawn with GDI / Direct2D where needed (desktop viewer,
  chat bubbles). No framework above Win32.
- **Data types**: `std::string` (UTF-8) + `std::wstring` (UTF-16, for Win32 API).
  Convert at the boundary with `MultiByteToWideChar` / `WideCharToMultiByte`.
- **Event loop**: Win32 message pump in the main thread. Asio `io_context`
  runs on a worker thread and posts results back via `PostMessage` +
  custom `WM_APP+N` messages.
- **SQL**: direct `sqlite3` C API (already shipped via vcpkg as a transitive
  dependency of Qt; switch to the `sqlite3` vcpkg port).
- **XML/settings**: `pugixml` (vcpkg).
- **Translations**: gettext-style lookup against compiled `.mo`, or a
  hand-rolled `std::unordered_map<std::string, std::wstring>` loaded from
  a single binary blob generated at build time. `.ts` files will be
  discarded (or kept for translator reference, re-emitted through a
  script).
- **Protobuf**: replace custom `qtprotoc` with stock `protoc` + the
  `libprotobuf-lite` C++ runtime. Generated types use `std::string` etc.
  natively.

## Work packages

### WP1 — Skeleton (DONE, this commit)

- [x] `source/client_win32/` skeleton with main window, menu, accelerators,
      status bar, tab control. Links only to `comctl32`/`uxtheme`/etc.
- [x] Registered as `aspia_client_win32` target in `source/CMakeLists.txt`.

### WP2 — Base-layer de-Qt-ification

Goal: make `aspia_base` compile without any Qt header.

- [ ] Replace `QString` in public headers with `std::u16string` (or
      `std::wstring` on Windows, since they are the same). Keep a thin
      `aspia::string` alias while the conversion is in flight.
- [ ] Rewrite `base/xml_settings.{cc,h}` on top of `pugixml`.
- [ ] Replace `base/translations.{cc,h}` with a Win32-resource-backed
      lookup table (or `FormatMessage`-based string table from `.mc`).
- [ ] Replace `base/asio_event_dispatcher.{cc,h}` with a Win32 message-pump
      bridge: Asio runs on a worker thread, results posted to the UI
      thread through `PostMessage(WM_APP + N, ...)`.
- [ ] Replace Qt `QLoggingCategory` usage in `base/logging.{cc,h}` with a
      plain `OutputDebugStringW` / rotating-file sink.
- [ ] Port `base/gui_application.{cc,h}` to Win32 `HINSTANCE`/message loop.

**Blocking point:** `QString` is exposed in every public API of `base`.
Conversion must ripple through consumers in lockstep.

### WP3 — Protobuf regeneration

- [ ] Rewrite `source/tools/qtprotoc/` so it either:
      (a) wraps stock `protoc` and emits `std::string`-based message types;
      or (b) is deleted entirely and replaced by a direct `protoc` call
      in `source/proto/CMakeLists.txt`.
- [ ] Regenerate all proto headers. Audit every `.cc` file for code that
      passed `QByteArray` directly to protobuf APIs.

### WP4 — Common layer

Goal: `aspia_common` compiles without Qt.

- [ ] `common/clipboard_win.{cc,h}` — drop `QMimeData`, use `OpenClipboard`
      / `SetClipboardData` directly.
- [ ] `common/keycode_converter.{cc,h}` — drop `Qt::Key`, map directly
      from Win32 `VK_*` to USB HID usage codes.
- [ ] `common/file_packet*.{cc,h}` — replace `QByteArray` with
      `std::string` (or `std::vector<uint8_t>` if binary-only).
- [ ] `common/update_checker.{cc,h}` — replace `QNetworkAccessManager`
      with `WinHTTP` (or keep the existing libcurl path).
- [ ] Delete `common/ui/` entirely; reimplement dialogs in
      `client_win32/` as Win32 templates.

### WP5 — Client UI rewrite (biggest piece)

Port each `.ui` file to a Win32 dialog template or a custom-drawn panel:

- [ ] `client/ui/authorization_dialog.ui` → `client_win32/auth_dialog.{cc,h}`
- [ ] `client/ui/main_window.ui` → already scaffolded (WP1)
- [ ] `client/ui/router_dialog.ui`
- [ ] `client/ui/settings_dialog.ui`
- [ ] `client/ui/desktop/desktop_config_dialog.ui`
- [ ] `client/ui/desktop/desktop_session_window.{cc,h}` — desktop viewer,
      swap `QImage`+`QPainter` for a D3D11 swap-chain + YUV→RGB pixel
      shader. Biggest perf win.
- [ ] `client/ui/desktop/desktop_toolbar.ui`
- [ ] `client/ui/desktop/record_settings_dialog.ui`
- [ ] `client/ui/desktop/statistics_dialog.ui`
- [ ] `client/ui/desktop/task_manager_window.ui`
- [ ] `client/ui/file_transfer/*.ui` (7 files) — dual-pane file manager
      with `SysListView32` in report mode.
- [ ] `client/ui/router_manager/*.ui` (2 files)
- [ ] `client/ui/sys_info/*.ui` (18 files) — trees/tables via
      `SysTreeView32`/`SysListView32`.
- [ ] `client/ui/chat/chat_session_window.ui` — custom-drawn chat bubbles
      on an owner-drawn `SysListView32` or custom panel.
- [ ] `client/ui/hosts/*.ui` (8 files)

Approx. **≥40 dialogs/windows** to port. Budget: one dialog per
engineer-day average, more for the desktop viewer and file manager.

### WP6 — Console

`source/console/` has ~30 `.ui` files for the address book. Port the same
way as WP5. Address book storage moves to `sqlite3` C API.

### WP7 — Host (service + agents)

Host service is mostly non-UI (good). `host/ui/` has ~10 `.ui` files for
notifier, user list, password dialogs — port after client.

### WP8 — Router, Relay

Non-UI binaries. Mostly Qt needed only for `QString`/`QSqlDatabase`
(router). Should fall out naturally after WP2 + WP3.

### WP9 — Cleanup

- [ ] Delete `source/client/` (legacy Qt client) once `aspia_client_win32`
      is feature-complete and stable.
- [ ] Delete `source/common/ui/` and `source/*/ui/` once migrated.
- [ ] Drop Qt from `vcpkg.json`. Downgrade compile defs
      (`QT_NO_CAST_TO_ASCII` etc.) in `source/CMakeLists.txt`.
- [ ] Delete `source/translations/*.ts` (or convert to `.mo`).
- [ ] Update `CLAUDE.md` and `PHASE2_QT_REMOVAL_PLAN.md` (this file) to
      reflect the final state.

## Guiding principles

1. **Every commit must keep `aspia_client_win32` buildable.** The legacy
   Qt targets can break temporarily, but the Win32 target is the
   contract.
2. **Strings: wide at the Win32 boundary, UTF-8 everywhere else.**
3. **No abstraction layer over Win32.** The point of this rewrite is
   fewer dependencies, not more.
4. **Keep proto wire format unchanged.** Old hosts must keep talking to
   new clients during the transition.
5. **One `.ui` → one `.cc/.h` pair.** Do not bundle multiple dialogs in
   a single file; it makes diffing against the Qt original impossible.

## Out of scope

- Porting the `drivers/` tree — already Win32 / WDM.
- Changing the protocol layer (`source/proto/`). Wire format is frozen
  during the rewrite.
- DPI scaling policy changes — keep `PerMonitorV2` from `client.manifest`.
