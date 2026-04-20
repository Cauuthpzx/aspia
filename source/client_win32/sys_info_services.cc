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

#include "client_win32/sys_info_services.h"

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

std::wstring serviceStatusToString(proto::system_info::Services::Service::Status status)
{
    using S = proto::system_info::Services::Service;
    switch (status)
    {
        case S::STATUS_RUNNING:          return L"Running";
        case S::STATUS_STOPPED:          return L"Stopped";
        case S::STATUS_PAUSED:           return L"Paused";
        case S::STATUS_START_PENDING:    return L"Start Pending";
        case S::STATUS_STOP_PENDING:     return L"Stop Pending";
        case S::STATUS_CONTINUE_PENDING: return L"Continue Pending";
        case S::STATUS_PAUSE_PENDING:    return L"Pause Pending";
        default:                         return L"Unknown";
    }
}

std::wstring serviceStartupToString(proto::system_info::Services::Service::StartupType st)
{
    using S = proto::system_info::Services::Service;
    switch (st)
    {
        case S::STARTUP_TYPE_AUTO_START:   return L"Auto Start";
        case S::STARTUP_TYPE_DEMAND_START: return L"Manual";
        case S::STARTUP_TYPE_DISABLED:     return L"Disabled";
        case S::STARTUP_TYPE_BOOT_START:   return L"Boot Start";
        case S::STARTUP_TYPE_SYSTEM_START: return L"System Start";
        default:                           return L"Unknown";
    }
}

// Control IDs - kept local until they are formally added to resource.h
// (see comment block at top of sys_info_services.h). Reserved range
// for the Services system-info widget: 4700..4799.
constexpr int kIdList = 4700;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kColumns[] = {
    { L"Display Name",    200 },
    { L"Name",            160 },
    { L"Description",     300 },
    { L"Status",          100 },
    { L"Startup Type",    120 },
    { L"Account",         180 },
    { L"Executable File", 300 },
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

SysInfoServices::SysInfoServices(HINSTANCE instance)
    : instance_(instance)
{
}

SysInfoServices::~SysInfoServices() = default;

bool SysInfoServices::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoServices::windowProc;
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

void SysInfoServices::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK SysInfoServices::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoServices* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoServices*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoServices*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoServices::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void SysInfoServices::onCreate()
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

void SysInfoServices::onSize(int width, int height)
{
    if (list_)
    {
        SetWindowPos(list_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoServices::setServices(const std::vector<Service>& services)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    for (size_t i = 0; i < services.size(); ++i)
    {
        const Service& s = services[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(s.displayName.c_str());

        const int row = ListView_InsertItem(list_, &item);
        if (row < 0)
            continue;

        setSubItem(list_, row, 1, s.name);
        setSubItem(list_, row, 2, s.description);
        setSubItem(list_, row, 3, s.status);
        setSubItem(list_, row, 4, s.startupType);
        setSubItem(list_, row, 5, s.account);
        setSubItem(list_, row, 6, s.executableFile);
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

void SysInfoServices::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_services()) return;
    std::vector<Service> services;
    for (const auto& s : si.services().service())
    {
        Service svc;
        svc.displayName    = toWide(s.display_name());
        svc.name           = toWide(s.name());
        svc.description    = toWide(s.description());
        svc.status         = serviceStatusToString(s.status());
        svc.startupType    = serviceStartupToString(s.startup_type());
        svc.account        = toWide(s.start_name());
        svc.executableFile = toWide(s.binary_path());
        services.push_back(std::move(svc));
    }
    setServices(services);
}

}  // namespace aspia::client_win32
