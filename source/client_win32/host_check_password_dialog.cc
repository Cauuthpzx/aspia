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
// Dialog template (add to the client_win32 .rc file):
//
// IDD_HOST_CHECK_PASSWORD DIALOGEX 0, 0, 220, 100
// STYLE DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION "Password Protection"
// FONT 8, "MS Shell Dlg", 400, 0, 0x1
// BEGIN
//     LTEXT        "The configuration is password protected. Enter the password to view and change the configuration.",
//                  IDC_HOST_CHECK_PASSWORD_PROMPT, 7, 7, 206, 24
//     GROUPBOX     "", IDC_STATIC, 7, 35, 206, 30
//     LTEXT        "Password:", IDC_HOST_CHECK_PASSWORD_LABEL, 14, 46, 40, 10
//     EDITTEXT     IDC_HOST_CHECK_PASSWORD_EDIT, 58, 44, 120, 14,
//                  ES_PASSWORD | ES_AUTOHSCROLL
//     AUTOCHECKBOX "Show", IDC_HOST_CHECK_PASSWORD_SHOW, 182, 46, 28, 10
//     DEFPUSHBUTTON "OK",     IDOK,     108, 78, 50, 14
//     PUSHBUTTON    "Cancel", IDCANCEL, 163, 78, 50, 14
// END
//

#include "client_win32/host_check_password_dialog.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

}  // namespace

HostCheckPasswordDialog::HostCheckPasswordDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool HostCheckPasswordDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_HOST_CHECK_PASSWORD), parent_,
        &HostCheckPasswordDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK HostCheckPasswordDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostCheckPasswordDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<HostCheckPasswordDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<HostCheckPasswordDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR HostCheckPasswordDialog::handleMessage(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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
                    onOk(hwnd);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDC_HOST_CHECK_PASSWORD_SHOW:
                    togglePasswordVisibility(hwnd);
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

void HostCheckPasswordDialog::onInitDialog(HWND hwnd)
{
    SendDlgItemMessageW(hwnd, IDC_HOST_CHECK_PASSWORD_EDIT, EM_LIMITTEXT, 32, 0);
    SetFocus(GetDlgItem(hwnd, IDC_HOST_CHECK_PASSWORD_EDIT));
}

void HostCheckPasswordDialog::onOk(HWND hwnd)
{
    result_.password = readEditText(GetDlgItem(hwnd, IDC_HOST_CHECK_PASSWORD_EDIT));
}

void HostCheckPasswordDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const HWND edit = GetDlgItem(hwnd, IDC_HOST_CHECK_PASSWORD_EDIT);
    SendMessageW(edit, EM_SETPASSWORDCHAR, passwordVisible_ ? 0 : L'*', 0);
    InvalidateRect(edit, nullptr, TRUE);
}

}  // namespace aspia::client_win32
