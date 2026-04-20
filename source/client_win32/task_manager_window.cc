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

#include "client_win32/task_manager_window.h"

#include "client_win32/resource.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Resource IDs - kept local to this TU until they are formally added to
// resource.h (see comment block at top of task_manager_window.h).
constexpr int kIdTabs         = 2900;
constexpr int kIdListProcess  = 2901;
constexpr int kIdListService  = 2902;
constexpr int kIdListUsers    = 2903;
constexpr int kIdListPerf     = 2904;
constexpr int kIdStatusBar    = 2905;
constexpr int kIdEndTask      = 2910;
constexpr int kIdDisconnect   = 2911;
constexpr int kIdLogoff       = 2912;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kProcessColumns[] = {
    { L"Name",                         160 },
    { L"Process ID",                    80 },
    { L"Session ID",                    80 },
    { L"User Name",                    120 },
    { L"CPU",                           60 },
    { L"Memory (private working set)", 160 },
    { L"Memory (working set)",         140 },
    { L"Memory (peak working set)",    160 },
    { L"Memory (working set delta)",   160 },
    { L"Threads",                       70 },
    { L"Image Path",                   220 },
};

const Column kServiceColumns[] = {
    { L"Name",         180 },
    { L"Status",       100 },
    { L"Startup type", 100 },
    { L"Description",  280 },
};

const Column kUserColumns[] = {
    { L"Name",         140 },
    { L"ID",            80 },
    { L"Status",       100 },
    { L"Client name",  140 },
    { L"Session name", 140 },
};

const Column kPerfColumns[] = {
    { L"Counter", 220 },
    { L"Value",   160 },
};

void addColumns(HWND list, const Column* cols, int count)
{
    for (int i = 0; i < count; ++i)
    {
        LVCOLUMNW col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = const_cast<wchar_t*>(cols[i].text);
        col.cx = cols[i].width;
        col.iSubItem = i;
        ListView_InsertColumn(list, i, &col);
    }
}

}  // namespace

TaskManagerWindow::TaskManagerWindow(HINSTANCE instance)
    : instance_(instance)
{
}

TaskManagerWindow::~TaskManagerWindow()
{
    if (accel_)
        DestroyAcceleratorTable(accel_);
}

bool TaskManagerWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &TaskManagerWindow::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"Task Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void TaskManagerWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool TaskManagerWindow::preTranslateMessage(MSG* msg)
{
    if (accel_ && TranslateAcceleratorW(hwnd_, accel_, msg))
        return true;
    return false;
}

// static
LRESULT CALLBACK TaskManagerWindow::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    TaskManagerWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<TaskManagerWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<TaskManagerWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT TaskManagerWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_SIZE:
            onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        case WM_NOTIFY:
            onNotify(lp);
            return 0;

        case WM_COMMAND:
            onCommand(LOWORD(wp));
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void TaskManagerWindow::onCreate()
{
    createTabs();
    createLists();
    createStatusBar();

    showTab(kTabProcesses);

    // Populate initial (empty) rows. Real data arrives later via IPC.
    populateProcesses();
    populateServices();
    populateUsers();
    populatePerformance();
}

void TaskManagerWindow::createTabs()
{
    tabs_ = CreateWindowExW(
        0, WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FOCUSNEVER,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTabs)),
        instance_, nullptr);

    struct TabInfo { const wchar_t* title; };
    const TabInfo kTabs[kTabCount] = {
        { L"Processes"   },
        { L"Services"    },
        { L"Users"       },
        { L"Performance" },
    };

    for (int i = 0; i < kTabCount; ++i)
    {
        TCITEMW item = {};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<wchar_t*>(kTabs[i].title);
        TabCtrl_InsertItem(tabs_, i, &item);
    }
}

