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

#include "client_win32/sys_info_video_adapters.h"

#include <commctrl.h>

#include "proto/system_info.h"

namespace aspia::client_win32 {

namespace {

std::wstring toWide(const std::string& s)
{
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) return {};
    std::wstring r(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, r.data(), n);
    return r;
}

std::wstring formatSize(uint64_t bytes)
{
    wchar_t buf[64];
    if (bytes >= 1024ULL*1024*1024) swprintf(buf,64,L"%.1f GB",bytes/(1024.0*1024*1024));
    else if (bytes >= 1024ULL*1024) swprintf(buf,64,L"%.1f MB",bytes/(1024.0*1024));
    else if (bytes >= 1024)         swprintf(buf,64,L"%.1f KB",bytes/1024.0);
    else                             swprintf(buf,64,L"%llu B",(unsigned long long)bytes);
    return buf;
}

// Control IDs - kept local until they are formally added to resource.h
// (see comment block at top of sys_info_video_adapters.h). Reserved range
// for the Video Adapters system-info widget: 6100..6199.
constexpr int kIdList = 6100;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kColumns[] = {
    { L"Name",           200 },
    { L"Adapter String", 180 },
    { L"BIOS String",    180 },
    { L"Chip Type",      150 },
    { L"DAC Type",       120 },
    { L"Memory Size",    100 },
    { L"Driver Version", 120 },
    { L"Driver Date",    100 },
};

constexpr int kColumnCount = static_cast<int>(sizeof(kColumns) / sizeof(kColumns[0]));

void addColumns(HWND list)
{
    for (int i = 0; i < kColumnCount; ++i)
    {
        LVCOLUMNW col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = const_cast<wchar_t*>(kColumns[i].text);
        col.cx = kColumns[i].width;
        col.iSubItem = i;
        ListView_InsertColumn(list, i, &col);
    }
}

void setSubItem(HWND list, int row, int col, const std::wstring& text)
{
    ListView_SetItemText(list, row, col, const_cast<wchar_t*>(text.c_str()));
}

}  // namespace

SysInfoVideoAdapters::SysInfoVideoAdapters(HINSTANCE instance)
    : instance_(instance)
{
}

SysInfoVideoAdapters::~SysInfoVideoAdapters() = default;

bool SysInfoVideoAdapters::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoVideoAdapters::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 0, 0, parent, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void SysInfoVideoAdapters::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK SysInfoVideoAdapters::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoVideoAdapters* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoVideoAdapters*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoVideoAdapters*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoVideoAdapters::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_SIZE:
            onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void SysInfoVideoAdapters::onCreate()
{
    const DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER |
                        LVS_REPORT | LVS_SHOWSELALWAYS;

    list_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        style, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdList)),
        instance_, nullptr);

    if (!list_)
        return;

    ListView_SetExtendedListViewStyle(
        list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

    addColumns(list_);
}

void SysInfoVideoAdapters::onSize(int width, int height)
{
    if (list_)
    {
        SetWindowPos(list_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoVideoAdapters::setAdapters(const std::vector<VideoAdapter>& adapters)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    for (size_t i = 0; i < adapters.size(); ++i)
    {
        const VideoAdapter& a = adapters[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(a.name.c_str());

        const int row = ListView_InsertItem(list_, &item);
        if (row < 0)
            continue;

        setSubItem(list_, row, 1, a.adapterString);
        setSubItem(list_, row, 2, a.biosString);
        setSubItem(list_, row, 3, a.chipType);
        setSubItem(list_, row, 4, a.dacType);
        setSubItem(list_, row, 5, a.memorySize);
        setSubItem(list_, row, 6, a.driverVersion);
        setSubItem(list_, row, 7, a.driverDate);
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

void SysInfoVideoAdapters::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_video_adapters()) return;
    std::vector<VideoAdapter> adapters;
    for (const auto& a : si.video_adapters().adapter())
    {
        VideoAdapter va;
        va.name          = toWide(a.description());
        va.adapterString = toWide(a.adapter_string());
        va.biosString    = toWide(a.bios_string());
        va.chipType      = toWide(a.chip_type());
        va.dacType       = toWide(a.dac_type());
        va.memorySize    = formatSize(a.memory_size());
        va.driverVersion = toWide(a.driver_version());
        va.driverDate    = toWide(a.driver_date());
        adapters.push_back(std::move(va));
    }
    setAdapters(adapters);
}

}  // namespace aspia::client_win32
