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

#include "client_win32/sys_info_connections.h"

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

// Control IDs - kept local until they are formally added to resource.h
// (see comment block at top of sys_info_connections.h). Reserved range
// for the Connections system-info widget: 4800..4899.
constexpr int kIdList = 4800;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kColumns[] = {
    { L"Process Name",   200 },
    { L"Protocol",        80 },
    { L"Local Address",  140 },
    { L"Local Port",      80 },
    { L"Remote Address", 140 },
    { L"Remote Port",     80 },
    { L"State",          120 },
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

SysInfoConnections::SysInfoConnections(HINSTANCE instance)
    : instance_(instance)
{
}

SysInfoConnections::~SysInfoConnections() = default;

bool SysInfoConnections::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoConnections::windowProc;
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

void SysInfoConnections::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK SysInfoConnections::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoConnections* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoConnections*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoConnections*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoConnections::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void SysInfoConnections::onCreate()
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

void SysInfoConnections::onSize(int width, int height)
{
    if (list_)
    {
        SetWindowPos(list_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoConnections::setConnections(const std::vector<Connection>& connections)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    for (size_t i = 0; i < connections.size(); ++i)
    {
        const Connection& c = connections[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(c.processName.c_str());

        const int row = ListView_InsertItem(list_, &item);
        if (row < 0)
            continue;

        setSubItem(list_, row, 1, c.protocol);
        setSubItem(list_, row, 2, c.localAddress);
        setSubItem(list_, row, 3, c.localPort);
        setSubItem(list_, row, 4, c.remoteAddress);
        setSubItem(list_, row, 5, c.remotePort);
        setSubItem(list_, row, 6, c.state);
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

void SysInfoConnections::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_connections()) return;
    std::vector<Connection> conns;
    for (const auto& c : si.connections().connection())
    {
        Connection conn;
        conn.processName   = toWide(c.process_name());
        conn.protocol      = toWide(c.protocol());
        conn.localAddress  = toWide(c.local_address());
        conn.remoteAddress = toWide(c.remote_address());

        wchar_t portBuf[16];
        swprintf(portBuf, 16, L"%u", c.local_port());
        conn.localPort = portBuf;
        swprintf(portBuf, 16, L"%u", c.remote_port());
        conn.remotePort = portBuf;

        conn.state = toWide(c.state());
        conns.push_back(std::move(conn));
    }
    setConnections(conns);
}

}  // namespace aspia::client_win32
