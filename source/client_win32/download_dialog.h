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
// Win32 replacement for common/ui/download_dialog.ui.
//
// Resource IDs for this dialog (add to client_win32/resource.h):
//
// #define IDD_DOWNLOAD              230
// #define IDC_DOWNLOAD_STATUS       2300
// #define IDC_DOWNLOAD_PROGRESS     2301
//

#ifndef CLIENT_WIN32_DOWNLOAD_DIALOG_H
#define CLIENT_WIN32_DOWNLOAD_DIALOG_H

#include <windows.h>
#include <commctrl.h>

#include <string>

namespace aspia::client_win32 {

class DownloadDialog
{
public:
    DownloadDialog(HINSTANCE instance, HWND parent);
    ~DownloadDialog() = default;

    DownloadDialog(const DownloadDialog&) = delete;
    DownloadDialog& operator=(const DownloadDialog&) = delete;

    // Runs the dialog modally. Returns true if the dialog ended with IDOK,
    // false for IDCANCEL (e.g. the user pressed the Cancel button or closed
    // the window). The owner is disabled while this call is active.
    bool exec();

    // Sets the progress bar position. |percent| is clamped to [0, 100].
    // Safe to call from the thread that owns the dialog; when the dialog is
    // not yet created this stores the value and applies it on WM_INITDIALOG.
    void setProgress(int percent);

    // Replaces the status line text shown above the progress bar.
    void setStatus(const std::wstring& text);

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    HWND hwnd_ = nullptr;
    int progress_ = 0;
    std::wstring status_ = L"Downloading update in progress...";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_DOWNLOAD_DIALOG_H
