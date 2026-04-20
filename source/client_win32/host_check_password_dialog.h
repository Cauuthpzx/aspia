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
// Win32 replacement for host/ui/check_password_dialog.ui.
//
// Resource IDs (defined in client_win32/resource.h):
// #define IDD_HOST_CHECK_PASSWORD            370
// #define IDC_HOST_CHECK_PASSWORD_EDIT       3700
// #define IDC_HOST_CHECK_PASSWORD_SHOW       3701
// #define IDC_HOST_CHECK_PASSWORD_PROMPT     3702
// #define IDC_HOST_CHECK_PASSWORD_LABEL      3703
//

#ifndef CLIENT_WIN32_HOST_CHECK_PASSWORD_DIALOG_H
#define CLIENT_WIN32_HOST_CHECK_PASSWORD_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class HostCheckPasswordDialog
{
public:
    struct Result
    {
        std::wstring password;
    };

    HostCheckPasswordDialog(HINSTANCE instance, HWND parent);
    ~HostCheckPasswordDialog() = default;

    HostCheckPasswordDialog(const HostCheckPasswordDialog&) = delete;
    HostCheckPasswordDialog& operator=(const HostCheckPasswordDialog&) = delete;

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
    Result result_;
    bool passwordVisible_ = false;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_HOST_CHECK_PASSWORD_DIALOG_H
