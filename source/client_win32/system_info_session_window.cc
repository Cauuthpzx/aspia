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

#include "client_win32/system_info_session_window.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

// Toolbar button height (pixels); the toolbar control auto-sizes vertically.
constexpr int kToolbarBtnSize = 24;

// Minimum splitter left-pane width.
constexpr int kMinSplitX = 80;

// Padding between splitter panes.
constexpr int kSplitGap = 4;

// ----- toolbar button definitions ----------------------------------------
struct TbButton
{
    int   cmdId;
    int   bitmapIndex; // index into the standard image list (not used here –
                       // we use text-only buttons like the Qt ToolButtonTextBesideIcon)
    const wchar_t* text;
};

const TbButton kTbButtons[] = {
    { IDM_SYSINFO_SAVE,    0, L"Save"    },
    { IDM_SYSINFO_PRINT,   1, L"Print"   },
    { IDM_SYSINFO_REFRESH, 2, L"Refresh" },
};

constexpr int kTbButtonCount =
    static_cast<int>(sizeof(kTbButtons) / sizeof(kTbButtons[0]));

// -------------------------------------------------------------------------
// Helper: integer item data on a HTREEITEM.
// -------------------------------------------------------------------------
void treeSetItemId(HWND tree, HTREEITEM item, int id)
{
    TVITEMW tv  = {};
    tv.hItem    = item;
    tv.mask     = TVIF_PARAM;
    tv.lParam   = static_cast<LPARAM>(id);
    TreeView_SetItem(tree, &tv);
}

int treeGetItemId(HWND tree, HTREEITEM item)
{
    TVITEMW tv  = {};
    tv.hItem    = item;
    tv.mask     = TVIF_PARAM;
    if (!TreeView_GetItem(tree, &tv))
        return -1;
    return static_cast<int>(tv.lParam);
}

}  // namespace

SystemInfoSessionWindow::SystemInfoSessionWindow(HINSTANCE instance)
    : instance_(instance)
{
}

SystemInfoSessionWindow::~SystemInfoSessionWindow() = default;

bool SystemInfoSessionWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_WIN95_CLASSES | ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &SystemInfoSessionWindow::windowProc;
    wc.hInstance     = instance_;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm       = LoadIconW(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        WS_EX_APPWINDOW,
        kClassName, L"System Information",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT, 990, 703,
        nullptr, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void SystemInfoSessionWindow::show(int showCmd)
{
    if (hwnd_)
    {
        ShowWindow(hwnd_, showCmd);
        UpdateWindow(hwnd_);
    }
}

HTREEITEM SystemInfoSessionWindow::addCategory(
    HTREEITEM parent, const wchar_t* name, int categoryId)
{
    if (!tree_)
        return nullptr;

    TVINSERTSTRUCTW ins  = {};
    ins.hParent          = parent ? parent : TVI_ROOT;
    ins.hInsertAfter     = TVI_LAST;
    ins.item.mask        = TVIF_TEXT | TVIF_PARAM;
    ins.item.pszText     = const_cast<wchar_t*>(name);
    ins.item.lParam      = static_cast<LPARAM>(categoryId);

    return TreeView_InsertItem(tree_, &ins);
}

void SystemInfoSessionWindow::setContentWindow(HWND child)
{
    if (!content_ || !child)
        return;

    // Re-parent the new child into the content placeholder.
    SetParent(child, content_);

    // Stretch it to fill the entire content pane.
    RECT rc = {};
    GetClientRect(content_, &rc);
    SetWindowPos(child, nullptr,
                 0, 0, rc.right, rc.bottom,
                 SWP_NOZORDER | SWP_SHOWWINDOW);
}

void SystemInfoSessionWindow::enableButton(int cmdId, bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(cmdId),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

bool SystemInfoSessionWindow::preTranslateMessage(MSG* msg)
{
    if (!msg || msg->message != WM_KEYDOWN)
        return false;

    const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

    if (ctrl && msg->wParam == 'S')
    {
        onCommand(IDM_SYSINFO_SAVE);
        return true;
    }
    if (ctrl && msg->wParam == 'P')
    {
        onCommand(IDM_SYSINFO_PRINT);
        return true;
    }
    if (msg->wParam == VK_F5)
    {
        onCommand(IDM_SYSINFO_REFRESH);
        return true;
    }
    return false;
}

// static
LRESULT CALLBACK SystemInfoSessionWindow::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SystemInfoSessionWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<SystemInfoSessionWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<SystemInfoSessionWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT SystemInfoSessionWindow::handleMessage(
    UINT msg, WPARAM wp, LPARAM lp)
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

        case WM_CONTEXTMENU:
            onContextMenu(reinterpret_cast<HWND>(wp),
                          GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void SystemInfoSessionWindow::onCreate()
{
    createToolbar();
    createTree();
    createContentPane();
}

void SystemInfoSessionWindow::createToolbar()
{
    toolbar_ = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST |
            CCS_NODIVIDER | CCS_NOPARENTALIGN,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SYSINFO_WIN_TOOLBAR)),
        instance_, nullptr);

    SendMessageW(toolbar_, TB_BUTTONSTRUCTSIZE,
                 sizeof(TBBUTTON), 0);

    // Use the standard small image list (16×16) via TB_LOADIMAGES,
    // but since we show text-beside-icon like the Qt original we add
    // string labels. We set a small bitmap size and use IDB_STD_SMALL_COLOR.
    SendMessageW(toolbar_, TB_SETBITMAPSIZE, 0, MAKELPARAM(16, 16));
    SendMessageW(toolbar_, TB_SETIMAGELIST,  0,
                 reinterpret_cast<LPARAM>(
                     ImageList_Create(16, 16, ILC_COLOR32, 3, 0)));

    for (int i = 0; i < kTbButtonCount; ++i)
    {
        // Add the button label string and get its string index.
        const int strIdx = static_cast<int>(
            SendMessageW(toolbar_, TB_ADDSTRING, 0,
                         reinterpret_cast<LPARAM>(kTbButtons[i].text)));

        TBBUTTON btn  = {};
        btn.iBitmap   = I_IMAGENONE;
        btn.idCommand = kTbButtons[i].cmdId;
        btn.fsState   = TBSTATE_ENABLED;
        btn.fsStyle   = BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT;
        btn.iString   = strIdx;
        SendMessageW(toolbar_, TB_ADDBUTTONS, 1,
                     reinterpret_cast<LPARAM>(&btn));
    }

    SendMessageW(toolbar_, TB_AUTOSIZE, 0, 0);
}

