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
// Win32 replacement for host/ui/main_window.ui.
//
// Control IDs:
//   IDC_HOSTMAIN_LABEL_ID              6500
//   IDC_HOSTMAIN_VALUE_ID              6501
//   IDC_HOSTMAIN_LABEL_IP              6510
//   IDC_HOSTMAIN_VALUE_IP              6502
//   IDC_HOSTMAIN_LABEL_STATUS          6520
//   IDC_HOSTMAIN_VALUE_STATUS          6503
//   IDC_HOSTMAIN_CHECK_ACCEPT          6530
//   IDC_HOSTMAIN_CHECK_USE_PASSWORD    6531
//   IDC_HOSTMAIN_BUTTON_CHANGE_PWD     6540
//   IDC_HOSTMAIN_BUTTON_SETTINGS       6541
//   IDC_HOSTMAIN_BUTTON_HELP           6542
//   IDC_HOSTMAIN_BUTTON_EXIT           6543
//   IDC_HOSTMAIN_STATUSBAR             6590
//
// Menu / accelerator resources (not a DIALOGEX):
//   IDR_HOST_MAIN_MENU                 420
//   IDR_HOST_MAIN_ACCEL                421
//

#ifndef CLIENT_WIN32_HOST_MAIN_WINDOW_H
#define CLIENT_WIN32_HOST_MAIN_WINDOW_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class HostMainWindow
{
public:
    explicit HostMainWindow(HINSTANCE instance);
    virtual ~HostMainWindow();

    HostMainWindow(const HostMainWindow&) = delete;
    HostMainWindow& operator=(const HostMainWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND handle() const { return hwnd_; }

    // Populates the "Your ID", "Your IP Address" and "Status" value labels.
    void setLocalInfo(const std::wstring& id,
                      const std::wstring& ip,
                      const std::wstring& status);

    // If set, WM_COMMAND notifications are forwarded to this HWND. Mutually
    // exclusive with the virtual hooks below (they are still invoked locally).
    void setCommandForwardTarget(HWND target) { forward_ = target; }

protected:
    // Hooks a derived class can override. Default implementations are no-ops.
    virtual void onAcceptConnectionsToggled(bool checked);
    virtual void onUsePasswordToggled(bool checked);
    virtual void onChangePassword();
    virtual void onSettings();
    virtual void onHelp();
    virtual void onExit();

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onCommand(int id, int code, HWND ctrl);
    void onDestroy();

    void createChildren();
    void layoutChildren(int width, int height);

    HINSTANCE instance_;
    HWND hwnd_ = nullptr;
    HWND forward_ = nullptr;

    HWND label_id_ = nullptr;
    HWND value_id_ = nullptr;
    HWND label_ip_ = nullptr;
    HWND value_ip_ = nullptr;
    HWND label_status_ = nullptr;
    HWND value_status_ = nullptr;

    HWND check_accept_ = nullptr;
    HWND check_use_password_ = nullptr;

    HWND button_change_password_ = nullptr;
    HWND button_settings_ = nullptr;
    HWND button_help_ = nullptr;
    HWND button_exit_ = nullptr;

    HWND status_ = nullptr;

    HFONT font_ = nullptr;
    HACCEL accel_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32HostMain";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_HOST_MAIN_WINDOW_H
