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
// Win32 replacement for client/ui/file_transfer/file_remove_dialog.ui.
//
// Resource IDs for this dialog (add to client_win32/resource.h):
//
// #define IDD_FILE_REMOVE               310
// #define IDC_FILE_REMOVE_CURRENT_ITEM  3100
// #define IDC_FILE_REMOVE_PROGRESS      3101
//

#ifndef CLIENT_WIN32_FILE_REMOVE_DIALOG_H
#define CLIENT_WIN32_FILE_REMOVE_DIALOG_H

#include <windows.h>
#include <commctrl.h>

#include <string>

namespace aspia::client_win32 {

class FileRemoveDialog
{
public:
    FileRemoveDialog(HINSTANCE instance, HWND parent);
    ~FileRemoveDialog() = default;

    FileRemoveDialog(const FileRemoveDialog&) = delete;
    FileRemoveDialog& operator=(const FileRemoveDialog&) = delete;

    // Runs the dialog modally. Returns true if the dialog ended with IDOK
    // (i.e. the removal finished), false for IDCANCEL (the user pressed the
    // Cancel button or closed the window). The owner is disabled while this
    // call is active.
    bool exec();

    // Sets the progress bar position. |percent| is clamped to [0, 100].
    // Safe to call from the thread that owns the dialog; when the dialog is
    // not yet created this stores the value and applies it on WM_INITDIALOG.
    void setProgress(int percent);

    // Replaces the status line shown above the progress bar with the path of
    // the item that is currently being removed.
    void setCurrentItem(const std::wstring& path);

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    HWND hwnd_ = nullptr;
    int progress_ = 0;
    std::wstring currentItem_ = L"Creating a list of files to delete...";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_FILE_REMOVE_DIALOG_H
