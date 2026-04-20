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
// Win32 replacement for client/ui/authorization_dialog.ui.
//

#ifndef CLIENT_WIN32_AUTH_DIALOG_H
#define CLIENT_WIN32_AUTH_DIALOG_H

#include <windows.h>

#include <cstdint>
#include <string>

namespace aspia::client_win32 {

class AuthDialog
{
public:
    struct Result
    {
        std::wstring address;
        uint16_t     port = 8050;
        std::wstring username;
        std::wstring password;
        bool         oneTimePassword = false;
    };

    AuthDialog(HINSTANCE instance, HWND parent);
    ~AuthDialog() = default;

    AuthDialog(const AuthDialog&) = delete;
    AuthDialog& operator=(const AuthDialog&) = delete;

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

#endif  // CLIENT_WIN32_AUTH_DIALOG_H
