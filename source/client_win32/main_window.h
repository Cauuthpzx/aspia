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

#ifndef CLIENT_WIN32_MAIN_WINDOW_H
#define CLIENT_WIN32_MAIN_WINDOW_H

#include <windows.h>

namespace aspia::client_win32 {

class MainWindow
{
public:
    explicit MainWindow(HINSTANCE instance);
    ~MainWindow();

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND hwnd() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onCommand(int id);
    void onDestroy();

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND tabs_ = nullptr;
    HWND status_ = nullptr;
    HACCEL accel_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32MainWindow";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_MAIN_WINDOW_H
