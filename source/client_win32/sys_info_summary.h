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
// Win32 replacement for client/ui/sys_info/sys_info_widget_summary.ui.
//
// Control IDs (range 4500..4599, kept local to this TU until added to
// resource.h by a separate change):
//   #define IDC_SYSINFO_SUMMARY_TREE  4500
//

#ifndef CLIENT_WIN32_SYS_INFO_SUMMARY_H
#define CLIENT_WIN32_SYS_INFO_SUMMARY_H

#include <windows.h>
#include <commctrl.h>

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace aspia::client_win32 {

class SysInfoSummary
{
public:
    using KeyValue = std::pair<std::wstring, std::wstring>;

    SysInfoSummary() = default;
    ~SysInfoSummary() = default;

    SysInfoSummary(const SysInfoSummary&) = delete;
    SysInfoSummary& operator=(const SysInfoSummary&) = delete;

    bool create(HWND parent);

    HWND handle() const { return hwnd_; }

    void move(int x, int y, int w, int h);

    // Inserts a new top-level group named `root_name` and fills it with
    // "key: value" children. Each call appends a new root.
    void addSection(const std::wstring& root_name,
                    std::initializer_list<KeyValue> items);
    void addSection(const std::wstring& root_name,
                    const std::vector<KeyValue>& items);

    // Removes all items from the tree.
    void clear();

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);

    HWND hwnd_ = nullptr;
    HWND tree_ = nullptr;
    HINSTANCE instance_ = nullptr;

    static constexpr int kIdTree = 4500;
    static constexpr wchar_t kClassName[] = L"AspiaClientWin32SysInfoSummary";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_SYS_INFO_SUMMARY_H
