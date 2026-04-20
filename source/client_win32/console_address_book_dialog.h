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
// Win32 replacement for console/address_book_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_CONSOLE_ADDRESS_BOOK                    450
//   #define IDC_CONSOLE_AB_NAME                        6800
//   #define IDC_CONSOLE_AB_ENCRYPTION                  6801
//   #define IDC_CONSOLE_AB_PASSWORD                    6802
//   #define IDC_CONSOLE_AB_PASSWORD_RETRY              6803
//   #define IDC_CONSOLE_AB_SHOW_PASSWORD               6804
//   #define IDC_CONSOLE_AB_COMMENT                     6805
//   #define IDC_CONSOLE_AB_LABEL_NAME                  6806
//   #define IDC_CONSOLE_AB_LABEL_ENCRYPTION            6807
//   #define IDC_CONSOLE_AB_LABEL_PASSWORD              6808
//   #define IDC_CONSOLE_AB_LABEL_PASSWORD_RETRY        6809
//   #define IDC_CONSOLE_AB_LABEL_COMMENT               6810
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_CONSOLE_ADDRESS_BOOK_DIALOG_H
#define CLIENT_WIN32_CONSOLE_ADDRESS_BOOK_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class ConsoleAddressBookDialog
{
public:
    // Encryption type ordinals. These mirror the ordering of the combo-box
    // entries and correspond to the proto::address_book::EncryptionType enum
    // used by the address-book file format; keep the ordering stable.
    enum EncryptionType
    {
        kEncryptionNone             = 0,
        kEncryptionChaCha20Poly1305 = 1,
    };

    struct Result
    {
        std::wstring name;
        int encryption_type = kEncryptionChaCha20Poly1305;
        std::wstring password;
        std::wstring password_retry;
        std::wstring comment;
    };

    ConsoleAddressBookDialog(HINSTANCE instance, HWND parent);
    ~ConsoleAddressBookDialog() = default;

    ConsoleAddressBookDialog(const ConsoleAddressBookDialog&) = delete;
    ConsoleAddressBookDialog& operator=(const ConsoleAddressBookDialog&) = delete;

    void setInitial(const Result& initial);

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onEncryptionChanged(HWND hwnd);
    void onOk(HWND hwnd);
    void togglePasswordVisibility(HWND hwnd);

    void updatePasswordEnableState(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result initial_;
    Result result_;
    bool passwordVisible_ = false;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_CONSOLE_ADDRESS_BOOK_DIALOG_H
