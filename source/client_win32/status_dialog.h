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
// Win32 replacement for common/ui/status_dialog.ui.
//
// resource.h additions:
//   #define IDD_STATUS           220
//   #define IDC_STATUS_MESSAGE   2200
//

#ifndef CLIENT_WIN32_STATUS_DIALOG_H
#define CLIENT_WIN32_STATUS_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class StatusDialog
{
public:
    StatusDialog(HINSTANCE instance, HWND parent);
    ~StatusDialog() = default;

    StatusDialog(const StatusDialog&) = delete;
    StatusDialog& operator=(const StatusDialog&) = delete;

    void exec(const std::wstring& title, const std::wstring& message);

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    std::wstring title_;
    std::wstring message_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_STATUS_DIALOG_H
