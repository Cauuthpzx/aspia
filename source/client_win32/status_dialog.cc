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
// Append the following DIALOGEX template to client_win32/client.rc:
//
// IDD_STATUS DIALOGEX 0, 0, 240, 80
// STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION ""
// FONT 9, "Segoe UI", 400, 0, 0x1
// BEGIN
//     LTEXT           "", IDC_STATUS_MESSAGE, 10, 10, 220, 40
//     DEFPUSHBUTTON   "OK", IDOK, 90, 58, 60, 14
// END
//

#include "client_win32/status_dialog.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

StatusDialog::StatusDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

void StatusDialog::exec(const std::wstring& title, const std::wstring& message)
{
    title_ = title;
    message_ = message;

    DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_STATUS), parent_,
        &StatusDialog::dialogProc, reinterpret_cast<LPARAM>(this));
}

// static
INT_PTR CALLBACK StatusDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    StatusDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<StatusDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<StatusDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR StatusDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            onInitDialog(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wp))
            {
                case IDOK:
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

void StatusDialog::onInitDialog(HWND hwnd)
{
    SetWindowTextW(hwnd, title_.c_str());
    SetDlgItemTextW(hwnd, IDC_STATUS_MESSAGE, message_.c_str());
    SetFocus(GetDlgItem(hwnd, IDOK));
}

}  // namespace aspia::client_win32
