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
// Win32 replacement for client/ui/router_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_ROUTER                   240
//   #define IDC_ROUTER_ADDRESS          2401
//   #define IDC_ROUTER_NAME             2402
//   #define IDC_ROUTER_SESSION_TYPE     2403
//   #define IDC_ROUTER_USERNAME         2404
//   #define IDC_ROUTER_PASSWORD         2405
//   #define IDC_ROUTER_SHOW_PASSWORD    2406
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_ROUTER_DIALOG_H
#define CLIENT_WIN32_ROUTER_DIALOG_H

#include <windows.h>

#include <cstdint>
#include <string>

namespace aspia::client_win32 {

class RouterDialog
{
public:
    struct Result
    {
        std::wstring address;
        uint16_t port = 0;
        std::wstring name;
        std::wstring sessionType;
        std::wstring username;
        std::wstring password;
    };

    RouterDialog(HINSTANCE instance, HWND parent);
    ~RouterDialog() = default;

    RouterDialog(const RouterDialog&) = delete;
    RouterDialog& operator=(const RouterDialog&) = delete;

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

#endif  // CLIENT_WIN32_ROUTER_DIALOG_H
