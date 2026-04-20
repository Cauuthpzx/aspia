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

#include "client_win32/main_window.h"

#include "client_win32/auth_dialog.h"
#include "client_win32/desktop_session_adapter.h"
#include "client_win32/desktop_session_window.h"
#include "client_win32/resource.h"
#include "client/client_desktop.h"
#include "client/config.h"
#include "client/session_state.h"
#include "proto/desktop_control.h"
#include "proto/peer.h"

#include <QCoreApplication>
#include <QString>
#include <commctrl.h>

namespace aspia::client_win32 {

MainWindow::MainWindow(HINSTANCE instance)
    : instance_(instance)
{
}

MainWindow::~MainWindow()
{
    if (accel_)
        DestroyAcceleratorTable(accel_);
}

bool MainWindow::create()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &MainWindow::windowProc;
    wc.hInstance = instance_;
    wc.hIcon = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_ASPIA));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    HMENU menu = LoadMenuW(instance_, MAKEINTRESOURCEW(IDR_MAIN_MENU));

    hwnd_ = CreateWindowExW(
        0, kClassName, L"Aspia Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 640,
        nullptr, menu, instance_, this);

    return hwnd_ != nullptr;
}

void MainWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool MainWindow::preTranslateMessage(MSG* msg)
{
    if (accel_ && TranslateAcceleratorW(hwnd_, accel_, msg))
        return true;
    return false;
}

// static
LRESULT CALLBACK MainWindow::windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    MainWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<MainWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT MainWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
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

void MainWindow::onCreate()
{
    tabs_ = CreateWindowExW(
        0, WC_TABCONTROLW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_FOCUSNEVER,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_MAIN_TABS)),
        instance_, nullptr);

    status_ = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_MAIN_STATUSBAR)),
        instance_, nullptr);

    accel_ = LoadAcceleratorsW(instance_, MAKEINTRESOURCEW(IDR_MAIN_ACCEL));
}

void MainWindow::onSize(int width, int height)
{
    if (status_)
    {
        SendMessageW(status_, WM_SIZE, 0, 0);
    }

    RECT statusRect = {};
    if (status_)
        GetWindowRect(status_, &statusRect);
    const int statusHeight = statusRect.bottom - statusRect.top;

    if (tabs_)
    {
        SetWindowPos(tabs_, nullptr, 0, 0, width, height - statusHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void MainWindow::onCommand(int id)
{
    switch (id)
    {
        case ID_FILE_CONNECT:
        {
            AuthDialog dialog(instance_, hwnd_);
            if (dialog.exec())
            {
                const AuthDialog::Result& r = dialog.result();
                if (!r.address.empty())
                    launchDesktopSession(r.address, r.port, r.username, r.password);
            }
            break;
        }

        case ID_FILE_EXIT:
            DestroyWindow(hwnd_);
            break;

        default:
            break;
    }
}

void MainWindow::onDestroy()
{
    // Destroy all active sessions before the message loop exits.
    sessions_.clear();
    QCoreApplication::quit();
}

void MainWindow::launchDesktopSession(const std::wstring& address, uint16_t port,
                                      const std::wstring& username,
                                      const std::wstring& password)
{
    client::Config cfg;
    cfg.address_or_id = QString::fromStdWString(address);
    cfg.port          = port;
    cfg.username      = QString::fromStdWString(username);
    cfg.password      = QString::fromStdWString(password);
    cfg.session_type  = proto::peer::SESSION_TYPE_DESKTOP_MANAGE;

    auto state = std::make_shared<client::SessionState>(cfg);

    proto::control::Config desktop_cfg;
    // Use default desktop config (codec/quality chosen by the host).

    auto entry = std::make_unique<Session>();

    entry->client = std::make_unique<client::ClientDesktop>(desktop_cfg);
    entry->client->setSessionState(state);

    entry->window = std::make_unique<DesktopSessionWindow>(instance_);
    if (!entry->window->create())
        return;

    const std::wstring title = address + L" \u2014 Aspia Remote Desktop";
    entry->window->setTitle(title.c_str());

    entry->adapter = std::make_unique<DesktopSessionAdapter>(
        entry->client.get(), entry->window.get());

    entry->window->show();
    entry->client->start();

    sessions_.push_back(std::move(entry));
}

}  // namespace aspia::client_win32
