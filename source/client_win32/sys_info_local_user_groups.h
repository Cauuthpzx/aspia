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
// Win32 replacement for client/ui/sys_info/sys_info_widget_local_user_groups.ui.
//
// Control IDs:
//   #define IDC_SYSINFO_LOCAL_USER_GROUPS_LIST   5400
//

#ifndef CLIENT_WIN32_SYS_INFO_LOCAL_USER_GROUPS_H
#define CLIENT_WIN32_SYS_INFO_LOCAL_USER_GROUPS_H

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>

namespace aspia::client_win32 {

class SysInfoLocalUserGroups
{
public:
    struct LocalUserGroup
    {
        std::wstring name;
        std::wstring description;
    };

    explicit SysInfoLocalUserGroups(HINSTANCE instance);
    ~SysInfoLocalUserGroups();

    SysInfoLocalUserGroups(const SysInfoLocalUserGroups&) = delete;
    SysInfoLocalUserGroups& operator=(const SysInfoLocalUserGroups&) = delete;

    bool create(HWND parent);
    HWND handle() const { return hwnd_; }
    void move(int x, int y, int width, int height);

    void setGroups(const std::vector<LocalUserGroup>& groups);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoLocalUserGroups";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYS_INFO_LOCAL_USER_GROUPS_H
