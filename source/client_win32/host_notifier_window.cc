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

#include "client_win32/host_notifier_window.h"

#include "client_win32/resource.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Resource IDs - kept local to this TU until they are formally added to
// resource.h (see comment block at top of host_notifier_window.h).
constexpr int kIdTitle        = 6400;
constexpr int kIdCollapse     = 6401;
constexpr int kIdExpand       = 6402;
constexpr int kIdSessions     = 6403;
constexpr int kIdLockMouse    = 6410;
constexpr int kIdLockKbd      = 6411;
constexpr int kIdPause        = 6412;
constexpr int kIdDisconnect   = 6413;
// Per-row "disconnect" pseudo-IDs. LVN_ITEMACTIVATE on the listview is how
// the user actually asks to drop a given session, but a separate range is
// reserved here for a future custom-draw button-column layout.
constexpr int kIdFirstDiscon  = 6450;
constexpr int kIdLastDiscon   = 6499;
(void)kIdFirstDiscon;
(void)kIdLastDiscon;

// Layout constants. The notifier is a compact floating panel; the numbers
// below mirror the spacing used by the Qt .ui (3px margins, 26px buttons).
constexpr int kMargin         = 3;
constexpr int kHeaderHeight   = 26;
constexpr int kToolbarHeight  = 30;
constexpr int kButtonWidth    = 28;
constexpr int kDefaultWidth   = 267;
constexpr int kDefaultHeight  = 176;
constexpr int kCollapsedWidth = 34;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kSessionColumns[] = {
    { L"User",   110 },
    { L"Type",   110 },
    { L"Action",  40 },
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

HostNotifierWindow::HostNotifierWindow(HINSTANCE instance, HWND notifyParent)
    : instance_(instance), notifyParent_(notifyParent)
{
}

HostNotifierWindow::~HostNotifierWindow()
{
    if (accel_)
        DestroyAcceleratorTable(accel_);
}

bool HostNotifierWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &HostNotifierWindow::windowProc;
    wc.hInstance     = instance_;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    // WS_POPUP | WS_CLIPCHILDREN: borderless floating panel.
    // WS_EX_TOPMOST | WS_EX_TOOLWINDOW: always on top and hidden from taskbar.
    hwnd_ = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        kClassName, L"Aspia Notifier",
        WS_POPUP | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, kDefaultWidth, kDefaultHeight,
        nullptr, nullptr, instance_, this);

    if (!hwnd_)
        return false;

    accel_ = LoadAcceleratorsW(instance_,
                               MAKEINTRESOURCEW(IDR_HOST_NOTIFIER_ACCEL));
    return true;
}

