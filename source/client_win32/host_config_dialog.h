//
// Aspia Project
// Copyright (C) 2016-2026 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// Win32 replacement for host/ui/config_dialog.ui.
//
// The Qt original uses a QTabWidget with four tabs (General, Security, Router,
// Users). A plain Win32 DIALOG does not have a tab control in this project's
// dialog style, so the tabs are flattened into titled GROUPBOXes arranged
// vertically -- the same approach taken by settings_dialog.h.
//
// When wiring this dialog up, add the following to
// source/client_win32/resource.h (do NOT edit that file from this port;
// these are the identifiers the implementation below expects). Note that the
// dialog template ID (IDD_HOST_CONFIG = 430) deliberately does not share a
// prefix with the control IDs: 4300..4399 are already taken by the hosts_tab
// controls, so config_dialog controls live in 6600..6799.
//
//   #define IDD_HOST_CONFIG                              430
//
//   // --- General tab ---------------------------------------------------------
//   #define IDC_HOST_CONFIG_PORT                         6600
//   #define IDC_HOST_CONFIG_ALLOW_UDP                    6601
//   #define IDC_HOST_CONFIG_VIDEO_CAPTURER               6602
//   #define IDC_HOST_CONFIG_AUTO_UPDATE                  6603
//   #define IDC_HOST_CONFIG_UPDATE_CHECK_FREQ            6604
//   #define IDC_HOST_CONFIG_USE_CUSTOM_UPDATE_SERVER     6605
//   #define IDC_HOST_CONFIG_UPDATE_SERVER                6606
//   #define IDC_HOST_CONFIG_CHECK_UPDATES_NOW            6607
//   #define IDC_HOST_CONFIG_IMPORT                       6608
//   #define IDC_HOST_CONFIG_EXPORT                       6609
//
//   // --- Security tab --------------------------------------------------------
//   #define IDC_HOST_CONFIG_CHANGE_PASSWORD              6620
//   #define IDC_HOST_CONFIG_PASS_PROTECTION              6621
//   #define IDC_HOST_CONFIG_ONETIME_PASSWORD_ENABLE      6622
//   #define IDC_HOST_CONFIG_ONETIME_PASS_CHANGE          6623
//   #define IDC_HOST_CONFIG_ONETIME_PASS_CHARS           6624
//   #define IDC_HOST_CONFIG_ONETIME_PASS_CHAR_COUNT      6625
//   #define IDC_HOST_CONFIG_CONN_CONFIRM_REQUIRE         6626
//   #define IDC_HOST_CONFIG_CONN_CONFIRM_AUTO            6627
//   #define IDC_HOST_CONFIG_NO_USER_ACTION               6628
//   #define IDC_HOST_CONFIG_DISABLE_SHUTDOWN             6629
//
//   // --- Router tab ----------------------------------------------------------
//   #define IDC_HOST_CONFIG_ENABLE_ROUTER                6640
//   #define IDC_HOST_CONFIG_ROUTER_ADDRESS               6641
//   #define IDC_HOST_CONFIG_ROUTER_PUBLIC_KEY            6642
//
//   // --- Users tab -----------------------------------------------------------
//   #define IDC_HOST_CONFIG_USERS_LIST                   6660
//   #define IDC_HOST_CONFIG_USER_ADD                     6661
//   #define IDC_HOST_CONFIG_USER_MODIFY                  6662
//   #define IDC_HOST_CONFIG_USER_DELETE                  6663
//
//   // --- Buttons -------------------------------------------------------------
//   #define IDC_HOST_CONFIG_APPLY                        6680
//

