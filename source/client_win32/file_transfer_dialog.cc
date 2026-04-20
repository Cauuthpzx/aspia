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
// runtime via DialogBoxParamW(IDD_FILE_TRANSFER, ...). The equivalent
// resource script entry is:
//
// IDD_FILE_TRANSFER DIALOGEX 0, 0, 320, 160
// STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
// CAPTION "File Transfer"
// FONT 9, "Segoe UI", 400, 0, 0x1
// BEGIN
//     LTEXT           "Current Task: Creating a list of files to copy...",
//                                     IDC_FILE_TRANSFER_STATUS,
//                                                               7,   7, 306,   9
//     LTEXT           "",             IDC_FILE_TRANSFER_CURRENT_ITEM,
//                                                               7,  22, 306,   9
//     LTEXT           "Speed: ...",   IDC_FILE_TRANSFER_SPEED,
//                                                               7,  36, 306,   9
//     GROUPBOX        "Progress",     -1,                        7,  52, 306,  78
//     LTEXT           "Total:",       IDC_FILE_TRANSFER_OVERALL_LABEL,
//                                                              14,  66,  40,   9
//     CONTROL         "",             IDC_FILE_TRANSFER_PROGRESS_OVERALL,
//                                     "msctls_progress32", WS_BORDER,
//                                                              58,  65, 248,  12
//     LTEXT           "Item:",        IDC_FILE_TRANSFER_ITEM_LABEL,
//                                                              14,  86,  40,   9
//     CONTROL         "",             IDC_FILE_TRANSFER_PROGRESS_ITEM,
//                                     "msctls_progress32", WS_BORDER,
//                                                              58,  85, 248,  12
//     PUSHBUTTON      "Cancel",       IDCANCEL,                263, 139,  50,  14
// END
//

#include "client_win32/file_transfer_dialog.h"

#include "client_win32/resource.h"

#include <algorithm>
#include <cwchar>

#include <commctrl.h>
#include <shlwapi.h>

namespace aspia::client_win32 {

FileTransferDialog::FileTransferDialog(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

bool FileTransferDialog::exec()
{
    const INT_PTR result = DialogBoxParamW(
        instance_, MAKEINTRESOURCEW(IDD_FILE_TRANSFER), parent_,
        &FileTransferDialog::dialogProc, reinterpret_cast<LPARAM>(this));
    return result == IDOK;
}

void FileTransferDialog::setOverallProgress(int percent)
{
    overall_progress_ = std::clamp(percent, 0, 100);
    if (hwnd_)
    {
        SendDlgItemMessageW(
            hwnd_, IDC_FILE_TRANSFER_PROGRESS_OVERALL, PBM_SETPOS,
            static_cast<WPARAM>(overall_progress_), 0);
    }
}

void FileTransferDialog::setItemProgress(int percent)
{
    item_progress_ = std::clamp(percent, 0, 100);
    if (hwnd_)
    {
        SendDlgItemMessageW(
            hwnd_, IDC_FILE_TRANSFER_PROGRESS_ITEM, PBM_SETPOS,
            static_cast<WPARAM>(item_progress_), 0);
    }
}

void FileTransferDialog::setCurrentItem(const std::wstring& path)
{
    current_item_ = path;
    if (hwnd_)
        applyCurrentItem();
}

void FileTransferDialog::setSpeed(int64_t bytes_per_sec)
{
    speed_text_ = formatSpeed(bytes_per_sec);
    if (hwnd_)
        applySpeed();
}

void FileTransferDialog::setStatus(const std::wstring& text)
{
    status_ = text;
    if (hwnd_)
        SetDlgItemTextW(hwnd_, IDC_FILE_TRANSFER_STATUS, status_.c_str());
}

// static
INT_PTR CALLBACK FileTransferDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    FileTransferDialog* self = nullptr;

    if (msg == WM_INITDIALOG)
    {
        self = reinterpret_cast<FileTransferDialog*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lp));
    }
    else
    {
        self = reinterpret_cast<FileTransferDialog*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self)
        return FALSE;

    return self->handleMessage(hwnd, msg, wp, lp);
}

INT_PTR FileTransferDialog::handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
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

void FileTransferDialog::onInitDialog(HWND hwnd)
{
    hwnd_ = hwnd;

    SetDlgItemTextW(hwnd, IDC_FILE_TRANSFER_STATUS, status_.c_str());

    SendDlgItemMessageW(
        hwnd, IDC_FILE_TRANSFER_PROGRESS_OVERALL, PBM_SETRANGE32, 0, 100);
    SendDlgItemMessageW(
        hwnd, IDC_FILE_TRANSFER_PROGRESS_OVERALL, PBM_SETPOS,
        static_cast<WPARAM>(overall_progress_), 0);

    SendDlgItemMessageW(
        hwnd, IDC_FILE_TRANSFER_PROGRESS_ITEM, PBM_SETRANGE32, 0, 100);
    SendDlgItemMessageW(
        hwnd, IDC_FILE_TRANSFER_PROGRESS_ITEM, PBM_SETPOS,
        static_cast<WPARAM>(item_progress_), 0);

    applyCurrentItem();
    applySpeed();

    SetFocus(GetDlgItem(hwnd, IDCANCEL));
}

void FileTransferDialog::applyCurrentItem()
{
    // Shorten the path so it fits on the label. 128 chars is well above
    // anything reasonable for a one-line label at this width.
    constexpr size_t kMaxChars = 128;
    wchar_t buffer[kMaxChars] = {};

    const size_t len = current_item_.size();
    if (len < kMaxChars)
    {
        if (len > 0)
            std::wmemcpy(buffer, current_item_.c_str(), len);
        buffer[len] = L'\0';
    }
    else
    {
        std::wmemcpy(buffer, current_item_.c_str(), kMaxChars - 1);
        buffer[kMaxChars - 1] = L'\0';
        // Compact in place to fit the conceptual label width.
        PathCompactPathW(nullptr, buffer, 300);
    }

    SetDlgItemTextW(hwnd_, IDC_FILE_TRANSFER_CURRENT_ITEM, buffer);
}

void FileTransferDialog::applySpeed()
{
    SetDlgItemTextW(hwnd_, IDC_FILE_TRANSFER_SPEED, speed_text_.c_str());
}

// static
std::wstring FileTransferDialog::formatSpeed(int64_t bytes_per_sec)
{
    if (bytes_per_sec < 0)
        bytes_per_sec = 0;

    const wchar_t* unit = L"B/s";
    double value = static_cast<double>(bytes_per_sec);

    if (value >= 1024.0 * 1024.0 * 1024.0)
    {
        value /= 1024.0 * 1024.0 * 1024.0;
        unit = L"GB/s";
    }
    else if (value >= 1024.0 * 1024.0)
    {
        value /= 1024.0 * 1024.0;
        unit = L"MB/s";
    }
    else if (value >= 1024.0)
    {
        value /= 1024.0;
        unit = L"KB/s";
    }

    wchar_t buffer[64] = {};
    _snwprintf_s(
        buffer, _countof(buffer), _TRUNCATE, L"Speed: %.2f %ls", value, unit);
    return buffer;
}

}  // namespace aspia::client_win32