void TaskManagerWindow::createLists()
{
    const DWORD style = WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER |
                        LVS_REPORT | LVS_SHOWSELALWAYS;
    const int kIds[kTabCount] = {
        kIdListProcess, kIdListService, kIdListUsers, kIdListPerf
    };

    for (int i = 0; i < kTabCount; ++i)
    {
        lists_[i] = CreateWindowExW(
            WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
            style, 0, 0, 0, 0, hwnd_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIds[i])),
            instance_, nullptr);

        ListView_SetExtendedListViewStyle(
            lists_[i], LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
    }

    addColumns(lists_[kTabProcesses],   kProcessColumns,
               static_cast<int>(sizeof(kProcessColumns) / sizeof(kProcessColumns[0])));
    addColumns(lists_[kTabServices],    kServiceColumns,
               static_cast<int>(sizeof(kServiceColumns) / sizeof(kServiceColumns[0])));
    addColumns(lists_[kTabUsers],       kUserColumns,
               static_cast<int>(sizeof(kUserColumns) / sizeof(kUserColumns[0])));
    addColumns(lists_[kTabPerformance], kPerfColumns,
               static_cast<int>(sizeof(kPerfColumns) / sizeof(kPerfColumns[0])));
}

void TaskManagerWindow::createStatusBar()
{
    status_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusBar)),
        instance_, nullptr);
}

void TaskManagerWindow::onSize(int width, int height)
{
    if (status_)
        SendMessageW(status_, WM_SIZE, 0, 0);

    RECT statusRect = {};
    int statusHeight = 0;
    if (status_)
    {
        GetWindowRect(status_, &statusRect);
        statusHeight = statusRect.bottom - statusRect.top;
    }

    const int usableHeight = height - statusHeight;

    if (tabs_)
    {
        SetWindowPos(tabs_, nullptr, 0, 0, width, usableHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    layoutLists(width, usableHeight);
}

void TaskManagerWindow::layoutLists(int width, int height)
{
    if (!tabs_)
        return;

    // Compute the tab control's display area (inside the tab strip).
    RECT rc = { 0, 0, width, height };
    TabCtrl_AdjustRect(tabs_, FALSE, &rc);

    for (int i = 0; i < kTabCount; ++i)
    {
        if (!lists_[i])
            continue;
        SetWindowPos(lists_[i], HWND_TOP,
                     rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOACTIVATE);
    }
}

void TaskManagerWindow::onNotify(LPARAM lp)
{
    auto* hdr = reinterpret_cast<NMHDR*>(lp);
    if (!hdr)
        return;

    if (hdr->hwndFrom == tabs_ && hdr->code == TCN_SELCHANGE)
    {
        const int sel = TabCtrl_GetCurSel(tabs_);
        if (sel >= 0 && sel < kTabCount)
            showTab(sel);
    }
}

void TaskManagerWindow::showTab(int index)
{
    if (index < 0 || index >= kTabCount)
        return;

    currentTab_ = index;
    for (int i = 0; i < kTabCount; ++i)
    {
        if (!lists_[i])
            continue;
        ShowWindow(lists_[i], (i == index) ? SW_SHOW : SW_HIDE);
    }

    if (lists_[index])
    {
        BringWindowToTop(lists_[index]);
        SetFocus(lists_[index]);
    }
}

void TaskManagerWindow::onCommand(int id)
{
    switch (id)
    {
        case kIdEndTask:
            // TODO(phase2): send proto::TaskManager::EndProcess over IPC.
            break;

        case kIdDisconnect:
            // TODO(phase2): send proto::TaskManager::DisconnectUser.
            break;

        case kIdLogoff:
            // TODO(phase2): send proto::TaskManager::LogoffUser.
            break;

        default:
            break;
    }
}

void TaskManagerWindow::onDestroy()
{
    // Top-level, non-modal window: do NOT post WM_QUIT here. The owning
    // application decides whether closing this window exits the process.
}

// ---------------------------------------------------------------------------
// Data-loading stubs. Real values will arrive from host/win/task_manager.*
// via IPC/proto messages once that path is wired up (phase 2).
// ---------------------------------------------------------------------------

void TaskManagerWindow::populateProcesses()
{
    // TODO(phase2): iterate proto::TaskManager::ProcessList entries and
    // ListView_InsertItem / ListView_SetItemText for each column.
}

void TaskManagerWindow::populateServices()
{
    // TODO(phase2): iterate proto::TaskManager::ServiceList entries.
}

void TaskManagerWindow::populateUsers()
{
    // TODO(phase2): iterate proto::TaskManager::SessionList entries.
}

void TaskManagerWindow::populatePerformance()
{
    // TODO(phase2): consume proto::TaskManager::Performance snapshot.
}

}  // namespace aspia::client_win32
