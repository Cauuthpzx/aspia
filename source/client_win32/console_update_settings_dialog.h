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
// Win32 replacement for console/update_settings_dialog.ui.
//
// When wiring this dialog up, add the following to
// source/client_win32/resource.h (do NOT edit that file from this port;
// these are the identifiers the implementation below expects):
//
// #define IDD_CONSOLE_UPDATE_SETTINGS              500
//
// #define IDC_CONSOLE_UPDATE_CHECK_ON_STARTUP      7300
// #define IDC_CONSOLE_UPDATE_USE_CUSTOM_SERVER     7301
// #define IDC_CONSOLE_UPDATE_SERVER_LABEL          7302
// #define IDC_CONSOLE_UPDATE_SERVER                7303
// #define IDC_CONSOLE_UPDATE_CHECK_NOW             7304
//

#ifndef CLIENT_WIN32_CONSOLE_UPDATE_SETTINGS_DIALOG_H
#define CLIENT_WIN32_CONSOLE_UPDATE_SETTINGS_DIALOG_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class ConsoleUpdateSettingsDialog
{
public:
    struct Result
    {
        bool check_on_startup = true;
        bool use_custom_server = false;
        std::wstring update_server;
    };

    ConsoleUpdateSettingsDialog(HINSTANCE instance, HWND parent);
    ~ConsoleUpdateSettingsDialog() = default;

    ConsoleUpdateSettingsDialog(const ConsoleUpdateSettingsDialog&) = delete;
    ConsoleUpdateSettingsDialog& operator=(const ConsoleUpdateSettingsDialog&) = delete;

    // Pre-populate the dialog with an existing configuration.
    void setInitial(const Result& initial) { result_ = initial; }

    bool exec();
    const Result& result() const { return result_; }

private:
    static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    INT_PTR handleMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    void onInitDialog(HWND hwnd);
    void onOk(HWND hwnd);
    void onCustomServerToggled(HWND hwnd);
    void onCheckForUpdatesNow(HWND hwnd);

    void updateServerEnable(HWND hwnd);

    HINSTANCE instance_;
    HWND parent_;
    Result result_;
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_CONSOLE_UPDATE_SETTINGS_DIALOG_H
