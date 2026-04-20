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
// runtime via DialogBoxParamW(IDD_DOWNLOAD, ...). The equivalent resource
// script entry is:
//
// IDD_DOWNLOAD DIALOGEX 0, 0, 235, 76
// STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION "Downloading"
// FONT 9, "Segoe UI", 400, 0, 0x1
// BEGIN
//     GROUPBOX        "",             -1,                  7,   4, 221,  46
//     LTEXT           "Downloading update in progress...",
//                                     IDC_DOWNLOAD_STATUS, 14,  16, 207,   9
//     CONTROL         "",             IDC_DOWNLOAD_PROGRESS,
//                                     "msctls_progress32", WS_BORDER,
//                                                          14,  30, 207,  12
//     PUSHBUTTON      "Cancel",       IDCANCEL,           178,  56,  50,  14
// END
//

#include "client_win32/download_dialog.h"

#include "client_win32/resource.h"

#include <algorithm>

#include <commctrl.h>

namespace aspia::client_win32 {

DownloadDialog::DownloadDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool DownloadDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_DOWNLOAD), parent_,
        &DownloadDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

void DownloadDialog::setProgress(int percent)
{
    progress_ = std::clamp(percent, 0, 100);
    if (hwnd_)
    {
        SendDlgItemMessageW(
            hwnd_, IDC_DOWNLOAD_PROGRESS, PBM_SETPOS,
            static_cast<WPARAM>(progress_), 0);
    }
}

void DownloadDialog::setStatus(const std::wstring& text)
{
    status_ = text;
    if (hwnd_)
    {
        SetDlgItemTextW(hwnd_, IDC_DOWNLOAD_STATUS, status_.c_str());
    }
}

// static
INT_PTR CALLBACK DownloadDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    DownloadDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<DownloadDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<DownloadDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR DownloadDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

void DownloadDialog::onInitDialog(HWND hwnd)
{
    hwnd_ = hwnd;

    SetDlgItemTextW(hwnd, IDC_DOWNLOAD_STATUS, status_.c_str());

    SendDlgItemMessageW(hwnd, IDC_DOWNLOAD_PROGRESS, PBM_SETRANGE32, 0, 100);
    SendDlgItemMessageW(
        hwnd, IDC_DOWNLOAD_PROGRESS, PBM_SETPOS,
        static_cast<WPARAM>(progress_), 0);

    SetFocus(GetDlgItem(hwnd, IDCANCEL));
}

}  // namespace aspia::client_win32