#ifndef CLIENT_WIN32_HOST_CONFIG_DIALOG_H
#define CLIENT_WIN32_HOST_CONFIG_DIALOG_H

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace aspia::client_win32 {

// Forward-declared; a sibling port owns this class. The Users tab's
// add/modify buttons will eventually launch it. See TODOs in the .cc.
class HostUserDialog;

class HostConfigDialog
{
public:
    // A single entry in the Users tab's list. The .ui stores users as a tree
    // widget, but only username + enabled flag are visible at the list level;
    // the rest lives in the per-user dialog.
    struct UserEntry
    {
        std::wstring username;
        bool         enabled = true;
        // Opaque per-user payload filled in by HostUserDialog when editing;
        // the config dialog treats it as a black box. Using a std::wstring as
        // a placeholder keeps this header dependency-free.
        std::wstring serialized_blob;
    };

    struct Result
    {
        // --- General tab (Qt: "widget" / tab_bar index 0) --------------------
        struct General
        {
            int          tcp_port              = 8050;
            bool         allow_udp             = false;
            std::wstring video_capturer;                // objectName in combo
            bool         auto_update_enabled   = true;
            int          update_check_freq     = 0;     // index into combo
            bool         use_custom_update_server = false;
            std::wstring update_server         = L"update.aspia.org";
        } general;

        // --- Users tab (Qt: "tab_users") ------------------------------------
        struct Users
        {
            std::vector<UserEntry> users;
        } users;

        // --- Service tab: the .ui does not actually have a dedicated
        //     "Service" tab -- the closest match is the "Common" group on the
        //     General tab. We keep the struct name requested by the port spec
        //     and map its fields onto the General controls. tcp_port mirrors
        //     general.tcp_port; udp_port is unused by the .ui (Qt only has an
        //     "Allow UDP" checkbox, not a separate port) and is retained for
        //     API completeness. ------------------------------------------------
        struct Service
        {
            int  tcp_port    = 8050;
            int  udp_port    = 0;
            bool enable_upnp = false;
        } service;

        // --- Router tab (Qt: "tab_router") ----------------------------------
        struct Router
        {
            bool         enabled    = false;
            std::wstring address;
            uint16_t     port       = 0;
            // The .ui exposes the router public key (multi-line edit); we
            // surface it as host_key per the port spec, and keep host_id as a
            // free-form identifier field (the .ui does not bind one directly,
            // but the host service tracks a generated ID).
            std::wstring host_id;
            std::wstring host_key;
        } router;

        // --- Advanced tab: the .ui's "Security" tab maps here; we keep the
        //     name as requested by the port spec. log_path / log_level are
        //     retained for parity with settings_dialog (the .ui itself
        //     exposes connection-confirmation / one-time-password controls,
        //     represented as the additional fields below). --------------------
        struct Advanced
        {
            std::wstring log_path;
            int          log_level = 0;

            // Password protection of settings.
            bool password_protect_settings = false;

            // One-time password group.
            bool one_time_password_enabled = false;
            int  one_time_pass_change      = 0;  // combo index
            int  one_time_pass_chars       = 0;  // combo index
            int  one_time_pass_char_count  = 8;

            // Connection confirmation group.
            bool require_connection_confirm = false;
            int  auto_confirm_via           = 0;  // combo index
            int  no_user_action             = 0;  // combo index

            // Other.
            bool disable_shutdown = false;
        } advanced;
    };

    HostConfigDialog(HINSTANCE instance, HWND parent);
    ~HostConfigDialog() = default;

    HostConfigDialog(const HostConfigDialog&) = delete;
    HostConfigDialog& operator=(const HostConfigDialog&) = delete;

    // Pre-populate the dialog with an existing configuration.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

    // Returns true if the user pressed Apply at least once before OK/Cancel.
    // The caller can use this to know whether to immediately persist the
    // result even on Cancel.
    bool applied() const { return applied_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);
    void onApply(HWND hwnd);

    void onAutoUpdateToggled(HWND hwnd);
    void onCustomUpdateServerToggled(HWND hwnd);
    void onEnableRouterToggled(HWND hwnd);
    void onOneTimePasswordToggled(HWND hwnd);
    void onRequireConfirmToggled(HWND hwnd);

    void onAddUser(HWND hwnd);
    void onModifyUser(HWND hwnd);
    void onDeleteUser(HWND hwnd);

    void onImport(HWND hwnd);
    void onExport(HWND hwnd);
    void onCheckForUpdatesNow(HWND hwnd);
    void onChangePassword(HWND hwnd);
    void onTogglePassProtection(HWND hwnd);

    void collectValues(HWND hwnd);
    void populateUsersList(HWND hwnd);

    void updateUpdateServerEnable(HWND hwnd);
    void updateRouterEnables(HWND hwnd);
    void updateOneTimePasswordEnables(HWND hwnd);
    void updateConnectionConfirmEnables(HWND hwnd);

    HINSTANCE instance_;
    HWND      parent_;
    Result    result_;
    bool      applied_ = false;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_HOST_CONFIG_DIALOG_H
