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

#include "client_win32/sys_info_event_logs.h"

#include <commctrl.h>
#include <time.h>

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

std::wstring formatTimestamp(int64_t t)
{
    if (t == 0) return {};
    time_t ts = static_cast<time_t>(t);
    struct tm tm_val = {};
#if defined(_WIN32)
    localtime_s(&tm_val, &ts);
#else
    localtime_r(&ts, &tm_val);
#endif
    wchar_t buf[32];
    swprintf(buf, 32, L"%04d-%02d-%02d %02d:%02d:%02d",
             tm_val.tm_year + 1900, tm_val.tm_mon + 1, tm_val.tm_mday,
             tm_val.tm_hour, tm_val.tm_min, tm_val.tm_sec);
    return buf;
}

const wchar_t* eventLevelToString(proto::system_info::EventLogs::Event::Level lvl)
{
    using L = proto::system_info::EventLogs::Event::Level;
    switch (lvl)
    {
        case L::LEVEL_SUCCESS:       return L"Success";
        case L::LEVEL_INFORMATION:   return L"Information";
        case L::LEVEL_WARNING:       return L"Warning";
        case L::LEVEL_ERROR:         return L"Error";
        case L::LEVEL_AUDIT_SUCCESS: return L"Audit Success";
        case L::LEVEL_AUDIT_FAILURE: return L"Audit Failure";
        default:                     return L"Unknown";
    }
}

// Control IDs - kept local until they are formally added to resource.h
// (see comment block at top of sys_info_event_logs.h). Reserved range
// for the Event Logs system-info widget: 5100..5199.
constexpr int kIdList = 5100;
constexpr int kIdCombo = 5101;

constexpr int kComboHeight = 24;
constexpr int kComboWidth = 200;
constexpr int kComboSpacing = 5;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kColumns[] = {
    { L"Time",        140 },
    { L"Level",        80 },
    { L"Category",    120 },
    { L"Event ID",     80 },
    { L"Source",      180 },
    { L"Description", 400 },
};

constexpr int kColumnCount = static_cast<int>(sizeof(kColumns) / sizeof(kColumns[0]));

const wchar_t* const kLogTypes[] = {
    L"Application",
    L"System",
    L"Security",
};

constexpr int kLogTypeCount = static_cast<int>(sizeof(kLogTypes) / sizeof(kLogTypes[0]));

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

SysInfoEventLogs::SysInfoEventLogs(HINSTANCE instance)
    : instance_(instance)
{
}

SysInfoEventLogs::~SysInfoEventLogs() = default;

bool SysInfoEventLogs::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &SysInfoEventLogs::windowProc;
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

void SysInfoEventLogs::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK SysInfoEventLogs::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SysInfoEventLogs* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SysInfoEventLogs*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SysInfoEventLogs*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SysInfoEventLogs::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void SysInfoEventLogs::onCreate()
{
    combo_ = CreateWindowExW(
        0, WC_COMBOBOXW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP |
            CBS_DROPDOWNLIST | WS_VSCROLL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCombo)),
        instance_, nullptr);

    if (combo_)
    {
        for (int i = 0; i < kLogTypeCount; ++i)
        {
            SendMessageW(combo_, CB_ADDSTRING, 0,
                         reinterpret_cast<LPARAM>(kLogTypes[i]));
        }
        SendMessageW(combo_, CB_SETCURSEL, 0, 0);
    }

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

void SysInfoEventLogs::onSize(int width, int height)
{
    if (combo_)
    {
        const int x = width - kComboWidth;
        SetWindowPos(combo_, nullptr, x > 0 ? x : 0, 0,
                     kComboWidth, kComboHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (list_)
    {
        const int top = kComboHeight + kComboSpacing;
        const int listHeight = height - top;
        SetWindowPos(list_, nullptr, 0, top, width,
                     listHeight > 0 ? listHeight : 0,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SysInfoEventLogs::setEvents(const std::vector<EventEntry>& events)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    for (size_t i = 0; i < events.size(); ++i)
    {
        const EventEntry& e = events[i];

        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(e.time.c_str());

        const int row = ListView_InsertItem(list_, &item);
        if (row < 0)
            continue;

        setSubItem(list_, row, 1, e.level);
        setSubItem(list_, row, 2, e.category);
        setSubItem(list_, row, 3, e.eventId);
        setSubItem(list_, row, 4, e.source);
        setSubItem(list_, row, 5, e.description);
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

std::wstring SysInfoEventLogs::getSelectedLog() const
{
    if (!combo_)
        return std::wstring();

    const LRESULT index = SendMessageW(combo_, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR)
        return std::wstring();

    const LRESULT len = SendMessageW(combo_, CB_GETLBTEXTLEN, index, 0);
    if (len <= 0)
        return std::wstring();

    std::wstring result(static_cast<size_t>(len), L'\0');
    SendMessageW(combo_, CB_GETLBTEXT, index,
                 reinterpret_cast<LPARAM>(result.data()));
    return result;
}

void SysInfoEventLogs::setFromProto(const proto::system_info::SystemInfo& si)
{
    if (!si.has_event_logs()) return;
    std::vector<EventEntry> entries;
    for (const auto& e : si.event_logs().event())
    {
        EventEntry entry;
        entry.time        = formatTimestamp(e.time());
        entry.level       = eventLevelToString(e.level());
        entry.category    = toWide(e.category());
        entry.source      = toWide(e.source());
        entry.description = toWide(e.description());

        wchar_t idBuf[16];
        swprintf(idBuf, 16, L"%u", e.event_id());
        entry.eventId = idBuf;

        entries.push_back(std::move(entry));
    }
    setEvents(entries);
}

}  // namespace aspia::client_win32
