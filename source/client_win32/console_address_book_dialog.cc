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
 * IDD_CONSOLE_ADDRESS_BOOK DIALOGEX 0, 0, 320, 240
 * STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
 * CAPTION "Address Book Properties"
 * FONT 9, "Segoe UI", 400, 0, 0x1
 * BEGIN
 *     LTEXT           "Name:",              IDC_CONSOLE_AB_LABEL_NAME,             7,  10,  90,   9
 *     EDITTEXT        IDC_CONSOLE_AB_NAME,                                       100,   8, 213,  12, ES_AUTOHSCROLL
 *     LTEXT           "Encryption:",        IDC_CONSOLE_AB_LABEL_ENCRYPTION,       7,  28,  90,   9
 *     COMBOBOX        IDC_CONSOLE_AB_ENCRYPTION,                                 100,  26, 213,  60, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
 *     LTEXT           "Password:",          IDC_CONSOLE_AB_LABEL_PASSWORD,         7,  46,  90,   9
 *     EDITTEXT        IDC_CONSOLE_AB_PASSWORD,                                   100,  44, 197,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     PUSHBUTTON      "...",                IDC_CONSOLE_AB_SHOW_PASSWORD,        300,  43,  13,  14
 *     LTEXT           "Password (repeat):", IDC_CONSOLE_AB_LABEL_PASSWORD_RETRY,   7,  62,  90,   9
 *     EDITTEXT        IDC_CONSOLE_AB_PASSWORD_RETRY,                             100,  60, 213,  12, ES_AUTOHSCROLL | ES_PASSWORD
 *     LTEXT           "Comment:",           IDC_CONSOLE_AB_LABEL_COMMENT,          7,  80,  90,   9
 *     EDITTEXT        IDC_CONSOLE_AB_COMMENT,                                      7,  92, 306, 124, ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL
 *     DEFPUSHBUTTON   "OK",                 IDOK,                                208, 220,  50,  14
 *     PUSHBUTTON      "Cancel",             IDCANCEL,                            263, 220,  50,  14
 * END
 * ---------------------------------------------------------------------------
 */

#include "client_win32/console_address_book_dialog.h"

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

// Combo-box entries, in insertion order. The zero-based index of each entry
// doubles as the EncryptionType value reported on the Result struct.
struct EncryptionEntry
{
    int type;
    const wchar_t* label;
};

constexpr EncryptionEntry kEncryptionEntries[] =
{
    { ConsoleAddressBookDialog::kEncryptionNone,             L"None"             },
    { ConsoleAddressBookDialog::kEncryptionChaCha20Poly1305, L"ChaCha20Poly1305" },
};

}  // namespace

ConsoleAddressBookDialog::ConsoleAddressBookDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

void ConsoleAddressBookDialog::setInitial(const Result& initial)
{
    initial_ = initial;
    result_ = initial;
}

bool ConsoleAddressBookDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_CONSOLE_ADDRESS_BOOK), parent_,
        &ConsoleAddressBookDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

