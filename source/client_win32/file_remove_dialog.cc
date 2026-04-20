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
// Reference DIALOGEX template for client.rc. This dialog is created at
// runtime via DialogBoxParamW(IDD_FILE_REMOVE, ...). The equivalent resource
// script entry is:
//
// IDD_FILE_REMOVE DIALOGEX 0, 0, 250, 90
// STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION "Deleting Items"
// FONT 9, "Segoe UI", 400, 0, 0x1
// BEGIN
//     LTEXT           "Creating a list of files to delete...",
//                                     IDC_FILE_REMOVE_CURRENT_ITEM,
//                                                           7,   7, 236,   9
//     CONTROL         "",             IDC_FILE_REMOVE_PROGRESS,
//                                     "msctls_progress32", WS_BORDER,
//                                                           7,  22, 236,  12
//     PUSHBUTTON      "Cancel",       IDCANCEL,            193,  69,  50,  14
// END
//

#include "client_win32/file_remove_dialog.h"

#include "client_win32/resource.h"

#include <algorithm>

#include <commctrl.h>

namespace aspia::client_win32 {

FileRemoveDialog::FileRemoveDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool FileRemoveDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_FILE_REMOVE), parent_,
        &FileRemoveDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

void FileRemoveDialog::setProgress(int percent)
{
    progress_ = std::clamp(percent, 0, 100);
    if (hwnd_)
    {
        SendDlgItemMessageW(
            hwnd_, IDC_FILE_REMOVE_PROGRESS, PBM_SETPOS,
            static_cast<WPARAM>(progress_), 0);
    }
}

void FileRemoveDialog::setCurrentItem(const std::wstring& path)
{
    currentItem_ = path;
    if (hwnd_)
    {
        SetDlgItemTextW(hwnd_, IDC_FILE_REMOVE_CURRENT_ITEM, currentItem_.c_str());
    }
}

// static
INT_PTR CALLBACK FileRemoveDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    FileRemoveDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<FileRemoveDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<FileRemoveDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR FileRemoveDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                default:
                    break;
            }
            break;

        case WM_DESTROY:
            hwnd_ = nullptr;
            break;

        default:
            break;
    }

    return FALSE;
}

void FileRemoveDialog::onInitDialog(HWND hwnd)
{
    hwnd_ = hwnd;

    SetDlgItemTextW(hwnd, IDC_FILE_REMOVE_CURRENT_ITEM, currentItem_.c_str());

    SendDlgItemMessageW(hwnd, IDC_FILE_REMOVE_PROGRESS, PBM_SETRANGE32, 0, 100);
    SendDlgItemMessageW(
        hwnd, IDC_FILE_REMOVE_PROGRESS, PBM_SETPOS,
        static_cast<WPARAM>(progress_), 0);

    SetFocus(GetDlgItem(hwnd, IDCANCEL));
}

}  // namespace aspia::client_win32
