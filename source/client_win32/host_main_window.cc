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

#include "client_win32/host_main_window.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Local control IDs (kept here until they are added to resource.h). See the
// comment block at the top of host_main_window.h for the full assignment.
constexpr int kIdLabelId           = 6500;
constexpr int kIdValueId           = 6501;
constexpr int kIdLabelIp           = 6510;
constexpr int kIdValueIp           = 6502;
constexpr int kIdLabelStatus       = 6520;
constexpr int kIdValueStatus       = 6503;
constexpr int kIdCheckAccept       = 6530;
constexpr int kIdCheckUsePassword  = 6531;
constexpr int kIdButtonChangePwd   = 6540;
constexpr int kIdButtonSettings    = 6541;
constexpr int kIdButtonHelp        = 6542;
constexpr int kIdButtonExit        = 6543;
constexpr int kIdStatusBar         = 6590;

constexpr int kIdrHostMainMenu  = 420;
constexpr int kIdrHostMainAccel = 421;

// Layout constants (device-independent pixels; Win32 code does not DPI-scale
// at this layer — callers can set DPI awareness at the app level).
constexpr int kMargin      = 10;
constexpr int kRowSpacing  = 6;
constexpr int kLabelHeight = 18;
constexpr int kValueHeight = 20;
constexpr int kCheckHeight = 20;
constexpr int kButtonW     = 120;
constexpr int kButtonH     = 26;

HFONT createDefaultFont()
{
    NONCLIENTMETRICSW ncm = {};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfoW(SPI_NONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
        return CreateFontIndirectW(&ncm.lfMessageFont);
    return static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
}

void setFont(HWND ctrl, HFONT font)
{
    if (ctrl && font)
        SendMessageW(ctrl, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

}  // namespace

HostMainWindow::HostMainWindow(HINSTANCE instance)
    : instance_(instance)
{
}

HostMainWindow::~HostMainWindow()
{
    if (accel_)
        DestroyAcceleratorTable(accel_);
    if (font_)
        DeleteObject(font_);
}

bool HostMainWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &HostMainWindow::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    HMENU menu = LoadMenuW(instance_, MAKEINTRESOURCEW(kIdrHostMainMenu));

    hwnd_ = CreateWindowExW(
        0, kClassName, L"Aspia Host",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 340,
        nullptr, menu, instance_, this);

    return hwnd_ != nullptr;
}

void HostMainWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool HostMainWindow::preTranslateMessage(MSG* msg)
{
    if (accel_ && TranslateAcceleratorW(hwnd_, accel_, msg))
        return true;
    return false;
}

void HostMainWindow::setLocalInfo(const std::wstring& id,
                                  const std::wstring& ip,
                                  const std::wstring& status)
{
    if (value_id_)
        SetWindowTextW(value_id_, id.c_str());
    if (value_ip_)
        SetWindowTextW(value_ip_, ip.c_str());
    if (value_status_)
        SetWindowTextW(value_status_, status.c_str());
    if (status_)
        SendMessageW(status_, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(status.c_str()));
}

void HostMainWindow::onAcceptConnectionsToggled(bool /*checked*/) {}
void HostMainWindow::onUsePasswordToggled(bool /*checked*/) {}
void HostMainWindow::onChangePassword() {}
void HostMainWindow::onSettings() {}
void HostMainWindow::onHelp() {}
void HostMainWindow::onExit() { if (hwnd_) DestroyWindow(hwnd_); }

// static
LRESULT CALLBACK HostMainWindow::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    HostMainWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<HostMainWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<HostMainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT HostMainWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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
            onCommand(LOWORD(wp), HIWORD(wp), reinterpret_cast<HWND>(lp));
            return 0;

        case WM_DESTROY:
            onDestroy();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void HostMainWindow::onCreate()
{
    font_ = createDefaultFont();
    createChildren();
    accel_ = LoadAcceleratorsW(instance_, MAKEINTRESOURCEW(kIdrHostMainAccel));
}

void HostMainWindow::createChildren()
{
    auto makeChild = [&](LPCWSTR cls, LPCWSTR text, DWORD style, int id) -> HWND
    {
        HWND h = CreateWindowExW(
            0, cls, text,
            WS_CHILD | WS_VISIBLE | style,
            0, 0, 0, 0, hwnd_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            instance_, nullptr);
        setFont(h, font_);
        return h;
    };

    label_id_      = makeChild(L"STATIC", L"Your ID:",          SS_LEFT, kIdLabelId);
    value_id_      = makeChild(L"STATIC", L"-",                 SS_LEFT, kIdValueId);
    label_ip_      = makeChild(L"STATIC", L"Your IP Address:",  SS_LEFT, kIdLabelIp);
    value_ip_      = makeChild(L"STATIC", L"-",                 SS_LEFT, kIdValueIp);
    label_status_  = makeChild(L"STATIC", L"Status:",           SS_LEFT, kIdLabelStatus);
    value_status_  = makeChild(L"STATIC", L"-",                 SS_LEFT, kIdValueStatus);

    check_accept_ = makeChild(
        L"BUTTON", L"Accept incoming connections",
        BS_AUTOCHECKBOX | WS_TABSTOP, kIdCheckAccept);
    check_use_password_ = makeChild(
        L"BUTTON", L"Use password",
        BS_AUTOCHECKBOX | WS_TABSTOP, kIdCheckUsePassword);

    button_change_password_ = makeChild(
        L"BUTTON", L"Change Password...",
        BS_PUSHBUTTON | WS_TABSTOP, kIdButtonChangePwd);
    button_settings_ = makeChild(
        L"BUTTON", L"Settings",
        BS_PUSHBUTTON | WS_TABSTOP, kIdButtonSettings);
    button_help_ = makeChild(
        L"BUTTON", L"Help",
        BS_PUSHBUTTON | WS_TABSTOP, kIdButtonHelp);
    button_exit_ = makeChild(
        L"BUTTON", L"Exit",
        BS_PUSHBUTTON | WS_TABSTOP, kIdButtonExit);

    status_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusBar)),
        instance_, nullptr);
}

