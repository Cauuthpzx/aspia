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

#include "client_win32/console_main_window.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Control IDs - kept local to this TU until they are formally added to
// resource.h (see comment block at top of console_main_window.h).
constexpr int kIdTabs        = 7400;
constexpr int kIdStatusBar   = 7401;
constexpr int kIdToolbar     = 7402;

// File menu
constexpr int kIdFileNew     = 7410;
constexpr int kIdFileOpen    = 7411;
constexpr int kIdFileSave    = 7412;
constexpr int kIdFileSaveAs  = 7413;
constexpr int kIdFileSaveAll = 7414;
constexpr int kIdFileClose   = 7415;
constexpr int kIdFileCloseAll = 7416;
constexpr int kIdFileExit    = 7417;

// Edit menu
constexpr int kIdImport      = 7428;
constexpr int kIdExport      = 7429;

// Menu/accel resource IDs.
constexpr int kIdrConsoleMainMenu  = 510;
constexpr int kIdrConsoleMainAccel = 511;

}  // namespace

ConsoleMainWindow::ConsoleMainWindow(HINSTANCE instance)
    : instance_(instance)
{
}

ConsoleMainWindow::~ConsoleMainWindow()
{
    if (accel_)
        DestroyAcceleratorTable(accel_);
}

bool ConsoleMainWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &ConsoleMainWindow::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    HMENU menu = LoadMenuW(instance_, MAKEINTRESOURCEW(kIdrConsoleMainMenu));

    hwnd_ = CreateWindowExW(
        0, kClassName, L"Aspia Console",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 891, 613,
        nullptr, menu, instance_, this);

    return hwnd_ != nullptr;
}

void ConsoleMainWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool ConsoleMainWindow::preTranslateMessage(MSG* msg)
{
    if (accel_ && TranslateAcceleratorW(hwnd_, accel_, msg))
        return true;
    return false;
}

int ConsoleMainWindow::addAddressBookTab(const std::wstring& title)
{
    if (!tabs_)
        return -1;

    const int index = TabCtrl_GetItemCount(tabs_);

    TCITEMW item = {};
    item.mask = TCIF_TEXT;
    item.pszText = const_cast<wchar_t*>(title.c_str());

    // TODO(phase2): attach an AddressBookTab child widget as the tab's
    // payload (see source/console/address_book_tab.* for the Qt version).
    return TabCtrl_InsertItem(tabs_, index, &item);
}

// static
LRESULT CALLBACK ConsoleMainWindow::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ConsoleMainWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<ConsoleMainWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<ConsoleMainWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT ConsoleMainWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

        case WM_DESTROY:
            onDestroy();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void ConsoleMainWindow::onCreate()
{
    // Toolbar (TBSTYLE_FLAT matches the Qt QToolBar appearance). Buttons
    // will be populated when the action icons are ported; for now the
    // toolbar exists so layout slots are stable.
    toolbar_ = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_TOP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdToolbar)),
        instance_, nullptr);
    if (toolbar_)
    {
        SendMessageW(toolbar_, TB_BUTTONSTRUCTSIZE,
                     static_cast<WPARAM>(sizeof(TBBUTTON)), 0);
    }

    tabs_ = CreateWindowExW(
        0, WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FOCUSNEVER |
            TCS_TABS | TCS_SINGLELINE,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdTabs)),
        instance_, nullptr);

    status_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusBar)),
        instance_, nullptr);

    accel_ = LoadAcceleratorsW(instance_, MAKEINTRESOURCEW(kIdrConsoleMainAccel));
}

void ConsoleMainWindow::onSize(int width, int height)
{
    // Let toolbar and status bar auto-size themselves.
    if (toolbar_)
        SendMessageW(toolbar_, TB_AUTOSIZE, 0, 0);
    if (status_)
        SendMessageW(status_, WM_SIZE, 0, 0);

    RECT toolbarRect = {};
    int toolbarHeight = 0;
    if (toolbar_)
    {
        GetWindowRect(toolbar_, &toolbarRect);
        toolbarHeight = toolbarRect.bottom - toolbarRect.top;
    }

    RECT statusRect = {};
    int statusHeight = 0;
    if (status_)
    {
        GetWindowRect(status_, &statusRect);
        statusHeight = statusRect.bottom - statusRect.top;
    }

    const int tabsTop = toolbarHeight;
    const int tabsHeight = height - toolbarHeight - statusHeight;

    if (tabs_)
    {
        SetWindowPos(tabs_, nullptr,
                     0, tabsTop, width, (tabsHeight > 0) ? tabsHeight : 0,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void ConsoleMainWindow::onCommand(int id)
{
    switch (id)
    {
        case kIdFileNew:      onFileNew();       break;
        case kIdFileOpen:     onFileOpen();      break;
        case kIdFileSave:     onFileSave();      break;
        case kIdFileSaveAs:   onFileSaveAs();    break;
        case kIdFileSaveAll:  onFileSaveAll();   break;
        case kIdFileClose:    onFileClose();     break;
        case kIdFileCloseAll: onFileCloseAll();  break;
        case kIdFileExit:     onFileExit();      break;
        case kIdImport:       onImportComputers(); break;
        case kIdExport:       onExportComputers(); break;
        default:
            break;
    }
}

void ConsoleMainWindow::onDestroy()
{
}

// ---------------------------------------------------------------------------
// Menu command stubs. Real behavior (file dialogs, serialization, address
// book decryption, etc.) arrives once console/address_book_tab.* and its
// dialogs are ported to Win32.
// ---------------------------------------------------------------------------

void ConsoleMainWindow::onFileNew()
{
    // TODO(phase2): open console_address_book_dialog (new-file mode) and,
    // on acceptance, create a fresh AddressBookTab and append it via
    // addAddressBookTab().
}

void ConsoleMainWindow::onFileOpen()
{
    // TODO(phase2): prompt via console_open_address_book_dialog, then call
    // addAddressBookTab() on success.
}

void ConsoleMainWindow::onFileSave()
{
    // TODO(phase2): delegate to the currently active AddressBookTab.
}

void ConsoleMainWindow::onFileSaveAs()
{
    // TODO(phase2): delegate to the active AddressBookTab with a
    // destination path chosen via GetSaveFileNameW.
}

void ConsoleMainWindow::onFileSaveAll()
{
    // TODO(phase2): iterate every tab and call its save().
}

void ConsoleMainWindow::onFileClose()
{
    // TODO(phase2): close the active tab (prompt for save if dirty).
}

void ConsoleMainWindow::onFileCloseAll()
{
    // TODO(phase2): close every tab (prompt for save for each dirty one).
}

void ConsoleMainWindow::onFileExit()
{
    DestroyWindow(hwnd_);
}

void ConsoleMainWindow::onImportComputers()
{
    // TODO(phase2): delegate to the active AddressBookTab's import flow.
}

void ConsoleMainWindow::onExportComputers()
{
    // TODO(phase2): delegate to the active AddressBookTab's export flow.
}

}  // namespace aspia::client_win32
