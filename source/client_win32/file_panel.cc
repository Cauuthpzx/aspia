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

#include "client_win32/file_panel.h"

#include <commctrl.h>

#include <algorithm>
#include <array>

namespace aspia::client_win32 {

namespace {

constexpr int kMargin         = 2;
constexpr int kSpacing        = 1;
constexpr int kLabelHeight    = 20;
constexpr int kAddressHeight  = 22;
constexpr int kDriveWidth     = 80;
constexpr int kToolbarHeight  = 28;
constexpr int kStatusHeight   = 18;

struct ToolbarButtonDef
{
    int id;
    const wchar_t* text;
};

constexpr std::array<ToolbarButtonDef, 8> kToolbarButtons = {{
    { FilePanel::kIdBtnUp,       L"Up" },
    { FilePanel::kIdBtnRefresh,  L"Refresh" },
    { FilePanel::kIdBtnHome,     L"Home" },
    { FilePanel::kIdBtnAddDir,   L"New Folder" },
    { FilePanel::kIdBtnDelete,   L"Delete" },
    { FilePanel::kIdBtnSend,     L"Send" },
    { FilePanel::kIdBtnUpload,   L"Upload" },
    { FilePanel::kIdBtnDownload, L"Download" },
}};

std::wstring formatSize(int64_t size)
{
    wchar_t buf[32] = {};
    if (size < 1024)
        swprintf_s(buf, L"%lld B", static_cast<long long>(size));
    else if (size < 1024 * 1024)
        swprintf_s(buf, L"%.1f KB", static_cast<double>(size) / 1024.0);
    else if (size < 1024LL * 1024 * 1024)
        swprintf_s(buf, L"%.1f MB", static_cast<double>(size) / (1024.0 * 1024.0));
    else
        swprintf_s(buf, L"%.2f GB", static_cast<double>(size) / (1024.0 * 1024.0 * 1024.0));
    return buf;
}

std::wstring formatModified(const FILETIME& ft)
{
    SYSTEMTIME st = {};
    FILETIME local = {};
    if (!FileTimeToLocalFileTime(&ft, &local) || !FileTimeToSystemTime(&local, &st))
        return {};

    wchar_t buf[64] = {};
    swprintf_s(buf, L"%04u-%02u-%02u %02u:%02u",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    return buf;
}

}  // namespace

FilePanel::FilePanel(HWND parent, HINSTANCE instance)
    : parent_(parent), instance_(instance)
{
}

FilePanel::~FilePanel()
{
    if (toolbarImages_)
        ImageList_Destroy(toolbarImages_);
}

bool FilePanel::create()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &FilePanel::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        WS_EX_CONTROLPARENT,
        kClassName, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        parent_, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void FilePanel::show(bool visible)
{
    if (hwnd_)
        ShowWindow(hwnd_, visible ? SW_SHOW : SW_HIDE);
}

void FilePanel::setPath(const std::wstring& path)
{
    if (addressEdit_)
        SetWindowTextW(addressEdit_, path.c_str());
}

void FilePanel::setPanelName(const std::wstring& name)
{
    if (nameLabel_)
        SetWindowTextW(nameLabel_, name.c_str());
}

void FilePanel::setStatusText(const std::wstring& text)
{
    if (statusLabel_)
        SetWindowTextW(statusLabel_, text.c_str());
}

void FilePanel::reloadList(std::span<const Entry> entries)
{
    if (!list_)
        return;

    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);

    int index = 0;
    for (const Entry& entry : entries)
    {
        LVITEMW item = {};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = index;
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(entry.name.c_str());
        item.lParam = entry.is_dir ? 1 : 0;

        const int inserted = ListView_InsertItem(list_, &item);
        if (inserted < 0)
            continue;

        const std::wstring sizeText = entry.is_dir ? std::wstring(L"<DIR>") : formatSize(entry.size);
        ListView_SetItemText(list_, inserted, 1, const_cast<LPWSTR>(sizeText.c_str()));

        const wchar_t* typeText = entry.is_dir ? L"File folder" : L"File";
        ListView_SetItemText(list_, inserted, 2, const_cast<LPWSTR>(typeText));

        const std::wstring modText = formatModified(entry.modified);
        ListView_SetItemText(list_, inserted, 3, const_cast<LPWSTR>(modText.c_str()));

        ++index;
    }

    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

// static
LRESULT CALLBACK FilePanel::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    FilePanel* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<FilePanel*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<FilePanel*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT FilePanel::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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
            // Forward to the parent window so the session window can act on
            // toolbar button presses and combo-box selection changes.
            if (parent_)
                SendMessageW(parent_, WM_COMMAND, wp, lp);
            return 0;

