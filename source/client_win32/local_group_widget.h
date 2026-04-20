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
// Win32 replacement for client/ui/hosts/local_group_widget.ui.
//
// The Qt original is a QWidget containing a QTreeWidget with:
//   - indentation=0  (flat list, no hierarchy)
//   - sortingEnabled=true
//   - 3 columns: Name | Address | Comment
//
// The Win32 equivalent is a plain child window that owns a
// SysListView32 (LVS_REPORT) filling the entire client area.
// IDC_LOCAL_GROUP_WIDGET_LIST = 8100
//

#ifndef CLIENT_WIN32_LOCAL_GROUP_WIDGET_H
#define CLIENT_WIN32_LOCAL_GROUP_WIDGET_H

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>

namespace aspia::client_win32 {

class LocalGroupWidget
{
public:
    struct ComputerEntry
    {
        std::wstring name;
        std::wstring address;
        std::wstring comment;
    };

    explicit LocalGroupWidget(HINSTANCE instance);
    ~LocalGroupWidget();

    LocalGroupWidget(const LocalGroupWidget&) = delete;
    LocalGroupWidget& operator=(const LocalGroupWidget&) = delete;

    // Creates the child window as a child of |parent|. Returns true on success.
    bool create(HWND parent);

    // Parent-driven layout.
    void move(int x, int y, int width, int height);

    // Replace the list contents with |computers|.
    void setComputers(const std::vector<ComputerEntry>& computers);

    // Returns the index of the currently selected item, or -1 if none.
    int selectedIndex() const;

    HWND handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onDestroy();

    void createList();
    void populateList(const std::vector<ComputerEntry>& entries);

    HINSTANCE instance_;
    HWND      hwnd_ = nullptr;
    HWND      list_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32LocalGroupWidget";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_LOCAL_GROUP_WIDGET_H
