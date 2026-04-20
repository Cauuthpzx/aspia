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
// Win32 replacement for client/ui/hosts/router_group_widget.ui.
//
// The Qt original is a QWidget containing a QTreeWidget with:
//   - indentation=0  (flat list, no hierarchy)
//   - sortingEnabled=true
//   - 3 columns: Name | Address / ID | Comment
//
// The Win32 equivalent is a plain child window that owns a
// SysListView32 (LVS_REPORT) filling the entire client area.
// The columns differ from LocalGroupWidget: the second column header is
// "Address / ID" instead of "Address".
//
// IDC_ROUTER_GROUP_WIDGET_LIST = 8120
//

#ifndef CLIENT_WIN32_ROUTER_GROUP_WIDGET_H
#define CLIENT_WIN32_ROUTER_GROUP_WIDGET_H

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>

namespace aspia::client_win32 {

class RouterGroupWidget
{
public:
    struct ComputerEntry
    {
        std::wstring name;
        std::wstring address_or_id;
        std::wstring comment;
    };

    explicit RouterGroupWidget(HINSTANCE instance);
    ~RouterGroupWidget();

    RouterGroupWidget(const RouterGroupWidget&) = delete;
    RouterGroupWidget& operator=(const RouterGroupWidget&) = delete;

    bool create(HWND parent);
    void move(int x, int y, int width, int height);
    void setComputers(const std::vector<ComputerEntry>& computers);
    int  selectedIndex() const;

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

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32RouterGroupWidget";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_ROUTER_GROUP_WIDGET_H
