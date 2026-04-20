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
// Win32 replacement for console/computer_group_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_CONSOLE_COMPUTER_GROUP                  490
//
//   // General tab controls.
//   #define IDC_CONSOLE_CG_NAME                        7200
//   #define IDC_CONSOLE_CG_PARENT                      7201
//   #define IDC_CONSOLE_CG_COMMENT                     7202
//   #define IDC_CONSOLE_CG_LABEL_NAME                  7203
//   #define IDC_CONSOLE_CG_LABEL_PARENT                7204
//   #define IDC_CONSOLE_CG_LABEL_COMMENT               7205
//
//   // Default desktop-session configuration (the "Default Configuration" tab
//   // of the original .ui, flattened onto the same dialog template).
//   #define IDC_CONSOLE_CG_INHERIT_CONFIG              7210
//   #define IDC_CONSOLE_CG_GB_FEATURES                 7211
//   #define IDC_CONSOLE_CG_AUDIO                       7212
//   #define IDC_CONSOLE_CG_CLIPBOARD                   7213
//   #define IDC_CONSOLE_CG_CURSOR_SHAPE                7214
//   #define IDC_CONSOLE_CG_CURSOR_POSITION             7215
//   #define IDC_CONSOLE_CG_GB_APPEARANCE               7216
//   #define IDC_CONSOLE_CG_DISABLE_EFFECTS             7217
//   #define IDC_CONSOLE_CG_DISABLE_WALLPAPER           7218
//   #define IDC_CONSOLE_CG_GB_OTHER                    7219
//   #define IDC_CONSOLE_CG_LOCK_AT_DISCONNECT          7220
//   #define IDC_CONSOLE_CG_BLOCK_REMOTE_INPUT          7221
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_CONSOLE_COMPUTER_GROUP_DIALOG_H
#define CLIENT_WIN32_CONSOLE_COMPUTER_GROUP_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class ConsoleComputerGroupDialog
{
public:
    // The original .ui has two tabs: a "General" tab (name / parent / comment)
    // and a "Default Configuration" tab that embeds the same desktop-session
    // override controls used by computer_group_dialog_desktop.ui. The Result
    // struct below flattens both tabs into a single record, mirroring the
    // flattening style used by ConsoleComputerDialog.
    struct Result
    {
        // --- General tab -----------------------------------------------------
        std::wstring name;
        std::wstring parent_group;   // read-only display in the dialog
        std::wstring comment;

        // --- Default Configuration tab (desktop session override) -----------
        // When true, all fields below are ignored and the group inherits its
        // default desktop-session configuration from its parent.
        bool inherit_config = true;

        // "Features" groupbox.
        bool enable_audio          = true;
        bool enable_clipboard      = true;
        bool enable_cursor_shape   = true;
        bool enable_cursor_position = false;

        // "Appearance" groupbox.
        bool disable_desktop_effects   = false;
        bool disable_desktop_wallpaper = false;

        // "Other" groupbox.
        bool lock_at_disconnect = false;
        bool block_remote_input = false;
    };

    ConsoleComputerGroupDialog(HINSTANCE instance, HWND parent);
    ~ConsoleComputerGroupDialog() = default;

    ConsoleComputerGroupDialog(const ConsoleComputerGroupDialog&) = delete;
    ConsoleComputerGroupDialog& operator=(const ConsoleComputerGroupDialog&) = delete;

    // Pre-populate the dialog fields. Call before exec() to repurpose this
    // dialog as an "Edit group" dialog.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onInheritConfigChanged(HWND hwnd);
    void onOk(HWND hwnd);

    // Enables/disables the per-feature check-boxes based on the state of the
    // "Inherit configuration from parent" check-box.
    void updateConfigEnableState(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_CONSOLE_COMPUTER_GROUP_DIALOG_H
