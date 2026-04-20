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
// Win32 replacement for client/ui/desktop/task_manager_window.ui.
//
// Resource IDs expected (to be added to resource.h by a separate change):
//   #define IDC_TASKMGR_TABS          2900
//   #define IDC_TASKMGR_LIST_PROCESS  2901
//   #define IDC_TASKMGR_LIST_SERVICE  2902
//   #define IDC_TASKMGR_LIST_USERS    2903
//   #define IDC_TASKMGR_LIST_PERF     2904
//   #define IDC_TASKMGR_STATUSBAR     2905
//   #define IDC_TASKMGR_END_TASK      2910
//   #define IDC_TASKMGR_DISCONNECT    2911
//   #define IDC_TASKMGR_LOGOFF        2912
//   #define IDR_TASKMGR_ACCEL         290
//

#ifndef CLIENT_WIN32_TASK_MANAGER_WINDOW_H
#define CLIENT_WIN32_TASK_MANAGER_WINDOW_H

#include <windows.h>

namespace aspia::client_win32 {

class TaskManagerWindow
{
public:
    enum Tab
    {
        kTabProcesses   = 0,
        kTabServices    = 1,
        kTabUsers       = 2,
        kTabPerformance = 3,
        kTabCount       = 4,
    };

    explicit TaskManagerWindow(HINSTANCE instance);
    ~TaskManagerWindow();

    TaskManagerWindow(const TaskManagerWindow&) = delete;
    TaskManagerWindow& operator=(const TaskManagerWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onNotify(LPARAM lp);
    void onCommand(int id);
    void onDestroy();

    void createTabs();
    void createLists();
    void createStatusBar();

    void showTab(int index);
    void layoutLists(int width, int height);

    // Stub data loaders - real data will arrive from host/win/task_manager.{cc,h}
    // over IPC/proto (see proto/task_manager.proto, to be wired up later).
    void populateProcesses();   // TODO: fill from proto::TaskManager::ProcessList
    void populateServices();    // TODO: fill from proto::TaskManager::ServiceList
    void populateUsers();       // TODO: fill from proto::TaskManager::SessionList
    void populatePerformance(); // TODO: fill from proto::TaskManager::Performance

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND tabs_ = nullptr;
    HWND status_ = nullptr;
    HWND lists_[kTabCount] = {};
    HACCEL accel_ = nullptr;
    int currentTab_ = kTabProcesses;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32TaskManager";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_TASK_MANAGER_WINDOW_H
