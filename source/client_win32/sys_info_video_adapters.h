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
// Win32 replacement for client/ui/sys_info/sys_info_widget_video_adapters.ui.
//
// Control IDs (reserved 6100..6199, not yet in resource.h):
//   #define IDC_SYSINFO_VIDEO_ADAPTERS_LIST   6100
//

#ifndef CLIENT_WIN32_SYS_INFO_VIDEO_ADAPTERS_H
#define CLIENT_WIN32_SYS_INFO_VIDEO_ADAPTERS_H

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>

#include "proto/system_info.h"

namespace aspia::client_win32 {

class SysInfoVideoAdapters
{
public:
    struct VideoAdapter
    {
        std::wstring name;
        std::wstring adapterString;
        std::wstring biosString;
        std::wstring chipType;
        std::wstring dacType;
        std::wstring memorySize;
        std::wstring driverVersion;
        std::wstring driverDate;
    };

    explicit SysInfoVideoAdapters(HINSTANCE instance);
    ~SysInfoVideoAdapters();

    SysInfoVideoAdapters(const SysInfoVideoAdapters&) = delete;
    SysInfoVideoAdapters& operator=(const SysInfoVideoAdapters&) = delete;

    bool create(HWND parent);
    HWND handle() const { return hwnd_; }
    void move(int x, int y, int width, int height);

    void setAdapters(const std::vector<VideoAdapter>& adapters);
    void setFromProto(const proto::system_info::SystemInfo& si);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoVideoAdapters";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYS_INFO_VIDEO_ADAPTERS_H
