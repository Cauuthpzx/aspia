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

namespace aspia::client_win32 {

namespace {

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

}  // namespace aspia::client_win32
