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

#include "client_win32/router_manager_window.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Resource IDs - kept local to this TU until they are formally added to
// resource.h (see comment block at top of router_manager_window.h).
constexpr int kIdTabs         = 4000;
constexpr int kIdListUsers    = 4001;
constexpr int kIdListHosts    = 4002;
constexpr int kIdListActive   = 4003;
constexpr int kIdStatusBar    = 4004;
constexpr int kIdToolbar      = 4005;
constexpr int kIdAdd          = 4010;
constexpr int kIdEdit         = 4011;
constexpr int kIdRemove       = 4012;
constexpr int kIdRefresh      = 4013;
constexpr int kIdDisconnect   = 4014;
constexpr int kIdRefreshAct   = 4015;
constexpr int kIdClose        = 4016;

// Height reserved for the toolbar / button row above the tab control.
constexpr int kToolbarHeight  = 32;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kUsersColumns[] = {
    { L"Name",     200 },
    { L"Sessions", 100 },
    { L"Active",   100 },
};

const Column kHostsColumns[] = {
    { L"ID",         120 },
    { L"IP Address", 140 },
    { L"Version",    100 },
    { L"User Agent", 200 },
    { L"Connected",  160 },
};

const Column kActiveColumns[] = {
    { L"Host ID",      120 },
    { L"Client IP",    140 },
    { L"Started At",   160 },
    { L"Duration",     100 },
    { L"Session Type", 140 },
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

RouterManagerWindow::RouterManagerWindow(HINSTANCE instance)
    : instance_(instance)
{
}

RouterManagerWindow::~RouterManagerWindow()
{
    if (accel_)
        DestroyAcceleratorTable(accel_);
}

bool RouterManagerWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &RouterManagerWindow::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"Router Management",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 550,
        nullptr, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void RouterManagerWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool RouterManagerWindow::preTranslateMessage(MSG* msg)
{
    if (accel_ && TranslateAcceleratorW(hwnd_, accel_, msg))
        return true;
    return false;
}

// static
LRESULT CALLBACK RouterManagerWindow::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    RouterManagerWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<RouterManagerWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<RouterManagerWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT RouterManagerWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void RouterManagerWindow::onCreate()
{
    createToolbar();
    createTabs();
    createLists();
    createStatusBar();

    showTab(kTabUsers);
}

void RouterManagerWindow::createToolbar()
{
    // Simple button row. Kept as plain BS_PUSHBUTTON children rather than a
    // TB_ toolbar so icons (.svg from the .ui) are not required at this stage;
    // a real toolbar can be swapped in once the Win32 image list is populated.
    toolbar_ = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdToolbar)),
        instance_, nullptr);

    struct ButtonInfo
    {
        const wchar_t* text;
        int id;
        int width;
    };

    const ButtonInfo kButtons[] = {
        { L"Add",            kIdAdd,        70 },
        { L"Edit",           kIdEdit,       70 },
        { L"Remove",         kIdRemove,     70 },
        { L"Refresh",        kIdRefresh,    80 },
        { L"Disconnect",     kIdDisconnect, 90 },
        { L"Refresh Active", kIdRefreshAct, 110 },
        { L"Close",          kIdClose,      70 },
    };

    int x = 4;
    for (const ButtonInfo& b : kButtons)
    {
        CreateWindowExW(
            0, L"BUTTON", b.text,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            x, 4, b.width, kToolbarHeight - 8, hwnd_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(b.id)),
            instance_, nullptr);
        x += b.width + 4;
    }
}

void RouterManagerWindow::createTabs()
{
    tabs_ = CreateWindowExW(
        0, WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FOCUSNEVER,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTabs)),
        instance_, nullptr);

    struct TabInfo { const wchar_t* title; };
    const TabInfo kTabs[kTabCount] = {
        { L"Users"              },
        { L"Hosts"              },
        { L"Active Connections" },
    };

    for (int i = 0; i < kTabCount; ++i)
    {
        TCITEMW item = {};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<wchar_t*>(kTabs[i].title);
        TabCtrl_InsertItem(tabs_, i, &item);
    }
}

