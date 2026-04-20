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
 * IDD_HOST_USER DIALOGEX 0, 0, 280, 240
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "User Properties"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     LTEXT           "User Name:",             -1,                                      7,  10,  70,   9
 *     EDITTEXT        IDC_HOST_USER_USERNAME,                                           80,   8, 193,  12, ES_AUTOHSCROLL
 *     LTEXT           "Password:",              -1,                                      7,  26,  70,   9
 *     EDITTEXT        IDC_HOST_USER_PASSWORD,                                           80,  24, 177,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     PUSHBUTTON      "...",                    IDC_HOST_USER_SHOW_PASSWORD,           260,  23,  13,  14
 *     LTEXT           "Password (repeat):",     -1,                                      7,  42,  70,   9
 *     EDITTEXT        IDC_HOST_USER_PASSWORD_RETRY,                                     80,  40, 193,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     AUTOCHECKBOX    "Disable User Account",   IDC_HOST_USER_DISABLE,                   7,  60, 266,  10
 *     GROUPBOX        "Allowed Session Types:", IDC_HOST_USER_SESSIONS_GROUP,            7,  75, 266, 140
 *     AUTOCHECKBOX    "Manage Desktop",         IDC_HOST_USER_SESSION_DESKTOP_MANAGE,   14,  88, 252,  10
 *     AUTOCHECKBOX    "View Desktop",           IDC_HOST_USER_SESSION_DESKTOP_VIEW,     14, 102, 252,  10
 *     AUTOCHECKBOX    "File Transfer",          IDC_HOST_USER_SESSION_FILE_TRANSFER,    14, 116, 252,  10
 *     AUTOCHECKBOX    "System Information",     IDC_HOST_USER_SESSION_SYSTEM_INFO,      14, 130, 252,  10
 *     AUTOCHECKBOX    "Text Chat",              IDC_HOST_USER_SESSION_TEXT_CHAT,        14, 144, 252,  10
 *     AUTOCHECKBOX    "Port Forwarding",        IDC_HOST_USER_SESSION_PORT_FORWARDING,  14, 158, 252,  10
 *     DEFPUSHBUTTON   "OK",                     IDOK,                                  168, 220,  50,  14
 *     PUSHBUTTON      "Cancel",                 IDCANCEL,                              223, 220,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/host_user_dialog.h"

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

// Session-type bit layout. These mirror the proto::SessionType flag values
// used by the host admin protocol; keep the ordering stable.
constexpr int kSessionDesktopManage   = 1 << 0;
constexpr int kSessionDesktopView     = 1 << 1;
constexpr int kSessionFileTransfer    = 1 << 2;
constexpr int kSessionSystemInfo      = 1 << 3;
constexpr int kSessionTextChat        = 1 << 4;
constexpr int kSessionPortForwarding  = 1 << 5;

struct SessionBinding
{
    int control_id;
    int flag;
};

constexpr SessionBinding kSessionBindings[] =
{
    { IDC_HOST_USER_SESSION_DESKTOP_MANAGE,  kSessionDesktopManage  },
    { IDC_HOST_USER_SESSION_DESKTOP_VIEW,    kSessionDesktopView    },
    { IDC_HOST_USER_SESSION_FILE_TRANSFER,   kSessionFileTransfer   },
    { IDC_HOST_USER_SESSION_SYSTEM_INFO,     kSessionSystemInfo     },
    { IDC_HOST_USER_SESSION_TEXT_CHAT,       kSessionTextChat       },
    { IDC_HOST_USER_SESSION_PORT_FORWARDING, kSessionPortForwarding },
};

}  // namespace

HostUserDialog::HostUserDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

void HostUserDialog::setInitial(const Result& initial)
{
    initial_ = initial;
    result_ = initial;
}

bool HostUserDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_HOST_USER), parent_,
        &HostUserDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK HostUserDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostUserDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<HostUserDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<HostUserDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR HostUserDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

                case IDC_HOST_USER_SHOW_PASSWORD:
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

void HostUserDialog::onInitDialog(HWND hwnd)
{
    SendDlgItemMessageW(hwnd, IDC_HOST_USER_USERNAME,       EM_LIMITTEXT, 64, 0);
    SendDlgItemMessageW(hwnd, IDC_HOST_USER_PASSWORD,       EM_LIMITTEXT, 64, 0);
    SendDlgItemMessageW(hwnd, IDC_HOST_USER_PASSWORD_RETRY, EM_LIMITTEXT, 64, 0);

    SetDlgItemTextW(hwnd, IDC_HOST_USER_USERNAME,       initial_.username.c_str());
    SetDlgItemTextW(hwnd, IDC_HOST_USER_PASSWORD,       initial_.password.c_str());
    SetDlgItemTextW(hwnd, IDC_HOST_USER_PASSWORD_RETRY,
                   initial_.password_retry.empty()
                       ? initial_.password.c_str()
                       : initial_.password_retry.c_str());

    CheckDlgButton(hwnd, IDC_HOST_USER_DISABLE,
                   initial_.disabled ? BST_CHECKED : BST_UNCHECKED);

    for (const SessionBinding& b : kSessionBindings)
    {
        CheckDlgButton(hwnd, b.control_id,
                       (initial_.sessions & b.flag) ? BST_CHECKED : BST_UNCHECKED);
    }

    SetFocus(GetDlgItem(hwnd, IDC_HOST_USER_USERNAME));
}

void HostUserDialog::onOk(HWND hwnd)
{
    result_.username       = readEditText(GetDlgItem(hwnd, IDC_HOST_USER_USERNAME));
    result_.password       = readEditText(GetDlgItem(hwnd, IDC_HOST_USER_PASSWORD));
    result_.password_retry = readEditText(GetDlgItem(hwnd, IDC_HOST_USER_PASSWORD_RETRY));
    result_.disabled =
        (IsDlgButtonChecked(hwnd, IDC_HOST_USER_DISABLE) == BST_CHECKED);

    int sessions = 0;
    for (const SessionBinding& b : kSessionBindings)
    {
        if (IsDlgButtonChecked(hwnd, b.control_id) == BST_CHECKED)
            sessions |= b.flag;
    }
    result_.sessions = sessions;
}

void HostUserDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const wchar_t mask = passwordVisible_ ? 0 : L'*';

    const HWND pwd = GetDlgItem(hwnd, IDC_HOST_USER_PASSWORD);
    SendMessageW(pwd, EM_SETPASSWORDCHAR, mask, 0);
    InvalidateRect(pwd, nullptr, TRUE);

    const HWND pwd_retry = GetDlgItem(hwnd, IDC_HOST_USER_PASSWORD_RETRY);
    SendMessageW(pwd_retry, EM_SETPASSWORDCHAR, mask, 0);
    InvalidateRect(pwd_retry, nullptr, TRUE);
}

}  // namespace aspia::client_win32
