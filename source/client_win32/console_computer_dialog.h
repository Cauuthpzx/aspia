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
// Win32 replacement for console/computer_dialog.ui (+ its three page .ui
// files: computer_dialog_general.ui, computer_dialog_desktop.ui,
// computer_dialog_parent.ui).
//
// The Qt original is a QSplitter with a QTreeWidget on the left that picks
// between three pages (General info, Desktop session config, Parent group
// reference). A plain Win32 DIALOG does not use this pattern in this
// project; the pages are flattened into titled GROUPBOXes arranged
// vertically, following the same approach used by settings_dialog.h and
// host_config_dialog.h.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to
// source/client_win32/resource.h by the resource-file owner (do NOT edit
// resource.h from here):
//
//   #define IDD_CONSOLE_COMPUTER                              480
//
//   // --- General page -----------------------------------------------------
//   #define IDC_CONSOLE_COMPUTER_NAME                        7100
//   #define IDC_CONSOLE_COMPUTER_PARENT_NAME                 7101
//   #define IDC_CONSOLE_COMPUTER_ADDRESS                     7102
//   #define IDC_CONSOLE_COMPUTER_INHERIT_CREDS               7103
//   #define IDC_CONSOLE_COMPUTER_USERNAME                    7104
//   #define IDC_CONSOLE_COMPUTER_PASSWORD                    7105
//   #define IDC_CONSOLE_COMPUTER_SHOW_PASSWORD               7106
//   #define IDC_CONSOLE_COMPUTER_COMMENT                     7107
//
//   // --- Desktop page -----------------------------------------------------
//   #define IDC_CONSOLE_COMPUTER_INHERIT_CONFIG              7120
//   #define IDC_CONSOLE_COMPUTER_AUDIO                       7121
//   #define IDC_CONSOLE_COMPUTER_CLIPBOARD                   7122
//   #define IDC_CONSOLE_COMPUTER_CURSOR_SHAPE                7123
//   #define IDC_CONSOLE_COMPUTER_CURSOR_POSITION             7124
//   #define IDC_CONSOLE_COMPUTER_DISABLE_EFFECTS             7125
//   #define IDC_CONSOLE_COMPUTER_DISABLE_WALLPAPER           7126
//   #define IDC_CONSOLE_COMPUTER_LOCK_AT_DISCONNECT          7127
//   #define IDC_CONSOLE_COMPUTER_BLOCK_REMOTE_INPUT          7128
//
//   // --- Parent page ------------------------------------------------------
//   #define IDC_CONSOLE_COMPUTER_PARENT_PATH                 7140
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_CONSOLE_COMPUTER_DIALOG_H
#define CLIENT_WIN32_CONSOLE_COMPUTER_DIALOG_H

#include <windows.h>

#include <cstdint>
#include <string>

namespace aspia::client_win32 {

class ConsoleComputerDialog
{
public:
    struct Result
    {
        // --- General page ------------------------------------------------
        // Mirrors computer_dialog_general.ui. Inherits credentials from the
        // parent group when inheritCreds is true; in that case username and
        // password are ignored on save.
        struct General
        {
            std::wstring name;                   // edit_name,    max 64
            std::wstring parentName;             // edit_parent_name (readonly)
            std::wstring address;                // edit_address, max 64
            uint16_t     port            = 0;    // parsed from address suffix
            bool         inheritCreds    = true; // groupbox_inherit_creds
            std::wstring username;               // edit_username, max 64
            std::wstring password;               // edit_password, max 64
            std::wstring comment;                // edit_comment (multiline)
        } general;

        // --- Desktop page ------------------------------------------------
        // Mirrors computer_dialog_desktop.ui. When inheritConfig is true the
        // remaining booleans reflect the parent group's values and should
        // not be written back to this entry individually.
        struct Desktop
        {
            bool inheritConfig          = true;  // checkbox_inherit_config

            // groupbox_features
            bool enableAudio            = true;
            bool enableClipboard        = true;
            bool enableCursorShape      = true;
            bool enableCursorPos        = false;

            // groupbox_appearance
            bool disableDesktopEffects   = false;
            bool disableDesktopWallpaper = false;

            // groupbox_other
            bool lockAtDisconnect       = false;
            bool blockRemoteInput       = false;
        } desktop;

        // --- Parent page -------------------------------------------------
        // Mirrors computer_dialog_parent.ui. The Qt page itself is mostly
        // informational ("Select child item"); we carry the parent group's
        // fully-qualified path so the dialog can display it read-only.
        struct Parent
        {
            std::wstring path;                   // e.g. L"Root/Servers/DC1"
        } parent;
    };

    ConsoleComputerDialog(HINSTANCE instance, HWND parent);
    ~ConsoleComputerDialog() = default;

    ConsoleComputerDialog(const ConsoleComputerDialog&) = delete;
    ConsoleComputerDialog& operator=(const ConsoleComputerDialog&) = delete;

    // Pre-populates the dialog fields. Call before exec() to reuse this
    // dialog as an "Edit computer" dialog.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);
    void onInheritCredsToggled(HWND hwnd);
    void onInheritConfigToggled(HWND hwnd);
    void togglePasswordVisibility(HWND hwnd);

    void updateInheritCredsEnables(HWND hwnd);
    void updateInheritConfigEnables(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
    bool passwordVisible_ = false;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_CONSOLE_COMPUTER_DIALOG_H
