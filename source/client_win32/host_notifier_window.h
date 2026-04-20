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
// Win32 replacement for host/ui/notifier_window.ui.
//
// Resource IDs expected (to be added to resource.h by a separate change):
//   #define IDC_HOST_NOTIFIER_TITLE        6400
//   #define IDC_HOST_NOTIFIER_COLLAPSE     6401
//   #define IDC_HOST_NOTIFIER_EXPAND       6402
//   #define IDC_HOST_NOTIFIER_SESSIONS     6403
//   #define IDC_HOST_NOTIFIER_LOCK_MOUSE   6410
//   #define IDC_HOST_NOTIFIER_LOCK_KBD     6411
//   #define IDC_HOST_NOTIFIER_PAUSE        6412
//   #define IDC_HOST_NOTIFIER_DISCONNECT   6413
//   #define IDC_HOST_NOTIFIER_FIRST_DISCON 6450
//   #define IDC_HOST_NOTIFIER_LAST_DISCON  6499
//   #define IDR_HOST_NOTIFIER_ACCEL        410
//

#ifndef CLIENT_WIN32_HOST_NOTIFIER_WINDOW_H
#define CLIENT_WIN32_HOST_NOTIFIER_WINDOW_H

#include <windows.h>
#include <commctrl.h>

#include <cstdint>
#include <string>
#include <vector>

namespace aspia::client_win32 {

class HostNotifierWindow
{
public:
    // Notification codes delivered to the parent HWND via WM_APP+N.
    // wParam: code (below); lParam: session_id (or 0 for global actions).
    enum Notification : UINT
    {
        kNotifyDisconnectSession = WM_APP + 1,
        kNotifyDisconnectAll     = WM_APP + 2,
        kNotifyLockMouse         = WM_APP + 3,
        kNotifyLockKeyboard      = WM_APP + 4,
        kNotifyPause             = WM_APP + 5,
    };

    struct Session
    {
        std::uint32_t session_id = 0;
        std::wstring user_name;
        std::wstring session_type;  // "Desktop Manage", "File Transfer", etc.
    };

    HostNotifierWindow(HINSTANCE instance, HWND notifyParent);
    ~HostNotifierWindow();

    HostNotifierWindow(const HostNotifierWindow&) = delete;
    HostNotifierWindow& operator=(const HostNotifierWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    HWND handle() const { return hwnd_; }

    // Replaces the currently displayed session list.
    void setSessions(const std::vector<Session>& sessions);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onCommand(int id);
    void onNotify(LPARAM lp);
    void onDestroy();

    void createChildren();
    void layoutChildren(int width, int height);
    void refreshListView();
    void positionInitially();

    void toggleCollapsed();
    void postToParent(UINT code, LPARAM lp);

    HINSTANCE instance_;
    HWND      notifyParent_ = nullptr;
    HWND      hwnd_         = nullptr;
    HWND      titleLabel_   = nullptr;
    HWND      collapseBtn_  = nullptr;
    HWND      expandBtn_    = nullptr;
    HWND      sessions_     = nullptr;     // SysListView32 LVS_REPORT
    HWND      lockMouseBtn_ = nullptr;
    HWND      lockKbdBtn_   = nullptr;
    HWND      pauseBtn_     = nullptr;
    HWND      disconnectBtn_ = nullptr;
    HACCEL    accel_        = nullptr;

    std::vector<Session> sessions_data_;
    bool collapsed_ = false;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32HostNotifier";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_HOST_NOTIFIER_WINDOW_H
