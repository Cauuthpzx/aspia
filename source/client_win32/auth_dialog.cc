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

#include "client_win32/auth_dialog.h"

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

AuthDialog::AuthDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool AuthDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_AUTH), parent_,
        &AuthDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK AuthDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    AuthDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<AuthDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<AuthDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR AuthDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

                case IDC_AUTH_SHOW_PASSWORD:
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

void AuthDialog::onInitDialog(HWND hwnd)
{
    SendDlgItemMessageW(hwnd, IDC_AUTH_USERNAME, EM_LIMITTEXT, 128, 0);
    SendDlgItemMessageW(hwnd, IDC_AUTH_PASSWORD, EM_LIMITTEXT, 128, 0);
    SetFocus(GetDlgItem(hwnd, IDC_AUTH_USERNAME));
}

void AuthDialog::onOk(HWND hwnd)
{
    result_.username = readEditText(GetDlgItem(hwnd, IDC_AUTH_USERNAME));
    result_.password = readEditText(GetDlgItem(hwnd, IDC_AUTH_PASSWORD));
    result_.oneTimePassword =
        (IsDlgButtonChecked(hwnd, IDC_AUTH_ONE_TIME) == BST_CHECKED);
}

void AuthDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const HWND edit = GetDlgItem(hwnd, IDC_AUTH_PASSWORD);
    SendMessageW(edit, EM_SETPASSWORDCHAR, passwordVisible_ ? 0 : L'*', 0);
    InvalidateRect(edit, nullptr, TRUE);
}

}  // namespace aspia::client_win32