// static
INT_PTR CALLBACK ConsoleAddressBookDialog::dialogProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleAddressBookDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<ConsoleAddressBookDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<ConsoleAddressBookDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR ConsoleAddressBookDialog::handleMessage(
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

                case IDC_CONSOLE_AB_SHOW_PASSWORD:
                    togglePasswordVisibility(hwnd);
                    return TRUE;

                case IDC_CONSOLE_AB_ENCRYPTION:
                    if (HIWORD(wp) == CBN_SELCHANGE)
                    {
                        onEncryptionChanged(hwnd);
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

void ConsoleAddressBookDialog::onInitDialog(HWND hwnd)
{
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_AB_NAME,            EM_LIMITTEXT, 64,  0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_AB_PASSWORD,        EM_LIMITTEXT, 64,  0);
    SendDlgItemMessageW(hwnd, IDC_CONSOLE_AB_PASSWORD_RETRY,  EM_LIMITTEXT, 64,  0);

    const HWND combo = GetDlgItem(hwnd, IDC_CONSOLE_AB_ENCRYPTION);
    int selected_index = 0;
    for (size_t i = 0; i < sizeof(kEncryptionEntries) / sizeof(kEncryptionEntries[0]); ++i)
    {
        const EncryptionEntry& e = kEncryptionEntries[i];
        SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(e.label));
        SendMessageW(combo, CB_SETITEMDATA, static_cast<WPARAM>(i),
                     static_cast<LPARAM>(e.type));
        if (e.type == initial_.encryption_type)
            selected_index = static_cast<int>(i);
    }
    SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(selected_index), 0);

    SetDlgItemTextW(hwnd, IDC_CONSOLE_AB_NAME,           initial_.name.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_AB_PASSWORD,       initial_.password.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_AB_PASSWORD_RETRY,
                    initial_.password_retry.empty()
                        ? initial_.password.c_str()
                        : initial_.password_retry.c_str());
    SetDlgItemTextW(hwnd, IDC_CONSOLE_AB_COMMENT,        initial_.comment.c_str());

    updatePasswordEnableState(hwnd);

    SetFocus(GetDlgItem(hwnd, IDC_CONSOLE_AB_NAME));
}

void ConsoleAddressBookDialog::onEncryptionChanged(HWND hwnd)
{
    updatePasswordEnableState(hwnd);
}

void ConsoleAddressBookDialog::updatePasswordEnableState(HWND hwnd)
{
    const HWND combo = GetDlgItem(hwnd, IDC_CONSOLE_AB_ENCRYPTION);
    const LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);

    int type = kEncryptionNone;
    if (sel != CB_ERR)
    {
        const LRESULT data = SendMessageW(
            combo, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
        if (data != CB_ERR)
            type = static_cast<int>(data);
    }

    const BOOL enable = (type != kEncryptionNone) ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_AB_PASSWORD),       enable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_AB_PASSWORD_RETRY), enable);
    EnableWindow(GetDlgItem(hwnd, IDC_CONSOLE_AB_SHOW_PASSWORD),  enable);
}

void ConsoleAddressBookDialog::onOk(HWND hwnd)
{
    result_.name     = readEditText(GetDlgItem(hwnd, IDC_CONSOLE_AB_NAME));
    result_.password = readEditText(GetDlgItem(hwnd, IDC_CONSOLE_AB_PASSWORD));
    result_.password_retry =
        readEditText(GetDlgItem(hwnd, IDC_CONSOLE_AB_PASSWORD_RETRY));
    result_.comment  = readEditText(GetDlgItem(hwnd, IDC_CONSOLE_AB_COMMENT));

    const HWND combo = GetDlgItem(hwnd, IDC_CONSOLE_AB_ENCRYPTION);
    const LRESULT sel = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (sel != CB_ERR)
    {
        const LRESULT data = SendMessageW(
            combo, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
        if (data != CB_ERR)
            result_.encryption_type = static_cast<int>(data);
    }
}

void ConsoleAddressBookDialog::togglePasswordVisibility(HWND hwnd)
{
    passwordVisible_ = !passwordVisible_;
    const wchar_t mask = passwordVisible_ ? 0 : L'*';

    const HWND pwd = GetDlgItem(hwnd, IDC_CONSOLE_AB_PASSWORD);
    SendMessageW(pwd, EM_SETPASSWORDCHAR, mask, 0);
    InvalidateRect(pwd, nullptr, TRUE);

    const HWND pwd_retry = GetDlgItem(hwnd, IDC_CONSOLE_AB_PASSWORD_RETRY);
    SendMessageW(pwd_retry, EM_SETPASSWORDCHAR, mask, 0);
    InvalidateRect(pwd_retry, nullptr, TRUE);
}

}  // namespace aspia::client_win32
