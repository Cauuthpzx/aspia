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

#include "client_win32/sys_info_drivers.h"

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

const wchar_t* driverStatusToString(proto::system_info::Drivers::Driver::Status s)
{
    using S = proto::system_info::Drivers::Driver::Status;
    switch (s)
    {
        case S::STATUS_CONTINUE_PENDING: return L"Continue Pending";
        case S::STATUS_PAUSE_PENDING:    return L"Pause Pending";
        case S::STATUS_PAUSED:           return L"Paused";
        case S::STATUS_RUNNING:          return L"Running";
        case S::STATUS_START_PENDING:    return L"Start Pending";
        case S::STATUS_STOP_PENDING:     return L"Stop Pending";
        case S::STATUS_STOPPED:          return L"Stopped";
        default:                         return L"Unknown";
    }
}

const wchar_t* driverStartupToString(proto::system_info::Drivers::Driver::StartupType t)
{
    using T = proto::system_info::Drivers::Driver::StartupType;
    switch (t)
    {
        case T::STARTUP_TYPE_AUTO_START:   return L"Auto Start";
        case T::STARTUP_TYPE_DEMAND_START: return L"Demand Start";
        case T::STARTUP_TYPE_DISABLED:     return L"Disabled";
        case T::STARTUP_TYPE_BOOT_START:   return L"Boot Start";
        case T::STARTUP_TYPE_SYSTEM_START: return L"System Start";
        default:                           return L"Unknown";
    }
}

// Control IDs - matches IDC_SYSINFO_DRIVERS_LIST.
constexpr int kIdList = 5000;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kColumns[] = {
    { L"Display Name",    200 },
    { L"Name",            160 },
    { L"Description",     260 },
    { L"Status",          100 },
    { L"Startup Type",    120 },
    { L"Executable File", 320 },
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

SysInfoDrivers::SysInfoDrivers(HINSTANCE instance)
    : instance_(instance)
{
}

SysInfoDrivers::~SysInfoDrivers() = default;

bool SysInfoDrivers::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoDrivers::windowProc;
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

void SysInfoDrivers::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK SysInfoDrivers::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoDrivers* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoDrivers*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoDrivers*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoDrivers::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void SysInfoDrivers::onCreate()
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

void SysInfoDrivers::onSize(int width, int height)
{
    if (list_)
    {
        SetWindowPos(list_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoDrivers::setDrivers(const std::vector<Driver>& drivers)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    for (size_t i = 0; i < drivers.size(); ++i)
    {
        const Driver& d = drivers[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(d.displayName.c_str());

        const int row = ListView_InsertItem(list_, &item);
        if (row < 0)
            continue;

        setSubItem(list_, row, 1, d.name);
        setSubItem(list_, row, 2, d.description);
        setSubItem(list_, row, 3, d.status);
        setSubItem(list_, row, 4, d.startupType);
        setSubItem(list_, row, 5, d.executableFile);
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

void SysInfoDrivers::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_drivers()) return;
    std::vector<Driver> drvs;
    for (const auto& d : si.drivers().driver())
    {
        Driver drv;
        drv.displayName    = toWide(d.display_name());
        drv.name           = toWide(d.name());
        drv.description    = toWide(d.description());
        drv.status         = driverStatusToString(d.status());
        drv.startupType    = driverStartupToString(d.startup_type());
        drv.executableFile = toWide(d.binary_path());
        drvs.push_back(std::move(drv));
    }
    setDrivers(drvs);
}

}  // namespace aspia::client_win32
