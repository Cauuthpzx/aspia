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
// Win32 replacement for client/ui/file_transfer/file_transfer_dialog.ui.
//
// Resource IDs for this dialog (add to client_win32/resource.h):
//
// #define IDD_FILE_TRANSFER                  300
// #define IDC_FILE_TRANSFER_CURRENT_ITEM     3000
// #define IDC_FILE_TRANSFER_SPEED            3001
// #define IDC_FILE_TRANSFER_OVERALL_LABEL    3002
// #define IDC_FILE_TRANSFER_ITEM_LABEL       3003
// #define IDC_FILE_TRANSFER_PROGRESS_OVERALL 3004
// #define IDC_FILE_TRANSFER_PROGRESS_ITEM    3005
// #define IDC_FILE_TRANSFER_STATUS           3006
//

#ifndef CLIENT_WIN32_FILE_TRANSFER_DIALOG_H
#define CLIENT_WIN32_FILE_TRANSFER_DIALOG_H

#include <windows.h>
#include <commctrl.h>

#include <cstdint>
#include <string>

namespace aspia::client_win32 {

class FileTransferDialog
{
public:
    FileTransferDialog(HINSTANCE instance, HWND parent);
    ~FileTransferDialog() = default;

    FileTransferDialog(const FileTransferDialog&) = delete;
    FileTransferDialog& operator=(const FileTransferDialog&) = delete;

    // Runs the dialog modally. Returns true if the dialog ended with IDOK,
    // false for IDCANCEL (e.g. the user pressed the Cancel button or closed
    // the window). The owner is disabled while this call is active.
    bool exec();

    // Sets the overall (total) progress bar position. |percent| is clamped
    // to [0, 100]. Values set before the dialog is created are cached and
    // applied on WM_INITDIALOG.
    void setOverallProgress(int percent);

    // Sets the current-item progress bar position. |percent| is clamped to
    // [0, 100].
    void setItemProgress(int percent);

    // Updates the label showing the path of the file currently being
    // transferred. Long paths are shortened with PathCompactPathW so they fit
    // on a single line.
    void setCurrentItem(const std::wstring& path);

    // Updates the speed label. The value is formatted using the appropriate
    // unit (B/s, KB/s, MB/s, GB/s) with two decimal places.
    void setSpeed(int64_t bytes_per_sec);

    // Replaces the status line text shown at the top of the dialog.
    void setStatus(const std::wstring& text);

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void applyCurrentItem();
    void applySpeed();

    static std::wstring formatSpeed(int64_t bytes_per_sec);

    HINSTANCE instance_;
    HWND parent_;
    HWND hwnd_ = nullptr;

    int overall_progress_ = 0;
    int item_progress_ = 0;
    std::wstring current_item_;
    std::wstring speed_text_ = L"Speed: ...";
    std::wstring status_ = L"Current Task: Creating a list of files to copy...";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_FILE_TRANSFER_DIALOG_H
