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
// Win32 replacement for client/ui/sys_info/sys_info_widget_routes.ui.
//
// Control IDs (reserved 6000..6099, not yet in resource.h):
//   #define IDC_SYSINFO_ROUTES_LIST   6000
//

#ifndef CLIENT_WIN32_SYS_INFO_ROUTES_H
#define CLIENT_WIN32_SYS_INFO_ROUTES_H

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>

#include "proto/system_info.h"

namespace aspia::client_win32 {

class SysInfoRoutes
{
public:
    struct Route
    {
        std::wstring destination;
        std::wstring netmask;
        std::wstring gateway;
        std::wstring metric;
    };

    explicit SysInfoRoutes(HINSTANCE instance);
    ~SysInfoRoutes();

    SysInfoRoutes(const SysInfoRoutes&) = delete;
    SysInfoRoutes& operator=(const SysInfoRoutes&) = delete;

    bool create(HWND parent);
    HWND handle() const { return hwnd_; }
    void move(int x, int y, int width, int height);

    void setRoutes(const std::vector<Route>& routes);
    void setFromProto(const proto::system_info::SystemInfo& si);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoRoutes";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYS_INFO_ROUTES_H
