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
// Win32 replacement for client/ui/sys_info/sys_info_widget_open_files.ui.
//
// Control IDs (reserved 5800..5899, not yet in resource.h):
//   #define IDC_SYSINFO_OPEN_FILES_LIST   5800
//

#ifndef CLIENT_WIN32_SYS_INFO_OPEN_FILES_H
#define CLIENT_WIN32_SYS_INFO_OPEN_FILES_H

#include "proto/system_info.h"

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <vector>

namespace aspia::client_win32 {

class SysInfoOpenFiles
{
public:
    struct OpenFile
    {
        std::wstring id;
        std::wstring username;
        std::wstring locksCount;
        std::wstring filePath;
    };

    explicit SysInfoOpenFiles(HINSTANCE instance);
    ~SysInfoOpenFiles();

    SysInfoOpenFiles(const SysInfoOpenFiles&) = delete;
    SysInfoOpenFiles& operator=(const SysInfoOpenFiles&) = delete;

    bool create(HWND parent);
    HWND handle() const { return hwnd_; }
    void move(int x, int y, int width, int height);

    void setOpenFiles(const std::vector<OpenFile>& files);
    void setFromProto(const proto::system_info::SystemInfo& si);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND list_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoOpenFiles";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYS_INFO_OPEN_FILES_H
