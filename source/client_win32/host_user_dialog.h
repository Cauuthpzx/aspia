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
// Win32 replacement for host/ui/user_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_HOST_USER                           390
//   #define IDC_HOST_USER_USERNAME                 3900
//   #define IDC_HOST_USER_PASSWORD                 3901
//   #define IDC_HOST_USER_PASSWORD_RETRY           3902
//   #define IDC_HOST_USER_SHOW_PASSWORD            3903
//   #define IDC_HOST_USER_DISABLE                  3904
//   #define IDC_HOST_USER_SESSIONS_GROUP           3905
//   #define IDC_HOST_USER_SESSION_DESKTOP_MANAGE   3910
//   #define IDC_HOST_USER_SESSION_DESKTOP_VIEW     3911
//   #define IDC_HOST_USER_SESSION_FILE_TRANSFER    3912
//   #define IDC_HOST_USER_SESSION_SYSTEM_INFO      3913
//   #define IDC_HOST_USER_SESSION_TEXT_CHAT        3914
//   #define IDC_HOST_USER_SESSION_PORT_FORWARDING  3915
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_HOST_USER_DIALOG_H
#define CLIENT_WIN32_HOST_USER_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class HostUserDialog
{
public:
    struct Result
    {
        std::wstring username;
        std::wstring password;
        std::wstring password_retry;
        bool disabled = false;
        // Bitmask of session-type flags (e.g. SessionTypeDesktopManage |
        // SessionTypeFileTransfer). The value is passed through as-is; the
        // caller maps between proto enums and individual checkbox controls.
        int sessions = 0;
    };

    HostUserDialog(HINSTANCE instance, HWND parent);
    ~HostUserDialog() = default;

    HostUserDialog(const HostUserDialog&) = delete;
    HostUserDialog& operator=(const HostUserDialog&) = delete;

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

#endif  // CLIENT_WIN32_HOST_USER_DIALOG_H
