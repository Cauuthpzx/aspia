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

#include "client_win32/sys_info_power_options.h"

#include "proto/system_info.h"

#include <commctrl.h>

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
LRESULT CALLBACK SysInfoPowerOptions::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoPowerOptions* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoPowerOptions*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoPowerOptions*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoPowerOptions::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

bool SysInfoPowerOptions::create(HWND parent)
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
    wc.lpfnWndProc = &SysInfoPowerOptions::windowProc;
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

void SysInfoPowerOptions::onCreate()
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

void SysInfoPowerOptions::onSize(int width, int height)
{
    if (tree_)
    {
        SetWindowPos(tree_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoPowerOptions::move(int x, int y, int w, int h)
{
    if (hwnd_)
    {
        SetWindowPos(hwnd_, nullptr, x, y, w, h,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoPowerOptions::addSection(const std::wstring& root_name,
                                     std::initializer_list<KeyValue> items)
{
    addSection(root_name, std::vector<KeyValue>(items.begin(), items.end()));
}

void SysInfoPowerOptions::addSection(const std::wstring& root_name,
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

void SysInfoPowerOptions::addBatteryEntry(const Battery& b)
{
    addSection(L"Battery", {
        { L"Vendor",                b.vendor },
        { L"Model",                 b.model },
        { L"Serial",                b.serial },
        { L"Design Capacity",       b.design_capacity },
        { L"Full Charged Capacity", b.full_charged_capacity },
        { L"Current Capacity",      b.current_capacity },
        { L"Status",                b.status },
        { L"Health",                b.health },
    });
}

void SysInfoPowerOptions::clear()
{
    if (tree_)
        TreeView_DeleteAllItems(tree_);
}

void SysInfoPowerOptions::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_power_options())
        return;

    clear();

    const proto::system_info::PowerOptions& po = si.power_options();

    // Determine power source string.
    const wchar_t* powerSourceStr = L"Unknown";
    switch (po.power_source())
    {
        case proto::system_info::PowerOptions::POWER_SOURCE_AC_LINE:
            powerSourceStr = L"AC Line";
            break;
        case proto::system_info::PowerOptions::POWER_SOURCE_DC_BATTERY:
            powerSourceStr = L"DC Battery";
            break;
        default:
            break;
    }

    // Determine battery status string.
    const wchar_t* batteryStatusStr = L"Unknown";
    switch (po.battery_status())
    {
        case proto::system_info::PowerOptions::BATTERY_STATUS_HIGH:
            batteryStatusStr = L"High";
            break;
        case proto::system_info::PowerOptions::BATTERY_STATUS_LOW:
            batteryStatusStr = L"Low";
            break;
        case proto::system_info::PowerOptions::BATTERY_STATUS_CRITICAL:
            batteryStatusStr = L"Critical";
            break;
        case proto::system_info::PowerOptions::BATTERY_STATUS_CHARGING:
            batteryStatusStr = L"Charging";
            break;
        case proto::system_info::PowerOptions::BATTERY_STATUS_NO_BATTERY:
            batteryStatusStr = L"No Battery";
            break;
        default:
            break;
    }

    // Format battery life times.
    std::wstring fullLifeTime;
    std::wstring remainingLifeTime;

    if (po.full_battery_life_time() != 0)
    {
        wchar_t buf[64];
        uint64_t secs = po.full_battery_life_time();
        swprintf(buf, 64, L"%llu:%02llu:%02llu",
                 (unsigned long long)(secs / 3600),
                 (unsigned long long)((secs % 3600) / 60),
                 (unsigned long long)(secs % 60));
        fullLifeTime = buf;
    }

    if (po.remaining_battery_life_time() != 0)
    {
        wchar_t buf[64];
        uint64_t secs = po.remaining_battery_life_time();
        swprintf(buf, 64, L"%llu:%02llu:%02llu",
                 (unsigned long long)(secs / 3600),
                 (unsigned long long)((secs % 3600) / 60),
                 (unsigned long long)(secs % 60));
        remainingLifeTime = buf;
    }

    wchar_t pctBuf[32] = {};
    swprintf(pctBuf, 32, L"%u%%", po.battery_life_percent());

    addSection(L"Power Options", {
        { L"Power Source",            powerSourceStr },
        { L"Battery Status",          batteryStatusStr },
        { L"Battery Life",            pctBuf },
        { L"Full Battery Life Time",  fullLifeTime },
        { L"Remaining Battery Time",  remainingLifeTime },
    });

    // Add each battery.
    for (int i = 0; i < po.battery_size(); ++i)
    {
        const proto::system_info::PowerOptions::Battery& b = po.battery(i);

        Battery bat;
        bat.vendor = toWide(b.manufacturer());
        bat.model  = toWide(b.device_name());
        bat.serial = toWide(b.serial_number());

        wchar_t buf[64];
        swprintf(buf, 64, L"%u mWh", b.design_capacity());
        bat.design_capacity = buf;

        swprintf(buf, 64, L"%u mWh", b.full_charged_capacity());
        bat.full_charged_capacity = buf;

        swprintf(buf, 64, L"%u mWh", b.current_capacity());
        bat.current_capacity = buf;

        // State is a bitmask.
        std::wstring stateStr;
        uint32_t state = b.state();
        if (state & proto::system_info::PowerOptions::Battery::STATE_CHARGING)
        {
            if (!stateStr.empty()) stateStr.append(L", ");
            stateStr.append(L"Charging");
        }
        if (state & proto::system_info::PowerOptions::Battery::STATE_DISCHARGING)
        {
            if (!stateStr.empty()) stateStr.append(L", ");
            stateStr.append(L"Discharging");
        }
        if (state & proto::system_info::PowerOptions::Battery::STATE_CRITICAL)
        {
            if (!stateStr.empty()) stateStr.append(L", ");
            stateStr.append(L"Critical");
        }
        if (state & proto::system_info::PowerOptions::Battery::STATE_POWER_ONLINE)
        {
            if (!stateStr.empty()) stateStr.append(L", ");
            stateStr.append(L"Power Online");
        }
        if (stateStr.empty())
            stateStr = L"Unknown";
        bat.status = stateStr;

        // Health = full_charged_capacity / design_capacity * 100 %
        if (b.design_capacity() > 0)
        {
            double health = (static_cast<double>(b.full_charged_capacity()) /
                             static_cast<double>(b.design_capacity())) * 100.0;
            swprintf(buf, 64, L"%.1f%%", health);
            bat.health = buf;
        }

        addBatteryEntry(bat);
    }
}

}  // namespace aspia::client_win32
