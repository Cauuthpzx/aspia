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

#include "client_win32/file_transfer_session_window.h"

#include "client_win32/file_panel.h"

#include <commctrl.h>

namespace aspia::client_win32 {

namespace {

// Control IDs (range 4200..4299). Kept local to this TU until they are
// formally added to resource.h (see comment block at top of
// file_transfer_session_window.h).
constexpr int kIdLocalPanel  = 4200;
constexpr int kIdRemotePanel = 4201;
constexpr int kIdStatusBar   = 4202;

// Horizontal gap between the two panels, in pixels. Matches the original
// QSplitter handleWidth of 3 from the Qt .ui file.
constexpr int kPanelGap = 3;

}  // namespace

FileTransferSessionWindow::FileTransferSessionWindow(HINSTANCE instance)
    : instance_(instance)
{
}

FileTransferSessionWindow::~FileTransferSessionWindow() = default;

bool FileTransferSessionWindow::create()
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &FileTransferSessionWindow::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"File Transfer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1009, 709,
        nullptr, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void FileTransferSessionWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool FileTransferSessionWindow::preTranslateMessage(MSG* /*msg*/)
{
    return false;
}

// static
LRESULT CALLBACK FileTransferSessionWindow::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    FileTransferSessionWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<FileTransferSessionWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<FileTransferSessionWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT FileTransferSessionWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void FileTransferSessionWindow::onCreate()
{
    localPanel_ = std::make_unique<FilePanel>(instance_, hwnd_);
    if (localPanel_->create())
    {
        SetWindowLongPtrW(localPanel_->handle(), GWLP_ID,
                          static_cast<LONG_PTR>(kIdLocalPanel));
    }

    remotePanel_ = std::make_unique<FilePanel>(instance_, hwnd_);
    if (remotePanel_->create())
    {
        SetWindowLongPtrW(remotePanel_->handle(), GWLP_ID,
                          static_cast<LONG_PTR>(kIdRemotePanel));
    }

    status_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdStatusBar)),
        instance_, nullptr);
}

void FileTransferSessionWindow::onSize(int width, int height)
{
    if (status_)
        SendMessageW(status_, WM_SIZE, 0, 0);

    RECT statusRect = {};
    int statusHeight = 0;
    if (status_)
    {
        GetWindowRect(status_, &statusRect);
        statusHeight = statusRect.bottom - statusRect.top;
    }

    const int usableHeight = height - statusHeight;
    if (usableHeight <= 0 || width <= 0)
        return;

    // 50/50 horizontal split with a small gap between the two panels,
    // matching the original QSplitter (handleWidth=3).
    const int halfWidth = (width - kPanelGap) / 2;
    const int leftX     = 0;
    const int rightX    = halfWidth + kPanelGap;
    const int rightW    = width - rightX;

    if (localPanel_ && localPanel_->handle())
        localPanel_->move(leftX, 0, halfWidth, usableHeight);

    if (remotePanel_ && remotePanel_->handle())
        remotePanel_->move(rightX, 0, rightW, usableHeight);
}

void FileTransferSessionWindow::onCommand(int /*id*/)
{
    // Toolbar actions originating in either FilePanel bubble up here as
    // WM_COMMAND notifications. Handling will be wired in once the Win32
    // file-transfer client core is ported (phase 2).
}

void FileTransferSessionWindow::onDestroy()
{
    // Top-level, non-modal window: do NOT post WM_QUIT here. The owning
    // application decides whether closing this window exits the process.
    localPanel_.reset();
    remotePanel_.reset();
}

}  // namespace aspia::client_win32
