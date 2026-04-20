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
// Win32 replacement for client/ui/router_manager/router_user_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_ROUTER_USER                           340
//   #define IDC_ROUTER_USER_USERNAME                 3400
//   #define IDC_ROUTER_USER_PASSWORD                 3401
//   #define IDC_ROUTER_USER_PASSWORD_RETRY           3402
//   #define IDC_ROUTER_USER_SHOW_PASSWORD            3403
//   #define IDC_ROUTER_USER_DISABLE                  3404
//   #define IDC_ROUTER_USER_SESSIONS_GROUP           3405
//   #define IDC_ROUTER_USER_SESSION_DESKTOP_MANAGE   3410
//   #define IDC_ROUTER_USER_SESSION_DESKTOP_VIEW     3411
//   #define IDC_ROUTER_USER_SESSION_FILE_TRANSFER    3412
//   #define IDC_ROUTER_USER_SESSION_SYSTEM_INFO      3413
//   #define IDC_ROUTER_USER_SESSION_TEXT_CHAT        3414
//   #define IDC_ROUTER_USER_SESSION_PORT_FORWARDING  3415
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_ROUTER_USER_DIALOG_H
#define CLIENT_WIN32_ROUTER_USER_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class RouterUserDialog
{
public:
    struct Result
    {
        std::wstring username;
        std::wstring password;
        bool active = true;
        // Bitmask of session-type flags (e.g. SessionTypeDesktopManage |
        // SessionTypeFileTransfer). The value is passed through as-is; the
        // caller maps between proto enums and individual checkbox controls.
        int sessions = 0;
    };

    RouterUserDialog(HINSTANCE instance, HWND parent);
    ~RouterUserDialog() = default;

    RouterUserDialog(const RouterUserDialog&) = delete;
    RouterUserDialog& operator=(const RouterUserDialog&) = delete;

    void setInitial(const Result& initial);

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);
    void togglePasswordVisibility(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result initial_;
    Result result_;
    bool passwordVisible_ = false;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_ROUTER_USER_DIALOG_H
