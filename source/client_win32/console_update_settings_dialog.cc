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
 * ------------------------------------------------------------------------
 * Win32 DIALOGEX template for this dialog. Copy this verbatim into
 * client.rc (it is NOT in the build yet; we deliberately avoid editing
 * client.rc from inside this port). The control IDs match the #defines
 * listed at the top of console_update_settings_dialog.h.
 *
 * The .ui layout uses a single unnamed GroupBox containing the two
 * checkboxes and a server row. We mirror that, and add a "Check now"
 * button aligned with the "Check for updates on startup" checkbox, as
 * is done in the companion settings_dialog port.
 *
 * IDD_CONSOLE_UPDATE_SETTINGS DIALOGEX 0, 0, 280, 160
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Update Settings"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     GROUPBOX        "",                       -1,
 *                     7,   4, 266, 126
 *     AUTOCHECKBOX    "Check for updates on startup",
 *                                               IDC_CONSOLE_UPDATE_CHECK_ON_STARTUP,
 *                     14,  16, 160,  10
 *     PUSHBUTTON      "Check for updates",      IDC_CONSOLE_UPDATE_CHECK_NOW,
 *                     180, 14,  86,  14
 *     AUTOCHECKBOX    "Use custom update server",
 *                                               IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER,
 *                     14,  34, 252,  10
 *     LTEXT           "Server:",                IDC_CONSOLE_UPDATE_SERVER_LABEL,
 *                     14,  52,  30,   9
 *     EDITTEXT        IDC_CONSOLE_UPDATE_SERVER,
 *                     50,  50, 216,  12, ES_AUTOHSCROLL
 *
 *     DEFPUSHBUTTON   "OK",                     IDOK,
 *                     166, 140,  50,  14
 *     PUSHBUTTON      "Cancel",                 IDCANCEL,
 *                     223, 140,  50,  14
 * END
 * ------------------------------------------------------------------------
 */

#include "client_win32/console_update_settings_dialog.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

constexpr int kUpdateServerMaxLength = 255;

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

ConsoleUpdateSettingsDialog::ConsoleUpdateSettingsDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool ConsoleUpdateSettingsDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_CONSOLE_UPDATE_SETTINGS), parent_,
        &ConsoleUpdateSettingsDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK ConsoleUpdateSettingsDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleUpdateSettingsDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<ConsoleUpdateSettingsDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<ConsoleUpdateSettingsDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR ConsoleUpdateSettingsDialog::handleMessage(
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

                case IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onCustomServerToggled(hwnd);
                        return TRUE;
                    }
                    break;

                case IDC_CONSOLE_UPDATE_CHECK_NOW:
                    if (HIWORD(wp) == BN_CLICKED)
                    {
                        onCheckForUpdatesNow(hwnd);
                        return TRUE;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

void ConsoleUpdateSettingsDialog::onInitDialog(HWND hwnd)
{
    CheckDlgButton(hwnd, IDC_CONSOLE_UPDATE_CHECK_ON_STARTUP,
                   result_.check_on_startup ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER,
                   result_.use_custom_server ? BST_CHECKED : BST_UNCHECKED);

    SendDlgItemMessageW(hwnd, IDC_CONSOLE_UPDATE_SERVER, EM_LIMITTEXT,
                        kUpdateServerMaxLength, 0);
    SetDlgItemTextW(hwnd, IDC_CONSOLE_UPDATE_SERVER,
                    result_.update_server.c_str());

    updateServerEnable(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_CONSOLE_UPDATE_CHECK_ON_STARTUP));
}

void ConsoleUpdateSettingsDialog::onOk(HWND hwnd)
{
    result_.check_on_startup =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_UPDATE_CHECK_ON_STARTUP) == BST_CHECKED;
    result_.use_custom_server =
        IsDlgButtonChecked(hwnd, IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER) == BST_CHECKED;
    result_.update_server =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_UPDATE_SERVER));
}

void ConsoleUpdateSettingsDialog::onCustomServerToggled(HWND hwnd)
{
    updateServerEnable(hwnd);
}

void ConsoleUpdateSettingsDialog::onCheckForUpdatesNow(HWND /*hwnd*/)
{
    // The actual update check is fired from the enclosing application
    // once the dialog returns IDOK; this stub exists so the button has a
    // handler and does not bubble as an accidental IDOK via accelerator
    // processing. No-op here.
}

void ConsoleUpdateSettingsDialog::updateServerEnable(HWND hwnd)
{
    const BOOL enabled =
        (IsDlgButtonChecked(hwnd, IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER)
             == BST_CHECKED) ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_UPDATE_SERVER_LABEL), enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_UPDATE_SERVER), enabled);
}

}  // namespace aspia::client_win32