void SystemInfoSessionWindow::createTree()
{
    tree_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SYSINFO_WIN_TREE_CAT)),
        instance_, nullptr);
}

void SystemInfoSessionWindow::createContentPane()
{
    // Plain child window — the active sys-info sub-widget is re-parented here.
    content_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SYSINFO_WIN_CONTENT)),
        instance_, nullptr);
}

void SystemInfoSessionWindow::onSize(int width, int height)
{
    layoutChildren(width, height);
}

void SystemInfoSessionWindow::layoutChildren(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    // 1. Resize the toolbar (it resizes itself horizontally).
    int toolbarHeight = 0;
    if (toolbar_)
    {
        SetWindowPos(toolbar_, nullptr,
                     0, 0, width, 0,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
        RECT tbrc = {};
        GetWindowRect(toolbar_, &tbrc);
        toolbarHeight = tbrc.bottom - tbrc.top;
    }

    // 2. Below the toolbar: split into tree (left) and content (right).
    const int contentY = toolbarHeight;
    const int contentH = height - toolbarHeight;

    if (contentH <= 0)
        return;

    // Clamp splitter position.
    if (splitterX_ < kMinSplitX)
        splitterX_ = kMinSplitX;
    if (splitterX_ > width - kMinSplitX - kSplitGap)
        splitterX_ = width - kMinSplitX - kSplitGap;

    const int treeW    = splitterX_;
    const int contentX = splitterX_ + kSplitGap;
    const int contentW = width - contentX;

    if (tree_)
        SetWindowPos(tree_, nullptr,
                     0, contentY, treeW, contentH,
                     SWP_NOZORDER | SWP_NOACTIVATE);

    if (content_)
    {
        SetWindowPos(content_, nullptr,
                     contentX, contentY, contentW, contentH,
                     SWP_NOZORDER | SWP_NOACTIVATE);

        // Also resize any child that has been re-parented into content_.
        HWND child = GetWindow(content_, GW_CHILD);
        if (child)
            SetWindowPos(child, nullptr,
                         0, 0, contentW, contentH,
                         SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void SystemInfoSessionWindow::onNotify(LPARAM lp)
{
    const NMHDR* hdr = reinterpret_cast<const NMHDR*>(lp);
    if (!hdr)
        return;

    if (hdr->hwndFrom == tree_ && hdr->code == TVN_SELCHANGEDW)
    {
        const NMTREEVIEWW* nm = reinterpret_cast<const NMTREEVIEWW*>(lp);
        const int id = static_cast<int>(nm->itemNew.lParam);
        if (delegate_.onCategorySelected)
            delegate_.onCategorySelected(id);
    }
}

void SystemInfoSessionWindow::onCommand(int cmdId)
{
    switch (cmdId)
    {
        case IDM_SYSINFO_SAVE:
            if (delegate_.onSave)    delegate_.onSave();
            break;
        case IDM_SYSINFO_PRINT:
            if (delegate_.onPrint)   delegate_.onPrint();
            break;
        case IDM_SYSINFO_REFRESH:
            if (delegate_.onRefresh) delegate_.onRefresh();
            break;
        case IDM_SYSINFO_COPY_ROW:
            if (delegate_.onCopyRow)   delegate_.onCopyRow();
            break;
        case IDM_SYSINFO_COPY_NAME:
            if (delegate_.onCopyName)  delegate_.onCopyName();
            break;
        case IDM_SYSINFO_COPY_VALUE:
            if (delegate_.onCopyValue) delegate_.onCopyValue();
            break;
        default:
            break;
    }
}

void SystemInfoSessionWindow::onContextMenu(HWND target, int screenX, int screenY)
{
    // Show context menu over the content pane (matches Qt's CustomContextMenu).
    if (target != content_)
        return;

    HMENU menu = CreatePopupMenu();
    if (!menu)
        return;

    AppendMenuW(menu, MF_STRING, IDM_SYSINFO_COPY_ROW,   L"Copy Row");
    AppendMenuW(menu, MF_STRING, IDM_SYSINFO_COPY_NAME,  L"Copy Name");
    AppendMenuW(menu, MF_STRING, IDM_SYSINFO_COPY_VALUE, L"Copy Value");

    TrackPopupMenu(menu,
                   TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                   screenX, screenY, 0, hwnd_, nullptr);
    DestroyMenu(menu);
}

void SystemInfoSessionWindow::onDestroy()
{
    PostQuitMessage(0);
}

}  // namespace aspia::client_win32
