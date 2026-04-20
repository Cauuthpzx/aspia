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

#include "client_win32/sys_info_processes.h"

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

// Resource IDs - kept local to this TU until they are formally added to
// resource.h. See the comment block at the top of sys_info_processes.h.
constexpr int kIdList = 4400;

struct Column
{
    const wchar_t* text;
    int width;
};

// Columns mirror the <column> entries in sys_info_widget_processes.ui,
// in the same order (Name, ID, CPU, Memory, Session ID, User Name, Path).
const Column kColumns[] = {
    { L"Name",       200 },
    { L"ID",          70 },
    { L"CPU",         60 },
    { L"Memory",     100 },
    { L"Session ID", 80 },
    { L"User Name",  140 },
    { L"Path",       260 },
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

// Helper: setItemText() takes a const pointer but LVITEM requires non-const.
void setItemText(HWND list, int row, int sub_item, const std::wstring& text)
{
    LVITEMW item = {};
    item.iSubItem = sub_item;
    item.pszText = const_cast<wchar_t*>(text.c_str());
    SendMessageW(list, LVM_SETITEMTEXTW, static_cast<WPARAM>(row),
                 reinterpret_cast<LPARAM>(&item));
}

}  // namespace

SysInfoProcesses::SysInfoProcesses() = default;

SysInfoProcesses::~SysInfoProcesses()
{
    if (hwnd_ && IsWindow(hwnd_))
        DestroyWindow(hwnd_);
}

bool SysInfoProcesses::create(HWND parent)
{
    if (!parent)
        return false;

    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    HINSTANCE instance = reinterpret_cast<HINSTANCE>(
        GetWindowLongPtrW(parent, GWLP_HINSTANCE));

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoProcesses::windowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 0, 0,
        parent, nullptr, instance, this);

    return hwnd_ != nullptr;
}

void SysInfoProcesses::move(int x, int y, int w, int h)
{
    if (!hwnd_)
        return;

    SetWindowPos(hwnd_, nullptr, x, y, w, h,
                 SWP_NOZORDER | SWP_NOACTIVATE);

    // Make sure the widget is visible once it has a real size.
    if (w > 0 && h > 0)
        ShowWindow(hwnd_, SW_SHOW);
}

// static
LRESULT CALLBACK SysInfoProcesses::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoProcesses* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoProcesses*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoProcesses*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoProcesses::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_SIZE:
            onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void SysInfoProcesses::onCreate()
{
    HINSTANCE instance = reinterpret_cast<HINSTANCE>(
        GetWindowLongPtrW(hwnd_, GWLP_HINSTANCE));

    const DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
                        LVS_REPORT | LVS_SHOWSELALWAYS;

    list_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        style, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdList)),
        instance, nullptr);

    if (!list_)
        return;

    ListView_SetExtendedListViewStyle(
        list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

    addColumns(list_);
}

void SysInfoProcesses::onSize(int width, int height)
{
    if (!list_)
        return;

    // The ListView fills the entire client area (the .ui has a QVBoxLayout
    // with 0 margins wrapping the single QTreeWidget).
    SetWindowPos(list_, nullptr, 0, 0, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

void SysInfoProcesses::onDestroy()
{
    list_ = nullptr;
}

void SysInfoProcesses::setProcesses(const std::vector<Process>& processes)
{
    if (!list_)
        return;

    // Repopulate from scratch. The data volume is small (hundreds of rows
    // at most) so a full refresh is fine.
    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    int row = 0;
    for (const Process& p : processes)
    {
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = row;
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(p.name.c_str());

        const int inserted = static_cast<int>(SendMessageW(
            list_, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item)));
        if (inserted < 0)
            continue;

        setItemText(list_, inserted, 1, p.id);
        setItemText(list_, inserted, 2, p.cpu);
        setItemText(list_, inserted, 3, p.memory);
        setItemText(list_, inserted, 4, p.session_id);
        setItemText(list_, inserted, 5, p.user_name);
        setItemText(list_, inserted, 6, p.path);

        ++row;
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

void SysInfoProcesses::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_processes()) return;
    std::vector<Process> processes;
    for (const auto& p : si.processes().process())
    {
        Process proc;
        proc.name     = toWide(p.name());

        wchar_t numBuf[32];
        swprintf(numBuf, 32, L"%u", p.pid());
        proc.id = numBuf;

        swprintf(numBuf, 32, L"%d%%", p.cpu());
        proc.cpu = numBuf;

        proc.memory     = formatSize(static_cast<uint64_t>(p.memory()));
        swprintf(numBuf, 32, L"%u", p.sid());
        proc.session_id = numBuf;

        proc.user_name  = toWide(p.user());
        proc.path       = toWide(p.path());
        processes.push_back(std::move(proc));
    }
    setProcesses(processes);
}

}  // namespace aspia::client_win32
