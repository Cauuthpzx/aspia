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
// IDD_HOST_CONNECT_CONFIRM DIALOGEX 0, 0, 280, 180
// STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION "Confirmation of connection"
// FONT 9, "Segoe UI", 400, 0, 0x1
// BEGIN
//     LTEXT           "A remote client is requesting a connection to this computer.",
//                     IDC_STATIC, 10, 10, 260, 20
//     LTEXT           "Client address:", IDC_STATIC, 10, 40, 70, 10
//     LTEXT           "", IDC_HOST_CONFIRM_CLIENT_ADDRESS, 85, 40, 185, 10
//     LTEXT           "User agent:", IDC_STATIC, 10, 55, 70, 10
//     LTEXT           "", IDC_HOST_CONFIRM_USER_AGENT, 85, 55, 185, 10
//     LTEXT           "Session type:", IDC_STATIC, 10, 70, 70, 10
//     LTEXT           "", IDC_HOST_CONFIRM_SESSION_TYPE, 85, 70, 185, 10
//     LTEXT           "Allow this connection?", IDC_STATIC, 10, 95, 260, 10
//     AUTOCHECKBOX    "Remember my choice", IDC_HOST_CONFIRM_REMEMBER,
//                     10, 115, 260, 12
//     DEFPUSHBUTTON   "Accept", IDOK, 80, 155, 60, 14
//     PUSHBUTTON      "Reject", IDCANCEL, 145, 155, 60, 14
// END
//

#include "client_win32/host_connect_confirm_dialog.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

HostConnectConfirmDialog::HostConnectConfirmDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

void HostConnectConfirmDialog::setRequest(Config config)
{
    config_ = std::move(config);
}

bool HostConnectConfirmDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_HOST_CONNECT_CONFIRM), parent_,
        &HostConnectConfirmDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK HostConnectConfirmDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostConnectConfirmDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<HostConnectConfirmDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<HostConnectConfirmDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR HostConnectConfirmDialog::handleMessage(
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
                    onClose(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    onClose(hwnd, IDCANCEL);
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

void HostConnectConfirmDialog::onInitDialog(HWND hwnd)
{
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIRM_CLIENT_ADDRESS, config_.client_address.c_str());
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIRM_USER_AGENT, config_.user_agent.c_str());
    SetDlgItemTextW(hwnd, IDC_HOST_CONFIRM_SESSION_TYPE, config_.session_type.c_str());
    CheckDlgButton(hwnd, IDC_HOST_CONFIRM_REMEMBER, BST_UNCHECKED);
    SetFocus(GetDlgItem(hwnd, IDCANCEL));
}

void HostConnectConfirmDialog::onClose(HWND hwnd, int code)
{
    remember_choice_ =
        (IsDlgButtonChecked(hwnd, IDC_HOST_CONFIRM_REMEMBER) == BST_CHECKED);
    EndDialog(hwnd, code);
}

}  // namespace aspia::client_win32
