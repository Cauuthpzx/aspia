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

#include "client_win32/local_group_widget.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

struct ColDef
{
    const wchar_t* header;
    int            width;
};

constexpr ColDef kColumns[] = {
    { L"Name",    200 },
    { L"Address", 180 },
    { L"Comment", 260 },
};

constexpr int kColCount = static_cast<int>(sizeof(kColumns) / sizeof(kColumns[0]));

void addListColumn(HWND list, int index, const wchar_t* text, int width)
{
    LVCOLUMNW col = {};
    col.mask      = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    col.pszText   = const_cast<wchar_t*>(text);
    col.cx        = width;
    col.iSubItem  = index;
    ListView_InsertColumn(list, index, &col);
}

}  // namespace

LocalGroupWidget::LocalGroupWidget(HINSTANCE instance)
    : instance_(instance)
{
}

LocalGroupWidget::~LocalGroupWidget() = default;

bool LocalGroupWidget::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &LocalGroupWidget::windowProc;
    wc.hInstance     = instance_;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
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

void LocalGroupWidget::move(int x, int y, int width, int height)
{
    if (hwnd_)
        SetWindowPos(hwnd_, nullptr, x, y, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
}

void LocalGroupWidget::setComputers(const std::vector<ComputerEntry>& computers)
{
    if (list_)
        populateList(computers);
}

int LocalGroupWidget::selectedIndex() const
{
    if (!list_)
        return -1;
    return ListView_GetNextItem(list_, -1, LVNI_SELECTED);
}

// static
LRESULT CALLBACK LocalGroupWidget::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    LocalGroupWidget* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<LocalGroupWidget*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<LocalGroupWidget*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT LocalGroupWidget::handleMessage(UINT msg, WPARAM /*wp*/, LPARAM lp)
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
            return DefWindowProcW(hwnd_, msg, 0, lp);
    }
}

void LocalGroupWidget::onCreate()
{
    createList();
}

void LocalGroupWidget::createList()
{
    list_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LOCAL_GROUP_WIDGET_LIST)),
        instance_, nullptr);

    ListView_SetExtendedListViewStyle(
        list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

    for (int i = 0; i < kColCount; ++i)
        addListColumn(list_, i, kColumns[i].header, kColumns[i].width);
}

void LocalGroupWidget::onSize(int width, int height)
{
    if (list_)
        SetWindowPos(list_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
}

void LocalGroupWidget::onDestroy()
{
    // Nothing — child controls are destroyed with the parent window.
}

void LocalGroupWidget::populateList(const std::vector<ComputerEntry>& entries)
{
    ListView_DeleteAllItems(list_);

    int row = 0;
    for (const ComputerEntry& e : entries)
    {
        LVITEMW item  = {};
        item.mask     = LVIF_TEXT;
        item.iItem    = row;
        item.iSubItem = 0;
        item.pszText  = const_cast<wchar_t*>(e.name.c_str());
        ListView_InsertItem(list_, &item);

        ListView_SetItemText(list_, row, 1,
                             const_cast<wchar_t*>(e.address.c_str()));
        ListView_SetItemText(list_, row, 2,
                             const_cast<wchar_t*>(e.comment.c_str()));
        ++row;
    }
}

}  // namespace aspia::client_win32
