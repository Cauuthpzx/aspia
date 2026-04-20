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

/*
 * ---------------------------------------------------------------------------
 * Win32 DIALOGEX template for this dialog. The resource-file owner should
 * paste this into client.rc (do NOT edit client.rc from here):
 *
 * IDD_HOST_CHANGE_PASSWORD DIALOGEX 0, 0, 280, 160
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Change Password"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     LTEXT           "Old password:",            -1,                                    7,  10,  90,   9
 *     EDITTEXT        IDC_HOST_CHANGE_PASSWORD_OLD,                                    100,   8, 157,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     PUSHBUTTON      "...",                      IDC_HOST_CHANGE_PASSWORD_SHOW,      260,   7,  13,  14
 *     LTEXT           "New password:",            -1,                                    7,  28,  90,   9
 *     EDITTEXT        IDC_HOST_CHANGE_PASSWORD_NEW,                                    100,  26, 173,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     LTEXT           "New password (repeat):",   -1,                                    7,  46,  90,   9
 *     EDITTEXT        IDC_HOST_CHANGE_PASSWORD_NEW_REPEAT,                             100,  44, 173,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     DEFPUSHBUTTON   "OK",                       IDOK,                                168, 139,  50,  14
 *     PUSHBUTTON      "Cancel",                   IDCANCEL,                            223, 139,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/host_change_password_dialog.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

constexpr int kPasswordMaxLength = 32;

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

HostChangePasswordDialog::HostChangePasswordDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

void HostChangePasswordDialog::setInitial(const Result& initial)
{
    initial_ = initial;
    result_ = initial;
}

bool HostChangePasswordDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_HOST_CHANGE_PASSWORD), parent_,
        &HostChangePasswordDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK HostChangePasswordDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostChangePasswordDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<HostChangePasswordDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<HostChangePasswordDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR HostChangePasswordDialog::handleMessage(
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
                    if (onOk(hwnd))
                        EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDC_HOST_CHANGE_PASSWORD_SHOW:
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

void HostChangePasswordDialog::onInitDialog(HWND hwnd)
{
    SendDlgItemMessageW(hwnd, IDC_HOST_CHANGE_PASSWORD_OLD,
                        EM_LIMITTEXT, kPasswordMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW,
                        EM_LIMITTEXT, kPasswordMaxLength, 0);
    SendDlgItemMessageW(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW_REPEAT,
                        EM_LIMITTEXT, kPasswordMaxLength, 0);

    SetDlgItemTextW(hwnd, IDC_HOST_CHANGE_PASSWORD_OLD,
                    initial_.old_password.c_str());
    SetDlgItemTextW(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW,
                    initial_.new_password.c_str());
    SetDlgItemTextW(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW_REPEAT,
                    initial_.new_password.c_str());

    SetFocus(GetDlgItem(hwnd, IDC_HOST_CHANGE_PASSWORD_OLD));
}

bool HostChangePasswordDialog::onOk(HWND hwnd)
{
    std::wstring old_pass =
        readEditText(GetDlgItem(hwnd, IDC_HOST_CHANGE_PASSWORD_OLD));
    std::wstring new_pass =
        readEditText(GetDlgItem(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW));
    std::wstring new_pass_repeat =
        readEditText(GetDlgItem(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW_REPEAT));

    if (new_pass != new_pass_repeat)
    {
        MessageBoxW(hwnd,
                    L"The entered passwords do not match.",
                    L"Change Password",
                    MB_OK | MB_ICONWARNING);
        SetFocus(GetDlgItem(hwnd, IDC_HOST_CHANGE_PASSWORD_NEW));
        return false;
    }

    result_.old_password = std::move(old_pass);
    result_.new_password = std::move(new_pass);
    return true;
}

void HostChangePasswordDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const wchar_t mask = passwordVisible_ ? 0 : L'*';

    const int ids[] = {
        IDC_HOST_CHANGE_PASSWORD_OLD,
        IDC_HOST_CHANGE_PASSWORD_NEW,
        IDC_HOST_CHANGE_PASSWORD_NEW_REPEAT,
    };

    for (int id : ids)
    {
        const HWND edit = GetDlgItem(hwnd, id);
        SendMessageW(edit, EM_SETPASSWORDCHAR, mask, 0);
        InvalidateRect(edit, nullptr, TRUE);
    }
}

}  // namespace aspia::client_win32
