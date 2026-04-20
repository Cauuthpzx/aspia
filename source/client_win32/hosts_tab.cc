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

#include "client_win32/hosts_tab.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Control IDs - kept local to this TU until formally added to resource.h
// (see comment block at the top of hosts_tab.h).
constexpr int kIdSearch = 4300;
constexpr int kIdTree   = 4301;
constexpr int kIdList   = 4302;

// Layout constants.
constexpr int kSearchHeight = 24;
constexpr int kPadding      = 4;
constexpr int kSplitNum     = 30;   // left pane = 30% of width
constexpr int kSplitDen     = 100;

struct Column
{
    const wchar_t* text;
    int width;
};

const Column kComputerColumns[] = {
    { L"Name",    200 },
    { L"Address", 160 },
    { L"User",    140 },
    { L"Comment", 260 },
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

HostsTab::HostsTab(HINSTANCE instance)
    : instance_(instance)
{
}

HostsTab::~HostsTab() = default;

bool HostsTab::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &HostsTab::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        parent, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void HostsTab::move(int x, int y, int width, int height)
{
    if (!hwnd_)
        return;
    SetWindowPos(hwnd_, nullptr, x, y, width, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK HostsTab::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostsTab* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<HostsTab*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<HostsTab*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT HostsTab::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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
            onCommand(wp);
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void HostsTab::onCreate()
{
    createChildren();

    // Populate initial (empty) data. Real data arrives later.
    populateGroups();
    populateComputers();
}

void HostsTab::createChildren()
{
    // Search edit (top of the left pane).
    search_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdSearch)),
        instance_, nullptr);

    // Group tree (SysTreeView32) on the left, below the search box.
    tree_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTree)),
        instance_, nullptr);

    // Computer list (SysListView32, LVS_REPORT) on the right.
    list_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            LVS_REPORT | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdList)),
        instance_, nullptr);

    ListView_SetExtendedListViewStyle(
        list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

    addColumns(list_, kComputerColumns,
               static_cast<int>(sizeof(kComputerColumns) / sizeof(kComputerColumns[0])));
}

void HostsTab::onSize(int width, int height)
{
    layoutChildren(width, height);
}

void HostsTab::layoutChildren(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    // Left pane = 30%, right pane = 70% (per task spec).
    const int leftWidth = (width * kSplitNum) / kSplitDen;
    const int rightX    = leftWidth + kPadding;
    const int rightWidth = width - rightX;

    if (search_)
    {
        SetWindowPos(search_, nullptr,
                     0, 0, leftWidth, kSearchHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    const int treeY = kSearchHeight + kPadding;
    const int treeHeight = height - treeY;
    if (tree_)
    {
        SetWindowPos(tree_, nullptr,
                     0, treeY, leftWidth, treeHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (list_ && rightWidth > 0)
    {
        SetWindowPos(list_, nullptr,
                     rightX, 0, rightWidth, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void HostsTab::onNotify(LPARAM lp)
{
    auto* hdr = reinterpret_cast<NMHDR*>(lp);
    if (!hdr)
        return;

    if (hdr->hwndFrom == tree_ && hdr->code == TVN_SELCHANGEDW)
    {
        onTreeSelectionChanged(reinterpret_cast<NMTREEVIEWW*>(lp));
    }
}

void HostsTab::onCommand(WPARAM wp)
{
    const int id = LOWORD(wp);
    const int code = HIWORD(wp);

    if (id == kIdSearch && code == EN_CHANGE)
    {
        // TODO(phase2): filter tree_/list_ against the search edit's text.
        return;
    }
}

void HostsTab::onTreeSelectionChanged(NMTREEVIEWW* /*nm*/)
{
    // TODO(phase2): look up the selected group and repopulate the computer
    // list from the address-book model.
    populateComputers();
}

void HostsTab::onDestroy()
{
    // Child window: nothing to post. The parent owns lifetime.
}

// ---------------------------------------------------------------------------
// Data-loading stubs. Real values will arrive from the address-book model
// (see console/address_book_tab.cc for the legacy Qt path) once that wiring
// is in place (phase 2).
// ---------------------------------------------------------------------------

void HostsTab::populateGroups()
{
    // TODO(phase2): walk proto::address_book::ComputerGroup tree and
    // TreeView_InsertItem for each node.
}

void HostsTab::populateComputers()
{
    // TODO(phase2): iterate the selected group's computers and
    // ListView_InsertItem / ListView_SetItemText for each column
    // (Name, Address, User, Comment).
}

}  // namespace aspia::client_win32
