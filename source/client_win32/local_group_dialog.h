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
// Win32 replacement for client/ui/hosts/local_group_dialog.ui.
//
// ---------------------------------------------------------------------------
// Resource IDs for this dialog. These should be added to resource.h by the
// resource-file owner (do NOT edit resource.h from here):
//
//   #define IDD_LOCAL_GROUP                 350
//   #define IDC_LOCAL_GROUP_PARENT         3501
//   #define IDC_LOCAL_GROUP_NAME           3502
//   #define IDC_LOCAL_GROUP_COMMENT        3503
// ---------------------------------------------------------------------------
//

#ifndef CLIENT_WIN32_LOCAL_GROUP_DIALOG_H
#define CLIENT_WIN32_LOCAL_GROUP_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class LocalGroupDialog
{
public:
    struct Result
    {
        std::wstring name;
        std::wstring parent_group;
        std::wstring comment;
    };

    LocalGroupDialog(HINSTANCE instance, HWND parent);
    ~LocalGroupDialog() = default;

    LocalGroupDialog(const LocalGroupDialog&) = delete;
    LocalGroupDialog& operator=(const LocalGroupDialog&) = delete;

    // Pre-populates the dialog fields. Call before exec() to repurpose this
    // dialog as an "Edit group" dialog.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_LOCAL_GROUP_DIALOG_H
