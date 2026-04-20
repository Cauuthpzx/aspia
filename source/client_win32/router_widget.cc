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

#include "client_win32/router_widget.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

RouterWidget::RouterWidget(HINSTANCE instance)
    : instance_(instance)
{
}

RouterWidget::~RouterWidget() = default;

bool RouterWidget::create(HWND parent)
{
    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &RouterWidget::windowProc;
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

void RouterWidget::move(int x, int y, int width, int height)
{
    if (hwnd_)
        SetWindowPos(hwnd_, nullptr, x, y, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
}

// static
LRESULT CALLBACK RouterWidget::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    RouterWidget* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<RouterWidget*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<RouterWidget*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT RouterWidget::handleMessage(UINT msg, WPARAM /*wp*/, LPARAM lp)
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

void RouterWidget::onCreate()
{
    // SS_CENTER centres the text horizontally; we also centre it vertically
    // by making the STATIC fill the whole client area.
    label_ = CreateWindowExW(
        0, L"STATIC", L"Router",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_ROUTER_WIDGET_LABEL)),
        instance_, nullptr);
}

void RouterWidget::onSize(int width, int height)
{
    if (label_)
        SetWindowPos(label_, nullptr, 0, 0, width, height,
                     SWP_NOZORDER | SWP_NOACTIVATE);
}

void RouterWidget::onDestroy()
{
}

}  // namespace aspia::client_win32
