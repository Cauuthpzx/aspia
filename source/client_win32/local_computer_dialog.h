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
// Win32 replacement for client/ui/hosts/local_computer_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_LOCAL_COMPUTER                 330
//   #define IDC_LOCAL_COMPUTER_GROUP          3301
//   #define IDC_LOCAL_COMPUTER_NAME           3302
//   #define IDC_LOCAL_COMPUTER_ADDRESS        3303
//   #define IDC_LOCAL_COMPUTER_USERNAME       3304
//   #define IDC_LOCAL_COMPUTER_PASSWORD       3305
//   #define IDC_LOCAL_COMPUTER_SHOW_PASSWORD  3306
//   #define IDC_LOCAL_COMPUTER_COMMENT        3307
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_LOCAL_COMPUTER_DIALOG_H
#define CLIENT_WIN32_LOCAL_COMPUTER_DIALOG_H

#include <windows.h>

#include <cstdint>
#include <string>

namespace aspia::client_win32 {

class LocalComputerDialog
{
public:
    struct Result
    {
        std::wstring group;
        std::wstring name;
        std::wstring address;
        uint16_t port = 0;
        std::wstring username;
        std::wstring password;
        std::wstring comment;
    };

    LocalComputerDialog(HINSTANCE instance, HWND parent);
    ~LocalComputerDialog() = default;

    LocalComputerDialog(const LocalComputerDialog&) = delete;
    LocalComputerDialog& operator=(const LocalComputerDialog&) = delete;

    // Pre-populates the dialog fields. Call before exec() to repurpose this
    // dialog as an "Edit computer" dialog.
    void setInitial(const Result& initial) { result_ = initial; }

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

#endif  // CLIENT_WIN32_LOCAL_COMPUTER_DIALOG_H