        case WM_NOTIFY:
            // Forward list-view / toolbar notifications to the parent too.
            if (parent_)
                return SendMessageW(parent_, WM_NOTIFY, wp, lp);
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void FilePanel::onCreate()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    createChildren();
}

void FilePanel::createChildren()
{
    nameLabel_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_STATICW, L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdNameLabel)),
        instance_, nullptr);

    driveCombo_ = CreateWindowExW(
        0, WC_COMBOBOXW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
            CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        0, 0, 0, 200, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdDriveCombo)),
        instance_, nullptr);

    addressEdit_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_EDITW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdAddressEdit)),
        instance_, nullptr);

    toolbar_ = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_NODIVIDER |
            CCS_NORESIZE | CCS_NOPARENTALIGN,
        0, 0, 0, 0, hwnd_,
        nullptr, instance_, nullptr);

    if (toolbar_)
    {
        SendMessageW(toolbar_, TB_BUTTONSTRUCTSIZE,
                     static_cast<WPARAM>(sizeof(TBBUTTON)), 0);
        SendMessageW(toolbar_, TB_SETEXTENDEDSTYLE, 0,
                     TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);

        std::array<TBBUTTON, kToolbarButtons.size()> buttons = {};
        for (size_t i = 0; i < kToolbarButtons.size(); ++i)
        {
            buttons[i].iBitmap = I_IMAGENONE;
            buttons[i].idCommand = kToolbarButtons[i].id;
            buttons[i].fsState = TBSTATE_ENABLED;
            buttons[i].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT;
            buttons[i].iString = reinterpret_cast<INT_PTR>(kToolbarButtons[i].text);
        }

        SendMessageW(toolbar_, TB_ADDBUTTONSW,
                     static_cast<WPARAM>(buttons.size()),
                     reinterpret_cast<LPARAM>(buttons.data()));
        SendMessageW(toolbar_, TB_AUTOSIZE, 0, 0);
    }

    list_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            LVS_REPORT | LVS_SHOWSELALWAYS | LVS_EDITLABELS,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdList)),
        instance_, nullptr);

    if (list_)
    {
        ListView_SetExtendedListViewStyle(list_,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER |
            LVS_EX_HEADERDRAGDROP);
        populateListColumns();
    }

    statusLabel_ = CreateWindowExW(
        0, WC_STATICW, L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusLabel)),
        instance_, nullptr);
}

void FilePanel::populateListColumns()
{
    struct ColumnDef
    {
        const wchar_t* title;
        int width;
        int fmt;
    };

    const std::array<ColumnDef, 4> columns = {{
        { L"Name",     220, LVCFMT_LEFT  },
        { L"Size",     90,  LVCFMT_RIGHT },
        { L"Type",     110, LVCFMT_LEFT  },
        { L"Modified", 140, LVCFMT_LEFT  },
    }};

    for (int i = 0; i < static_cast<int>(columns.size()); ++i)
    {
        LVCOLUMNW col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
        col.fmt = columns[i].fmt;
        col.cx = columns[i].width;
        col.iSubItem = i;
        col.pszText = const_cast<LPWSTR>(columns[i].title);
        ListView_InsertColumn(list_, i, &col);
    }
}

void FilePanel::onSize(int width, int height)
{
    layoutChildren(width, height);
}

void FilePanel::layoutChildren(int width, int height)
{
    const int innerLeft = kMargin;
    const int innerRight = width - kMargin;
    const int innerWidth = std::max(0, innerRight - innerLeft);

    int y = kMargin;

    if (nameLabel_)
    {
        SetWindowPos(nameLabel_, nullptr,
                     innerLeft, y, innerWidth, kLabelHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    y += kLabelHeight + kSpacing;

    // Address row: drive combo on the left + path edit filling the rest.
    const int driveWidth = std::min(kDriveWidth, innerWidth);
    const int editLeft = innerLeft + driveWidth + kSpacing;
    const int editWidth = std::max(0, innerRight - editLeft);

    if (driveCombo_)
    {
        SetWindowPos(driveCombo_, nullptr,
                     innerLeft, y, driveWidth, kAddressHeight * 8,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    if (addressEdit_)
    {
        SetWindowPos(addressEdit_, nullptr,
                     editLeft, y, editWidth, kAddressHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    y += kAddressHeight + kSpacing;

    if (toolbar_)
    {
        SetWindowPos(toolbar_, nullptr,
                     innerLeft, y, innerWidth, kToolbarHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        SendMessageW(toolbar_, TB_AUTOSIZE, 0, 0);
    }
    y += kToolbarHeight + kSpacing;

    const int statusY = std::max(y, height - kMargin - kStatusHeight);
    const int listBottom = statusY - kSpacing;
    const int listHeight = std::max(0, listBottom - y);

    if (list_)
    {
        SetWindowPos(list_, nullptr,
                     innerLeft, y, innerWidth, listHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (statusLabel_)
    {
        SetWindowPos(statusLabel_, nullptr,
                     innerLeft, statusY, innerWidth, kStatusHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void FilePanel::onDestroy()
{
    // Child controls are destroyed automatically with the parent window.
    nameLabel_ = nullptr;
    driveCombo_ = nullptr;
    addressEdit_ = nullptr;
    toolbar_ = nullptr;
    list_ = nullptr;
    statusLabel_ = nullptr;
}

}  // namespace aspia::client_win32
