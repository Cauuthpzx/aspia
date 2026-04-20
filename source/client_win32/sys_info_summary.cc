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

#include "client_win32/sys_info_summary.h"

#include <commctrl.h>
#include <ctime>

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

std::wstring formatUptime(uint64_t seconds)
{
    uint64_t d = seconds/86400, h=(seconds%86400)/3600, m=(seconds%3600)/60, s=seconds%60;
    wchar_t buf[64];
    swprintf(buf,64,L"%llud %lluh %llum %llus",
             (unsigned long long)d,(unsigned long long)h,
             (unsigned long long)m,(unsigned long long)s);
    return buf;
}

std::wstring formatTime(int64_t t)
{
    if (t == 0) return L"";
    time_t tt = static_cast<time_t>(t);
    struct tm tm_info;
    localtime_s(&tm_info, &tt);
    wchar_t buf[32];
    wcsftime(buf, 32, L"%Y-%m-%d", &tm_info);
    return buf;
}

HTREEITEM insertTreeItem(HWND tree, HTREEITEM parent, const std::wstring& text)
{
    TVINSERTSTRUCTW ins = {};
    ins.hParent = parent;
    ins.hInsertAfter = TVI_LAST;
    ins.item.mask = TVIF_TEXT;
    ins.item.pszText = const_cast<wchar_t*>(text.c_str());
    return TreeView_InsertItem(tree, &ins);
}

}  // namespace

// static
LRESULT CALLBACK SysInfoSummary::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoSummary* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoSummary*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoSummary*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoSummary::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

bool SysInfoSummary::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TREEVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    instance_ = reinterpret_cast<HINSTANCE>(
        GetWindowLongPtrW(parent, GWLP_HINSTANCE));
    if (!instance_)
        instance_ = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoSummary::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0, parent, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void SysInfoSummary::onCreate()
{
    const DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER |
                        TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |
                        TVS_FULLROWSELECT | TVS_SHOWSELALWAYS;

    tree_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"",
        style, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTree)),
        instance_, nullptr);
}

void SysInfoSummary::onSize(int width, int height)
{
    if (tree_)
    {
        SetWindowPos(tree_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoSummary::move(int x, int y, int w, int h)
{
    if (hwnd_)
    {
        SetWindowPos(hwnd_, nullptr, x, y, w, h,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoSummary::addSection(const std::wstring& root_name,
                                std::initializer_list<KeyValue> items)
{
    addSection(root_name, std::vector<KeyValue>(items.begin(), items.end()));
}

void SysInfoSummary::addSection(const std::wstring& root_name,
                                const std::vector<KeyValue>& items)
{
    if (!tree_)
        return;

    HTREEITEM root = insertTreeItem(tree_, TVI_ROOT, root_name);
    if (!root)
        return;

    for (const auto& kv : items)
    {
        std::wstring line = kv.first;
        line.append(L": ");
        line.append(kv.second);
        insertTreeItem(tree_, root, line);
    }

    TreeView_Expand(tree_, root, TVE_EXPAND);
}

void SysInfoSummary::clear()
{
    if (tree_)
        TreeView_DeleteAllItems(tree_);
}

void SysInfoSummary::setFromProto(const proto::system_info::SystemInfo& si)
{
    clear();

    // Computer
    if (si.has_computer())
    {
        const auto& c = si.computer();
        std::vector<KeyValue> items;
        if (!c.name().empty())
            items.push_back({ L"Name", toWide(c.name()) });
        if (!c.domain().empty())
            items.push_back({ L"Domain", toWide(c.domain()) });
        if (!c.workgroup().empty())
            items.push_back({ L"Workgroup", toWide(c.workgroup()) });
        if (c.uptime() > 0)
            items.push_back({ L"Uptime", formatUptime(c.uptime()) });
        if (!items.empty())
            addSection(L"Computer", items);
    }

    // Operating System
    if (si.has_operating_system())
    {
        const auto& os = si.operating_system();
        std::vector<KeyValue> items;
        if (!os.name().empty())
            items.push_back({ L"Name", toWide(os.name()) });
        if (!os.version().empty())
            items.push_back({ L"Version", toWide(os.version()) });
        if (!os.arch().empty())
            items.push_back({ L"Architecture", toWide(os.arch()) });
        if (os.install_date() != 0)
            items.push_back({ L"Install Date", formatTime(os.install_date()) });
        if (!items.empty())
            addSection(L"Operating System", items);
    }

    // Motherboard
    if (si.has_motherboard())
    {
        const auto& mb = si.motherboard();
        std::vector<KeyValue> items;
        if (!mb.manufacturer().empty())
            items.push_back({ L"Manufacturer", toWide(mb.manufacturer()) });
        if (!mb.model().empty())
            items.push_back({ L"Model", toWide(mb.model()) });
        if (!items.empty())
            addSection(L"Motherboard", items);
    }

    // BIOS
    if (si.has_bios())
    {
        const auto& bios = si.bios();
        std::vector<KeyValue> items;
        if (!bios.vendor().empty())
            items.push_back({ L"Vendor", toWide(bios.vendor()) });
        if (!bios.version().empty())
            items.push_back({ L"Version", toWide(bios.version()) });
        if (!bios.date().empty())
            items.push_back({ L"Date", toWide(bios.date()) });
        if (!items.empty())
            addSection(L"BIOS", items);
    }

    // Processor
    if (si.has_processor())
    {
        const auto& cpu = si.processor();
        std::vector<KeyValue> items;
        if (!cpu.vendor().empty())
            items.push_back({ L"Vendor", toWide(cpu.vendor()) });
        if (!cpu.model().empty())
            items.push_back({ L"Model", toWide(cpu.model()) });
        if (cpu.packages() > 0 || cpu.cores() > 0 || cpu.threads() > 0)
        {
            wchar_t buf[64];
            swprintf(buf, 64, L"%u / %u / %u",
                     cpu.packages(), cpu.cores(), cpu.threads());
            items.push_back({ L"Packages / Cores / Threads", buf });
        }
        if (!items.empty())
            addSection(L"Processor", items);
    }

    // Memory modules
    if (si.has_memory() && si.memory().module_size() > 0)
    {
        std::vector<KeyValue> items;
        for (const auto& mod : si.memory().module())
        {
            if (!mod.present())
                continue;
            std::wstring label = toWide(mod.location());
            if (label.empty())
                label = L"Module";
            std::wstring value;
            if (mod.size() > 0)
                value = formatSize(mod.size());
            if (!mod.type().empty())
            {
                if (!value.empty()) value += L" ";
                value += toWide(mod.type());
            }
            items.push_back({ label, value });
        }
        if (!items.empty())
            addSection(L"Memory", items);
    }

    // Logical Drives
    if (si.has_logical_drives() && si.logical_drives().drive_size() > 0)
    {
        std::vector<KeyValue> items;
        for (const auto& drv : si.logical_drives().drive())
        {
            std::wstring label = toWide(drv.path());
            if (!drv.file_system().empty())
                label += L" (" + toWide(drv.file_system()) + L")";
            std::wstring value;
            if (drv.total_size() > 0)
            {
                value = formatSize(drv.free_size());
                value += L" free / ";
                value += formatSize(drv.total_size());
            }
            items.push_back({ label, value });
        }
        if (!items.empty())
            addSection(L"Logical Drives", items);
    }
}

}  // namespace aspia::client_win32
