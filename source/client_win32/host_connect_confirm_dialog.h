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
// Win32 replacement for host/ui/connect_confirm_dialog.ui.
//
// resource.h additions:
//   #define IDD_HOST_CONNECT_CONFIRM             380
//   #define IDC_HOST_CONFIRM_CLIENT_ADDRESS      3800
//   #define IDC_HOST_CONFIRM_USER_AGENT          3801
//   #define IDC_HOST_CONFIRM_SESSION_TYPE        3802
//   #define IDC_HOST_CONFIRM_REMEMBER            3803
//

#ifndef CLIENT_WIN32_HOST_CONNECT_CONFIRM_DIALOG_H
#define CLIENT_WIN32_HOST_CONNECT_CONFIRM_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class HostConnectConfirmDialog
{
public:
    struct Config
    {
        std::wstring client_address;
        std::wstring user_agent;
        std::wstring session_type;
    };

    HostConnectConfirmDialog(HINSTANCE instance, HWND parent);
    ~HostConnectConfirmDialog() = default;

    HostConnectConfirmDialog(const HostConnectConfirmDialog&) = delete;
    HostConnectConfirmDialog& operator=(const HostConnectConfirmDialog&) = delete;

    void setRequest(Config config);
    bool exec();
    bool rememberChoice() const { return remember_choice_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onClose(HWND hwnd, int code);

    HINSTANCE instance_;
    HWND parent_;
    Config config_;
    bool remember_choice_ = false;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_HOST_CONNECT_CONFIRM_DIALOG_H
