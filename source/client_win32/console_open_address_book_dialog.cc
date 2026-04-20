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
// IDD_CONSOLE_OPEN_ADDRESS_BOOK DIALOGEX 0, 0, 240, 110
// STYLE DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION "Open Address Book"
// FONT 8, "MS Shell Dlg", 400, 0, 0x1
// BEGIN
//     GROUPBOX     "", IDC_STATIC, 7, 7, 226, 78
//     LTEXT        "Address book is encrypted. To open, you must enter a password.",
//                  IDC_CONSOLE_OPEN_ADDRESS_BOOK_PROMPT, 14, 16, 212, 16
//     LTEXT        "File:",
//                  IDC_CONSOLE_OPEN_ADDRESS_BOOK_FILE_LABEL, 14, 38, 50, 10
//     EDITTEXT     IDC_CONSOLE_OPEN_ADDRESS_BOOK_FILE,
//                  68, 36, 158, 12, ES_AUTOHSCROLL | ES_READONLY
//     LTEXT        "Encryption Type:",
//                  IDC_CONSOLE_OPEN_ADDRESS_BOOK_ENC_LABEL, 14, 54, 50, 10
//     EDITTEXT     IDC_CONSOLE_OPEN_ADDRESS_BOOK_ENC,
//                  68, 52, 158, 12, ES_AUTOHSCROLL | ES_READONLY
//     LTEXT        "Password:",
//                  IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD_LABEL, 14, 70, 50, 10
//     EDITTEXT     IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD,
//                  68, 68, 138, 12, ES_PASSWORD | ES_AUTOHSCROLL
//     AUTOCHECKBOX "Show", IDC_CONSOLE_OPEN_ADDRESS_BOOK_SHOW,
//                  209, 70, 28, 10
//     DEFPUSHBUTTON "OK",     IDOK,     128, 91, 50, 14
//     PUSHBUTTON    "Cancel", IDCANCEL, 183, 91, 50, 14
// END
//

#include "client_win32/console_open_address_book_dialog.h"

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

ConsoleOpenAddressBookDialog::ConsoleOpenAddressBookDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool ConsoleOpenAddressBookDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_CONSOLE_OPEN_ADDRESS_BOOK), parent_,
        &ConsoleOpenAddressBookDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK ConsoleOpenAddressBookDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleOpenAddressBookDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<ConsoleOpenAddressBookDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<ConsoleOpenAddressBookDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR ConsoleOpenAddressBookDialog::handleMessage(
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

                case IDC_CONSOLE_OPEN_ADDRESS_BOOK_SHOW:
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

void ConsoleOpenAddressBookDialog::onInitDialog(HWND hwnd)
{
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD, EM_LIMITTEXT, 128, 0);
    SetFocus(GetDlgItem(hwnd, IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD));
}

void ConsoleOpenAddressBookDialog::onOk(HWND hwnd)
{
    result_.password =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD));
}

void ConsoleOpenAddressBookDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const HWND edit = GetDlgItem(hwnd, IDC_CONSOLE_OPEN_ADDRESS_BOOK_PWD);
    SendMessageW(edit, EM_SETPASSWORDCHAR, passwordVisible_ ? 0 : L'*', 0);
    InvalidateRect(edit, nullptr, TRUE);
}

}  // namespace aspia::client_win32
