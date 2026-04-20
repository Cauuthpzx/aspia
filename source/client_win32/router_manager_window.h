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
// Win32 replacement for client/ui/router_manager/router_manager_window.ui.
//
// Resource IDs expected (to be added to resource.h by a separate change):
//   #define IDC_ROUTER_MGR_TABS          4000
//   #define IDC_ROUTER_MGR_LIST_USERS    4001
//   #define IDC_ROUTER_MGR_LIST_HOSTS    4002
//   #define IDC_ROUTER_MGR_LIST_ACTIVE   4003
//   #define IDC_ROUTER_MGR_STATUSBAR     4004
//   #define IDC_ROUTER_MGR_TOOLBAR       4005
//   #define IDC_ROUTER_MGR_ADD           4010
//   #define IDC_ROUTER_MGR_EDIT          4011
//   #define IDC_ROUTER_MGR_REMOVE        4012
//   #define IDC_ROUTER_MGR_REFRESH       4013
//   #define IDC_ROUTER_MGR_DISCONNECT    4014
//   #define IDC_ROUTER_MGR_REFRESH_ACT   4015
//   #define IDC_ROUTER_MGR_CLOSE         4016
//   #define IDR_ROUTER_MGR_ACCEL          400
//

#ifndef CLIENT_WIN32_ROUTER_MANAGER_WINDOW_H
#define CLIENT_WIN32_ROUTER_MANAGER_WINDOW_H

#include <windows.h>

#include <span>

namespace aspia::client_win32 {

// Forward-declared row types. Real payloads arrive over the router admin
// protocol (see proto/router_admin.proto); these structs are populated by
// whatever glue layer translates proto messages into UI rows.
struct RouterUserRow;
struct RouterHostRow;
struct RouterActiveRow;

class RouterManagerWindow
{
public:
    enum Tab
    {
        kTabUsers  = 0,
        kTabHosts  = 1,
        kTabActive = 2,
        kTabCount  = 3,
    };

    explicit RouterManagerWindow(HINSTANCE instance);
    ~RouterManagerWindow();

    RouterManagerWindow(const RouterManagerWindow&) = delete;
    RouterManagerWindow& operator=(const RouterManagerWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND handle() const { return hwnd_; }

    // Data reload entry points. Real data arrives over the router admin
    // protocol; the owning controller calls these to refresh the UI.
    void reloadUsers(std::span<const RouterUserRow> rows);
    void reloadHosts(std::span<const RouterHostRow> rows);
    void reloadActive(std::span<const RouterActiveRow> rows);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onNotify(LPARAM lp);
    void onCommand(int id);
    void onDestroy();

    void createToolbar();
    void createTabs();
    void createLists();
    void createStatusBar();

    void showTab(int index);
    void layoutChildren(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND toolbar_ = nullptr;
    HWND tabs_ = nullptr;
    HWND status_ = nullptr;
    HWND lists_[kTabCount] = {};
    HACCEL accel_ = nullptr;
    int currentTab_ = kTabUsers;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32RouterManager";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_ROUTER_MANAGER_WINDOW_H
