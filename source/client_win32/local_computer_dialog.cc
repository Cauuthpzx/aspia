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
 * IDD_LOCAL_COMPUTER DIALOGEX 0, 0, 300, 200
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Computer"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     LTEXT           "Group:",      -1,                                 7,   9,  60,   9
 *     COMBOBOX        IDC_LOCAL_COMPUTER_GROUP,                         70,   7, 223,  60,
 *                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Name:",       -1,                                 7,  25,  60,   9
 *     EDITTEXT        IDC_LOCAL_COMPUTER_NAME,                          70,  23, 223,  12, ES_AUTOHSCROLL
 *     LTEXT           "Address:",    -1,                                 7,  41,  60,   9
 *     EDITTEXT        IDC_LOCAL_COMPUTER_ADDRESS,                       70,  39, 223,  12, ES_AUTOHSCROLL
 *     LTEXT           "Username:",   -1,                                 7,  57,  60,   9
 *     EDITTEXT        IDC_LOCAL_COMPUTER_USERNAME,                      70,  55, 223,  12, ES_AUTOHSCROLL
 *     LTEXT           "Password:",   -1,                                 7,  73,  60,   9
 *     EDITTEXT        IDC_LOCAL_COMPUTER_PASSWORD,                      70,  71, 207,  12,
 *                     ES_AUTOHSCROLL | ES_PASSWORD
 *     PUSHBUTTON      "...",         IDC_LOCAL_COMPUTER_SHOW_PASSWORD, 280,  70,  13,  14
 *     LTEXT           "Comment:",    -1,                                 7,  90, 286,   9
 *     EDITTEXT        IDC_LOCAL_COMPUTER_COMMENT,                        7, 102, 286,  70,
 *                     ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL
 *     DEFPUSHBUTTON   "OK",          IDOK,                             188, 179,  50,  14
 *     PUSHBUTTON      "Cancel",      IDCANCEL,                         243, 179,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/local_computer_dialog.h"

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

// Joins a host and a port back into "host:port" form for display. If the port
// is zero, only the host is used.
std::wstring joinAddress(const std::wstring& host, uint16_t port)
{
    if (port == 0)
        return host;

    wchar_t buf[16];
    std::swprintf(buf, sizeof(buf) / sizeof(buf[0]), L":%u",
                  static_cast<unsigned int>(port));
    return host + buf;
}

}  // namespace

LocalComputerDialog::LocalComputerDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool LocalComputerDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_LOCAL_COMPUTER), parent_,
        &LocalComputerDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK LocalComputerDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    LocalComputerDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<LocalComputerDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<LocalComputerDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR LocalComputerDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

                case IDC_LOCAL_COMPUTER_SHOW_PASSWORD:
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

void LocalComputerDialog::onInitDialog(HWND hwnd)
{
    // Mirror the max-length limits used by the corresponding .ui fields.
    SendDlgItemMessageW(hwnd, IDC_LOCAL_COMPUTER_NAME,     EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_LOCAL_COMPUTER_ADDRESS,  EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_LOCAL_COMPUTER_USERNAME, EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_LOCAL_COMPUTER_PASSWORD, EM_LIMITTEXT,   64, 0);
    SendDlgItemMessageW(hwnd, IDC_LOCAL_COMPUTER_COMMENT,  EM_LIMITTEXT, 2048, 0);

    // If setInitial() was called before exec(), push those values into the
    // controls so this dialog can be used for "Edit computer" as well.
    if (!result_.group.empty())
    {
        const HWND combo = GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_GROUP);
        const int index = static_cast<int>(SendMessageW(
            combo, CB_FINDSTRINGEXACT, static_cast<WPARAM>(-1),
            reinterpret_cast<LPARAM>(result_.group.c_str())));
        if (index != CB_ERR)
        {
            SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(index), 0);
        }
        else
        {
            // Fallback: drop the group name as edit text for display.
            SetWindowTextW(combo, result_.group.c_str());
        }
    }

    SetDlgItemTextW(hwnd, IDC_LOCAL_COMPUTER_NAME,    result_.name.c_str());
    SetDlgItemTextW(hwnd, IDC_LOCAL_COMPUTER_ADDRESS,
                    joinAddress(result_.address, result_.port).c_str());
    SetDlgItemTextW(hwnd, IDC_LOCAL_COMPUTER_USERNAME, result_.username.c_str());
    SetDlgItemTextW(hwnd, IDC_LOCAL_COMPUTER_PASSWORD, result_.password.c_str());
    SetDlgItemTextW(hwnd, IDC_LOCAL_COMPUTER_COMMENT,  result_.comment.c_str());

    SetFocus(GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_NAME));
}

void LocalComputerDialog::onOk(HWND hwnd)
{
    // Read the currently selected group from the combobox (if any).
    const HWND combo = GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_GROUP);
    const LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel != CB_ERR)
    {
        const LRESULT len = SendMessageW(combo, CB_GETLBTEXTLEN,
                                         static_cast<WPARAM>(sel), 0);
        if (len > 0)
        {
            std::wstring out(static_cast<size_t>(len), L'\0');
            SendMessageW(combo, CB_GETLBTEXT, static_cast<WPARAM>(sel),
                         reinterpret_cast<LPARAM>(out.data()));
            result_.group = std::move(out);
        }
        else
        {
            result_.group.clear();
        }
    }
    else
    {
        result_.group = readEditText(combo);
    }

    result_.name = readEditText(GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_NAME));

    const std::wstring address_raw =
        readEditText(GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_ADDRESS));
    splitAddress(address_raw, &result_.address, &result_.port);

    result_.username = readEditText(GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_USERNAME));
    result_.password = readEditText(GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_PASSWORD));
    result_.comment  = readEditText(GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_COMMENT));
}

void LocalComputerDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const HWND edit = GetDlgItem(hwnd, IDC_LOCAL_COMPUTER_PASSWORD);
    SendMessageW(edit, EM_SETPASSWORDCHAR, passwordVisible_ ? 0 : L'*', 0);
    InvalidateRect(edit, nullptr, TRUE);
}

}  // namespace aspia::client_win32
