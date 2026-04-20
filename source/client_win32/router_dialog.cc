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
 * IDD_ROUTER DIALOGEX 0, 0, 280, 220
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Router"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     GROUPBOX        "",                    -1,                           7,   4, 266, 180
 *     LTEXT           "Address:",            -1,                          14,  16,  80,   9
 *     EDITTEXT        IDC_ROUTER_ADDRESS,                                 14,  28, 252,  12, ES_AUTOHSCROLL
 *     LTEXT           "Name:",               -1,                          14,  44,  80,   9
 *     EDITTEXT        IDC_ROUTER_NAME,                                    14,  56, 252,  12, ES_AUTOHSCROLL
 *     LTEXT           "Access Level:",       -1,                          14,  72,  80,   9
 *     COMBOBOX        IDC_ROUTER_SESSION_TYPE,                            14,  84, 252,  60,
 *                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "User Name:",          -1,                          14, 104,  80,   9
 *     EDITTEXT        IDC_ROUTER_USERNAME,                                14, 116, 252,  12, ES_AUTOHSCROLL
 *     LTEXT           "Password:",           -1,                          14, 132,  80,   9
 *     EDITTEXT        IDC_ROUTER_PASSWORD,                                14, 144, 236,  12,
 *                     ES_AUTOHSCROLL | ES_PASSWORD
 *     PUSHBUTTON      "...",                 IDC_ROUTER_SHOW_PASSWORD,   253, 143,  13,  14
 *     DEFPUSHBUTTON   "OK",                  IDOK,                       168, 194,  50,  14
 *     PUSHBUTTON      "Cancel",              IDCANCEL,                   223, 194,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/router_dialog.h"

#include "client_win32/resource.h"

#include <cwchar>

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

RouterDialog::RouterDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool RouterDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_ROUTER), parent_,
        &RouterDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK RouterDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    RouterDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<RouterDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<RouterDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR RouterDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

                case IDC_ROUTER_SHOW_PASSWORD:
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

void RouterDialog::onInitDialog(HWND hwnd)
{
    // .ui declares maxLength=64 on every QLineEdit. Mirror that exactly.
    SendDlgItemMessageW(hwnd, IDC_ROUTER_ADDRESS,  EM_LIMITTEXT, 64, 0);
    SendDlgItemMessageW(hwnd, IDC_ROUTER_NAME,     EM_LIMITTEXT, 64, 0);
    SendDlgItemMessageW(hwnd, IDC_ROUTER_USERNAME, EM_LIMITTEXT, 64, 0);
    SendDlgItemMessageW(hwnd, IDC_ROUTER_PASSWORD, EM_LIMITTEXT, 64, 0);

    SetFocus(GetDlgItem(hwnd, IDC_ROUTER_ADDRESS));
}

void RouterDialog::onOk(HWND hwnd)
{
    const std::wstring address_raw =
        readEditText(GetDlgItem(hwnd, IDC_ROUTER_ADDRESS));
    splitAddress(address_raw, &result_.address, &result_.port);

    result_.name     = readEditText(GetDlgItem(hwnd, IDC_ROUTER_NAME));
    result_.username = readEditText(GetDlgItem(hwnd, IDC_ROUTER_USERNAME));
    result_.password = readEditText(GetDlgItem(hwnd, IDC_ROUTER_PASSWORD));

    // Pull the currently selected session-type string out of the combobox.
    const HWND combo = GetDlgItem(hwnd, IDC_ROUTER_SESSION_TYPE);
    const LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel != CB_ERR)
    {
        const LRESULT len = SendMessageW(combo, CB_GETLBTEXTLEN, static_cast<WPARAM>(sel), 0);
        if (len > 0)
        {
            std::wstring out(static_cast<size_t>(len), L'\0');
            SendMessageW(combo, CB_GETLBTEXT, static_cast<WPARAM>(sel),
                         reinterpret_cast<LPARAM>(out.data()));
            result_.sessionType = std::move(out);
        }
    }
}

void RouterDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const HWND edit = GetDlgItem(hwnd, IDC_ROUTER_PASSWORD);
    SendMessageW(edit, EM_SETPASSWORDCHAR, passwordVisible_ ? 0 : L'*', 0);
    InvalidateRect(edit, nullptr, TRUE);
}

}  // namespace aspia::client_win32