void HostNotifierWindow::show(int showCmd)
{
    positionInitially();
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool HostNotifierWindow::preTranslateMessage(MSG* msg)
{
    if (accel_ && TranslateAcceleratorW(hwnd_, accel_, msg))
        return true;
    return false;
}

void HostNotifierWindow::setSessions(const std::vector<Session>& sessions)
{
    sessions_data_ = sessions;
    refreshListView();
}

// static
LRESULT CALLBACK HostNotifierWindow::windowProc(HWND hwnd, UINT msg,
                                                WPARAM wp, LPARAM lp)
{
    HostNotifierWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<HostNotifierWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<HostNotifierWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT HostNotifierWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_SIZE:
            onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        case WM_COMMAND:
            onCommand(LOWORD(wp));
            return 0;

        case WM_NOTIFY:
            onNotify(lp);
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void HostNotifierWindow::onCreate()
{
    createChildren();
    refreshListView();
}

void HostNotifierWindow::createChildren()
{
    const DWORD kBtnStyle = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
    const DWORD kHiddenBtnStyle = WS_CHILD | BS_PUSHBUTTON;

    // Header: title label (rich text in Qt; plain static here).
    titleLabel_ = CreateWindowExW(
        0, L"STATIC", L"Active sessions",
        WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTitle)),
        instance_, nullptr);

    // Collapse button (shown when expanded). ">" arrow.
    collapseBtn_ = CreateWindowExW(
        0, L"BUTTON", L">",
        kBtnStyle,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdCollapse)),
        instance_, nullptr);

    // Expand button (shown when collapsed). "<" arrow. Hidden by default.
    expandBtn_ = CreateWindowExW(
        0, L"BUTTON", L"<",
        kHiddenBtnStyle,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdExpand)),
        instance_, nullptr);

    // Session list.
    sessions_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT |
            LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdSessions)),
        instance_, nullptr);

    ListView_SetExtendedListViewStyle(
        sessions_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    addColumns(sessions_, kSessionColumns,
               static_cast<int>(sizeof(kSessionColumns) / sizeof(kSessionColumns[0])));

    // Toolbar buttons along the bottom.
    lockMouseBtn_ = CreateWindowExW(
        0, L"BUTTON", L"M",
        kBtnStyle, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdLockMouse)),
        instance_, nullptr);

    lockKbdBtn_ = CreateWindowExW(
        0, L"BUTTON", L"K",
        kBtnStyle, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdLockKbd)),
        instance_, nullptr);

    pauseBtn_ = CreateWindowExW(
        0, L"BUTTON", L"||",
        kBtnStyle, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdPause)),
        instance_, nullptr);

    // The big "disconnect all" action.
    disconnectBtn_ = CreateWindowExW(
        0, L"BUTTON", L"Disconnect all",
        kBtnStyle, 0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdDisconnect)),
        instance_, nullptr);
}

void HostNotifierWindow::onSize(int width, int height)
{
    layoutChildren(width, height);
}