void RouterManagerWindow::createLists()
{
    const DWORD style = WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER |
                        LVS_REPORT | LVS_SHOWSELALWAYS;
    const int kIds[kTabCount] = {
        kIdListUsers, kIdListHosts, kIdListActive
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

    addColumns(lists_[kTabUsers],  kUsersColumns,
               static_cast<int>(sizeof(kUsersColumns) / sizeof(kUsersColumns[0])));
    addColumns(lists_[kTabHosts],  kHostsColumns,
               static_cast<int>(sizeof(kHostsColumns) / sizeof(kHostsColumns[0])));
    addColumns(lists_[kTabActive], kActiveColumns,
               static_cast<int>(sizeof(kActiveColumns) / sizeof(kActiveColumns[0])));
}

void RouterManagerWindow::createStatusBar()
{
    status_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusBar)),
        instance_, nullptr);
}

void RouterManagerWindow::onSize(int width, int height)
{
    if (status_)
        SendMessageW(status_, WM_SIZE, 0, 0);

    layoutChildren(width, height);
}

void RouterManagerWindow::layoutChildren(int width, int height)
{
    int statusHeight = 0;
    if (status_)
    {
        RECT rc = {};
        GetWindowRect(status_, &rc);
        statusHeight = rc.bottom - rc.top;
    }

    if (toolbar_)
    {
        SetWindowPos(toolbar_, nullptr, 0, 0, width, kToolbarHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    const int tabsTop    = kToolbarHeight;
    const int tabsHeight = height - kToolbarHeight - statusHeight;

    if (tabs_)
    {
        SetWindowPos(tabs_, nullptr, 0, tabsTop, width, tabsHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (!tabs_)
        return;

    // Compute the tab control's display area (inside the tab strip).
    RECT rc = { 0, 0, width, tabsHeight };
    TabCtrl_AdjustRect(tabs_, FALSE, &rc);

    for (int i = 0; i < kTabCount; ++i)
    {
        if (!lists_[i])
            continue;
        SetWindowPos(lists_[i], HWND_TOP,
                     rc.left, tabsTop + rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOACTIVATE);
    }
}

void RouterManagerWindow::onNotify(LPARAM lp)
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

void RouterManagerWindow::showTab(int index)
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

void RouterManagerWindow::onCommand(int id)
{
    switch (id)
    {
        case kIdAdd:
            // TODO(phase2): open router_user_dialog to add a new user,
            // then send proto::router_admin::UserRequest{ADD} over IPC.
            break;

        case kIdEdit:
            // TODO(phase2): open router_user_dialog on the selected row,
            // then send proto::router_admin::UserRequest{MODIFY}.
            break;

        case kIdRemove:
            // TODO(phase2): confirm and send proto::router_admin::UserRequest{DELETE}.
            break;

        case kIdRefresh:
            // TODO(phase2): request UserList / HostList for the current tab.
            break;

        case kIdDisconnect:
            // TODO(phase2): send proto::router_admin::DisconnectHost for the
            // selected row in the Hosts list.
            break;

        case kIdRefreshAct:
            // TODO(phase2): request proto::router_admin::ConnectionList refresh.
            break;

        case kIdClose:
            if (hwnd_)
                DestroyWindow(hwnd_);
            break;

        default:
            break;
    }
}

void RouterManagerWindow::onDestroy()
{
    // Top-level, non-modal window: do NOT post WM_QUIT here. The owning
    // application decides whether closing this window exits the process.
}

// ---------------------------------------------------------------------------
// Data-loading stubs. Real values will arrive from the router admin session
// over IPC/proto (see proto/router_admin.proto) once that path is wired up.
// ---------------------------------------------------------------------------

void RouterManagerWindow::reloadUsers(std::span<const RouterUserRow> /*rows*/)
{
    // TODO(phase2): clear lists_[kTabUsers] and re-insert one LVITEM per row
    // (Name, Sessions, Active) from the incoming proto::router_admin::UserList.
    if (lists_[kTabUsers])
        ListView_DeleteAllItems(lists_[kTabUsers]);
}

void RouterManagerWindow::reloadHosts(std::span<const RouterHostRow> /*rows*/)
{
    // TODO(phase2): clear lists_[kTabHosts] and re-insert one LVITEM per row
    // (ID, IP Address, Version, User Agent, Connected) from HostList.
    if (lists_[kTabHosts])
        ListView_DeleteAllItems(lists_[kTabHosts]);
}

void RouterManagerWindow::reloadActive(std::span<const RouterActiveRow> /*rows*/)
{
    // TODO(phase2): clear lists_[kTabActive] and re-insert one LVITEM per row
    // (Host ID, Client IP, Started At, Duration, Session Type) from ConnectionList.
    if (lists_[kTabActive])
        ListView_DeleteAllItems(lists_[kTabActive]);
}

}  // namespace aspia::client_win32
