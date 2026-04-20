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
// Win32 replacement for client/ui/hosts/router_widget.ui.
//
// The Qt original is a QWidget with a single centred QLabel that reads
// "Router". This is a placeholder panel shown when the address-book is
// in Router mode but no computer group is selected. The Win32 equivalent
// is a simple child window that paints the same centred text.
//
// IDC_ROUTER_WIDGET_LABEL = 8110
//

#ifndef CLIENT_WIN32_ROUTER_WIDGET_H
#define CLIENT_WIN32_ROUTER_WIDGET_H

#include <windows.h>

namespace aspia::client_win32 {

class RouterWidget
{
public:
    explicit RouterWidget(HINSTANCE instance);
    ~RouterWidget();

    RouterWidget(const RouterWidget&) = delete;
    RouterWidget& operator=(const RouterWidget&) = delete;

    bool create(HWND parent);
    void move(int x, int y, int width, int height);

    HWND handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onDestroy();

    HINSTANCE instance_;
    HWND      hwnd_  = nullptr;
    HWND      label_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32RouterWidget";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_ROUTER_WIDGET_H