void HostMainWindow::onSize(int width, int height)
{
    if (status_)
        SendMessageW(status_, WM_SIZE, 0, 0);

    layoutChildren(width, height);
}

void HostMainWindow::layoutChildren(int width, int height)
{
    RECT statusRect = {};
    int statusHeight = 0;
    if (status_)
    {
        GetWindowRect(status_, &statusRect);
        statusHeight = statusRect.bottom - statusRect.top;
    }

    const int clientBottom = height - statusHeight;
    const int x = kMargin;
    const int w = width - 2 * kMargin;
    int y = kMargin;

    auto place = [](HWND h, int x, int y, int w, int hh)
    {
        if (h)
            SetWindowPos(h, nullptr, x, y, w, hh, SWP_NOZORDER | SWP_NOACTIVATE);
    };

    place(label_id_, x, y, w, kLabelHeight);
    y += kLabelHeight + 2;
    place(value_id_, x, y, w, kValueHeight);
    y += kValueHeight + kRowSpacing;

    place(label_ip_, x, y, w, kLabelHeight);
    y += kLabelHeight + 2;
    place(value_ip_, x, y, w, kValueHeight);
    y += kValueHeight + kRowSpacing;

    place(label_status_, x, y, w, kLabelHeight);
    y += kLabelHeight + 2;
    place(value_status_, x, y, w, kValueHeight);
    y += kValueHeight + kRowSpacing;

    place(check_accept_, x, y, w, kCheckHeight);
    y += kCheckHeight + 2;
    place(check_use_password_, x, y, w, kCheckHeight);
    y += kCheckHeight + kRowSpacing;

    // Button row, right-aligned, stacked toward the bottom of the client area.
    const int buttonsY = clientBottom - kMargin - kButtonH;
    int bx = width - kMargin - kButtonW;
    place(button_exit_,            bx, buttonsY, kButtonW, kButtonH);
    bx -= (kButtonW + kRowSpacing);
    place(button_help_,            bx, buttonsY, kButtonW, kButtonH);
    bx -= (kButtonW + kRowSpacing);
    place(button_settings_,        bx, buttonsY, kButtonW, kButtonH);
    bx -= (kButtonW + kRowSpacing);
    place(button_change_password_, bx, buttonsY, kButtonW, kButtonH);
}

void HostMainWindow::onCommand(int id, int code, HWND /*ctrl*/)
{
    // Forward to external parent if registered. The window still dispatches to
    // its own virtual hooks below so derived classes remain functional.
    if (forward_)
        PostMessageW(forward_, WM_COMMAND, MAKEWPARAM(id, code), 0);

    switch (id)
    {
        case kIdCheckAccept:
        {
            const bool checked =
                SendMessageW(check_accept_, BM_GETCHECK, 0, 0) == BST_CHECKED;
            onAcceptConnectionsToggled(checked);
            break;
        }

        case kIdCheckUsePassword:
        {
            const bool checked =
                SendMessageW(check_use_password_, BM_GETCHECK, 0, 0) == BST_CHECKED;
            onUsePasswordToggled(checked);
            break;
        }

        case kIdButtonChangePwd:
            onChangePassword();
            break;

        case kIdButtonSettings:
            onSettings();
            break;

        case kIdButtonHelp:
            onHelp();
            break;

        case kIdButtonExit:
            onExit();
            break;

        default:
            break;
    }
}

void HostMainWindow::onDestroy()
{
}

}  // namespace aspia::client_win32