void HostNotifierWindow::layoutChildren(int width, int height)
{
    if (collapsed_)
    {
        // Collapsed: only the "expand" tab is visible.
        if (expandBtn_)
        {
            SetWindowPos(expandBtn_, nullptr,
                         0, 0, width, kHeaderHeight,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        ShowWindow(titleLabel_,    SW_HIDE);
        ShowWindow(collapseBtn_,   SW_HIDE);
        ShowWindow(sessions_,      SW_HIDE);
        ShowWindow(lockMouseBtn_,  SW_HIDE);
        ShowWindow(lockKbdBtn_,    SW_HIDE);
        ShowWindow(pauseBtn_,      SW_HIDE);
        ShowWindow(disconnectBtn_, SW_HIDE);
        return;
    }

    ShowWindow(expandBtn_, SW_HIDE);

    // Header row.
    const int headerY = kMargin;
    const int headerX = kMargin;
    const int headerW = width - 2 * kMargin;

    SetWindowPos(collapseBtn_, nullptr,
                 headerX, headerY, kButtonWidth, kHeaderHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(collapseBtn_, SW_SHOW);

    const int titleX = headerX + kButtonWidth + kMargin;
    const int titleW = headerW - kButtonWidth - kMargin;
    SetWindowPos(titleLabel_, nullptr,
                 titleX, headerY + 4, titleW, kHeaderHeight - 4,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(titleLabel_, SW_SHOW);

    // Toolbar row (bottom).
    const int toolbarY = height - kMargin - kToolbarHeight;
    int x = width - kMargin;

    // Right-aligned toolbar: disconnect, pause, kbd, mouse (right to left).
    const int kBigBtnWidth = 110;
    x -= kBigBtnWidth;
    SetWindowPos(disconnectBtn_, nullptr,
                 x, toolbarY, kBigBtnWidth, kToolbarHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(disconnectBtn_, SW_SHOW);

    x -= kMargin + kButtonWidth;
    SetWindowPos(pauseBtn_, nullptr,
                 x, toolbarY, kButtonWidth, kToolbarHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(pauseBtn_, SW_SHOW);

    x -= kMargin + kButtonWidth;
    SetWindowPos(lockKbdBtn_, nullptr,
                 x, toolbarY, kButtonWidth, kToolbarHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(lockKbdBtn_, SW_SHOW);

    x -= kMargin + kButtonWidth;
    SetWindowPos(lockMouseBtn_, nullptr,
                 x, toolbarY, kButtonWidth, kToolbarHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(lockMouseBtn_, SW_SHOW);

    // Session listview fills the middle area.
    const int listY = headerY + kHeaderHeight + kMargin;
    const int listH = toolbarY - listY - kMargin;
    SetWindowPos(sessions_, nullptr,
                 kMargin, listY,
                 width - 2 * kMargin, listH > 0 ? listH : 0,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    ShowWindow(sessions_, SW_SHOW);
}

void HostNotifierWindow::refreshListView()
{
    if (!sessions_)
        return;

    ListView_DeleteAllItems(sessions_);

    for (size_t i = 0; i < sessions_data_.size(); ++i)
    {
        const Session& s = sessions_data_[i];

        LVITEMW item = {};
        item.mask     = LVIF_TEXT | LVIF_PARAM;
        item.iItem    = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText  = const_cast<wchar_t*>(s.user_name.c_str());
        item.lParam   = static_cast<LPARAM>(s.session_id);
        const int row = ListView_InsertItem(sessions_, &item);

        ListView_SetItemText(sessions_, row, 1,
                             const_cast<wchar_t*>(s.session_type.c_str()));
        ListView_SetItemText(sessions_, row, 2, const_cast<wchar_t*>(L"X"));
    }
}

void HostNotifierWindow::positionInitially()
{
    // Anchor the notifier to the upper-right corner of the primary work area.
    RECT work = {};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);

    const int w = kDefaultWidth;
    const int h = kDefaultHeight;
    const int x = work.right - w - 8;
    const int y = work.top + 8;

    SetWindowPos(hwnd_, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE);
}

void HostNotifierWindow::toggleCollapsed()
{
    collapsed_ = !collapsed_;

    RECT rc = {};
    GetWindowRect(hwnd_, &rc);

    const int newW = collapsed_ ? kCollapsedWidth : kDefaultWidth;
    const int newH = collapsed_ ? kHeaderHeight + 2 * kMargin : kDefaultHeight;

    // Keep the right edge pinned so the panel grows/shrinks from the left.
    const int newX = rc.right - newW;
    const int newY = rc.top;

    SetWindowPos(hwnd_, HWND_TOPMOST, newX, newY, newW, newH,
                 SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void HostNotifierWindow::onCommand(int id)
{
    switch (id)
    {
        case kIdCollapse:
        case kIdExpand:
            toggleCollapsed();
            return;

        case kIdLockMouse:
            postToParent(kNotifyLockMouse, 0);
            return;

        case kIdLockKbd:
            postToParent(kNotifyLockKeyboard, 0);
            return;

        case kIdPause:
            postToParent(kNotifyPause, 0);
            return;

        case kIdDisconnect:
            postToParent(kNotifyDisconnectAll, 0);
            return;

        default:
            break;
    }
}

void HostNotifierWindow::onNotify(LPARAM lp)
{
    auto* hdr = reinterpret_cast<NMHDR*>(lp);
    if (!hdr || hdr->hwndFrom != sessions_)
        return;

    // Double-click or Enter on a row requests per-session disconnect.
    if (hdr->code == NM_DBLCLK || hdr->code == LVN_ITEMACTIVATE)
    {
        auto* act = reinterpret_cast<NMITEMACTIVATE*>(lp);
        if (!act || act->iItem < 0)
            return;

        LVITEMW item = {};
        item.mask     = LVIF_PARAM;
        item.iItem    = act->iItem;
        item.iSubItem = 0;
        if (!ListView_GetItem(sessions_, &item))
            return;

        postToParent(kNotifyDisconnectSession, item.lParam);
    }
}

void HostNotifierWindow::postToParent(UINT code, LPARAM lp)
{
    if (!notifyParent_)
        return;
    PostMessageW(notifyParent_, code, 0, lp);
}

void HostNotifierWindow::onDestroy()
{
    // Top-level, non-modal window: do NOT post WM_QUIT here. The owning
    // application decides whether closing this window exits the process.
}

}  // namespace aspia::client_win32
