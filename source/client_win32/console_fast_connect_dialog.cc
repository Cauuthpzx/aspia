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
 * IDD_CONSOLE_FAST_CONNECT DIALOGEX 0, 0, 260, 120
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Fast Connect"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     GROUPBOX        "",                                 -1,                                    7,   4, 246,  86
 *     LTEXT           "Address / ID",                     -1,                                   14,  16, 100,   9
 *     COMBOBOX        IDC_CONSOLE_FAST_CONNECT_ADDRESS,                                         14,  28, 232,  60,
 *                     CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Session Type",                     -1,                                   14,  48, 100,   9
 *     COMBOBOX        IDC_CONSOLE_FAST_CONNECT_SESSION_TYPE,                                    14,  60, 232,  60,
 *                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
 *     DEFPUSHBUTTON   "OK",                               IDOK,                                148,  98,  50,  14
 *     PUSHBUTTON      "Cancel",                           IDCANCEL,                            203,  98,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/console_fast_connect_dialog.h"

#include "client_win32/resource.h"

#include <cwchar>

namespace aspia::client_win32 {

namespace {

std::wstring readComboText(HWND combo)
{
    // Editable combo: text reflects the edit control portion.
    const int len = GetWindowTextLengthW(combo);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(combo, out.data(), len + 1);
    return out;
}

std::wstring readComboSelection(HWND combo)
{
    const LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR)
        return {};

    const LRESULT len = SendMessageW(combo, CB_GETLBTEXTLEN, static_cast<WPARAM>(sel), 0);
    if (len <= 0)
        return {};

    std::wstring out(static_cast<size_t>(len), L'\0');
    SendMessageW(combo, CB_GETLBTEXT, static_cast<WPARAM>(sel),
                 reinterpret_cast<LPARAM>(out.data()));
    return out;
}

// Splits "host:port" into a host string and a port. If no ':' is present the
// whole string is treated as the host and 0 is returned for the port.
void splitAddress(const std::wstring& full, std::wstring* host, uint16_t* port)
{
    const size_t colon = full.find_last_of(L':');
    if (colon == std::wstring::npos)
    {
        *host = full;
        *port = 0;
        return;
    }

    *host = full.substr(0, colon);

    const wchar_t* begin = full.c_str() + colon + 1;
    wchar_t* end = nullptr;
    const unsigned long value = std::wcstoul(begin, &end, 10);
    if (end == begin || value > 0xFFFFu)
        *port = 0;
    else
        *port = static_cast<uint16_t>(value);
}

}  // namespace

ConsoleFastConnectDialog::ConsoleFastConnectDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool ConsoleFastConnectDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_CONSOLE_FAST_CONNECT), parent_,
        &ConsoleFastConnectDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK ConsoleFastConnectDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleFastConnectDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<ConsoleFastConnectDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<ConsoleFastConnectDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR ConsoleFastConnectDialog::handleMessage(
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

                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

void ConsoleFastConnectDialog::onInitDialog(HWND hwnd)
{
    SetFocus(GetDlgItem(hwnd, IDC_CONSOLE_FAST_CONNECT_ADDRESS));
}

void ConsoleFastConnectDialog::onOk(HWND hwnd)
{
    const std::wstring address_raw =
        readComboText(GetDlgItem(hwnd, IDC_CONSOLE_FAST_CONNECT_ADDRESS));
    splitAddress(address_raw, &result_.address, &result_.port);

    result_.session_type =
        readComboSelection(GetDlgItem(hwnd, IDC_CONSOLE_FAST_CONNECT_SESSION_TYPE));
}

}  // namespace aspia::client_win32
