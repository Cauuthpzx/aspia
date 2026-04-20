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
// Win32 replacement for client/ui/sys_info/sys_info_widget_processes.ui.
//
// This class is the TEMPLATE for the 17 other sys_info widgets in the
// client/ui/sys_info/ tree (memory, drives, cpu, network cards, video
// adapters, monitors, printers, services, drivers, environment, event
// log, open files, connections, routes, users, groups, licenses, etc.).
// Each of those follows the exact same pattern: a single SysListView32 in
// LVS_REPORT mode filling the client area, one column per QTreeWidget
// header, one row per item. To port another widget:
//   1. Copy this .h/.cc pair, rename the class / file / window-class / IDs.
//   2. Replace the kColumns array with the columns from that .ui file.
//   3. Replace the Process struct with the matching row struct and rename
//      setProcesses() accordingly.
// No dialog template / DIALOGEX is involved: this is a pure child widget
// owned by a parent tab control in the system-info session window.
//
// Resource IDs owned by this widget (range 4400..4499):
//   #define IDC_SYSINFO_PROCESSES_LIST  4400
//

#ifndef CLIENT_WIN32_SYS_INFO_PROCESSES_H
#define CLIENT_WIN32_SYS_INFO_PROCESSES_H

#include <windows.h>
#include <commctrl.h>

#include <cstdint>
#include <string>
#include <vector>

#include "proto/system_info.h"

namespace aspia::client_win32 {

class SysInfoProcesses
{
public:
    // One row in the processes list. Field order matches the column order
    // declared in sys_info_widget_processes.ui.
    struct Process
    {
        std::wstring name;
        std::wstring id;          // Process ID (PID) as string.
        std::wstring cpu;         // CPU % as pre-formatted string.
        std::wstring memory;      // Memory usage as pre-formatted string.
        std::wstring session_id;  // Session ID as string.
        std::wstring user_name;
        std::wstring path;        // Executable image path.
    };

    SysInfoProcesses();
    ~SysInfoProcesses();

    SysInfoProcesses(const SysInfoProcesses&) = delete;
    SysInfoProcesses& operator=(const SysInfoProcesses&) = delete;

    // Creates the child window as a child of |parent|. Returns true on
    // success. The widget is created hidden and positioned at 0,0 with
    // zero size - the owner must call move() to lay it out.
    bool create(HWND parent);

    // Parent-driven geometry. All coordinates are in the parent's client
    // area.
    void move(int x, int y, int w, int h);

    // Handle of the outer child window (the one registered under
    // kClassName). The ListView is an inner child of this window.
    HWND handle() const { return hwnd_; }

    // Replaces the contents of the list with |processes|. Called whenever
    // a fresh snapshot arrives from the host over IPC.
    void setProcesses(const std::vector<Process>& processes);
    void setFromProto(const proto::system_info::SystemInfo& si);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onDestroy();

    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoProcesses";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYS_INFO_PROCESSES_H
