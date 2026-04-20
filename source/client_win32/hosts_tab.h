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
// Win32 replacement for client/ui/hosts/hosts_tab.ui.
//
// Control IDs (range 4300..4399, to be added to resource.h by a separate
// change):
//   #define IDC_HOSTS_SEARCH    4300   // search edit (top-left)
//   #define IDC_HOSTS_TREE      4301   // group hierarchy (SysTreeView32)
//   #define IDC_HOSTS_LIST      4302   // computer list (SysListView32, REPORT)
//

#ifndef CLIENT_WIN32_HOSTS_TAB_H
#define CLIENT_WIN32_HOSTS_TAB_H

#include <windows.h>
#include <commctrl.h>

namespace aspia::client_win32 {

class HostsTab
{
public:
    explicit HostsTab(HINSTANCE instance);
    ~HostsTab();

    HostsTab(const HostsTab&) = delete;
    HostsTab& operator=(const HostsTab&) = delete;

    // Creates the child window as a child of |parent|. Returns true on success.
    bool create(HWND parent);

    // Parent-driven layout: move/resize this tab's container window.
    void move(int x, int y, int width, int height);

    // Container HWND (use for Show/Hide from the parent tab control).
    HWND handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onNotify(LPARAM lp);
    void onCommand(WPARAM wp);
    void onDestroy();

    void createChildren();
    void layoutChildren(int width, int height);

    // TVN_SELCHANGED handler: invoked when the selected tree item changes.
    void onTreeSelectionChanged(NMTREEVIEWW* nm);

    // Stub data loaders. Real values will arrive from the address-book model
    // later (see console/address_book_tab.cc for the legacy Qt path).
    void populateGroups();      // TODO(phase2): fill tree_ from AddressBook.
    void populateComputers();   // TODO(phase2): fill list_ for current group.

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;     // container
    HWND search_ = nullptr;   // edit control
    HWND tree_ = nullptr;     // group tree (left pane)
    HWND list_ = nullptr;     // computer list (right pane)

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32HostsTab";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_HOSTS_TAB_H
