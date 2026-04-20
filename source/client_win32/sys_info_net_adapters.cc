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

#include "client_win32/sys_info_net_adapters.h"

#include "proto/system_info.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Control IDs - kept local until they are formally added to resource.h
// (see comment block at top of sys_info_net_adapters.h). Reserved range
// for the Network Adapters system-info widget: 5600..5699.
constexpr int kIdList = 5600;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kColumns[] = {
    { L"Adapter Name",    240 },
    { L"Connection Name", 160 },
    { L"MAC Address",     140 },
    { L"IP Addresses",    180 },
    { L"Gateways",        160 },
    { L"DHCP Enabled",    100 },
    { L"DNS Servers",     200 },
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

std::wstring toWide(const std::string& s)
{
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) return {};
    std::wstring r(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, r.data(), n);
    return r;
}

// Join a repeated string field (UTF-8) into a comma-separated wide string.
template<typename RepeatedField>
std::wstring joinWide(const RepeatedField& field)
{
    std::wstring result;
    for (int i = 0; i < field.size(); ++i)
    {
        if (i != 0)
            result.append(L", ");
        result.append(toWide(field.Get(i)));
    }
    return result;
}

}  // namespace

SysInfoNetAdapters::SysInfoNetAdapters(HINSTANCE instance)
    : instance_(instance)
{
}

SysInfoNetAdapters::~SysInfoNetAdapters() = default;

bool SysInfoNetAdapters::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoNetAdapters::windowProc;
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

void SysInfoNetAdapters::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK SysInfoNetAdapters::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoNetAdapters* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoNetAdapters*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoNetAdapters*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoNetAdapters::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void SysInfoNetAdapters::onCreate()
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

void SysInfoNetAdapters::onSize(int width, int height)
{
    if (list_)
    {
        SetWindowPos(list_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoNetAdapters::setAdapters(const std::vector<NetAdapter>& adapters)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    for (size_t i = 0; i < adapters.size(); ++i)
    {
        const NetAdapter& a = adapters[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(a.adapterName.c_str());

        const int row = ListView_InsertItem(list_, &item);
        if (row < 0)
            continue;

        setSubItem(list_, row, 1, a.connectionName);
        setSubItem(list_, row, 2, a.macAddress);
        setSubItem(list_, row, 3, a.ipAddresses);
        setSubItem(list_, row, 4, a.gateways);
        setSubItem(list_, row, 5, a.dhcpEnabled);
        setSubItem(list_, row, 6, a.dnsServers);
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

void SysInfoNetAdapters::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_network_adapters())
        return;

    std::vector<NetAdapter> adapters;
    const proto::system_info::NetworkAdapters& na = si.network_adapters();

    for (int i = 0; i < na.adapter_size(); ++i)
    {
        const proto::system_info::NetworkAdapters::Adapter& p = na.adapter(i);

        NetAdapter a;
        a.adapterName    = toWide(p.adapter_name());
        a.connectionName = toWide(p.connection_name());
        a.macAddress     = toWide(p.mac());
        a.dhcpEnabled    = p.dhcp_enabled() ? L"Yes" : L"No";
        a.gateways       = joinWide(p.gateway());
        a.dnsServers     = joinWide(p.dns());

        // Build IP address list: "ip/mask, ip/mask, ..."
        std::wstring ipList;
        for (int j = 0; j < p.address_size(); ++j)
        {
            if (j != 0)
                ipList.append(L", ");
            ipList.append(toWide(p.address(j).ip()));
            if (!p.address(j).mask().empty())
            {
                ipList.append(L"/");
                ipList.append(toWide(p.address(j).mask()));
            }
        }
        a.ipAddresses = std::move(ipList);

        adapters.push_back(std::move(a));
    }

    setAdapters(adapters);
}

}  // namespace aspia::client_win32
