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
// Win32 replacement for client/ui/desktop/desktop_toolbar.ui.
//
// The Qt original is a QFrame containing a QToolBar with these buttons
// (in order):
//
//   action_pin (checkable)        ── separator ──
//   action_switch_session         action_power_control
//   action_cad                    action_settings
//   ── separator ──
//   action_autosize               action_fullscreen (checkable)
//   ── separator ──
//   action_file_transfer          action_text_chat
//   action_task_manager           action_system_info
//   ── separator ──
//   action_menu (popup)
//   ── separator ──
//   action_minimize               action_close
//
// The Win32 equivalent is a borderless, always-on-top child/popup window
// that contains a flat Win32 TOOLBARCLASSNAME control. When pinned
// (IDC_DT_PIN pressed = checked), the toolbar stays visible; when unpinned
// it auto-hides and only reappears when the mouse enters the top edge of the
// parent window.
//
// Power-control popup menu items:
//   IDM_DT_SHUTDOWN, IDM_DT_REBOOT, IDM_DT_REBOOT_SAFE_MODE,
//   IDM_DT_LOGOFF, IDM_DT_LOCK
//
// Advanced "menu" popup items:
//   IDM_DT_SCREENSHOT, IDM_DT_STATISTICS,
//   IDM_DT_RECORDING_SETTINGS, IDM_DT_START_RECORDING,
//   IDM_DT_PAUSE_VIDEO, IDM_DT_PAUSE_AUDIO,
//   IDM_DT_PASTE_CLIPBOARD, IDM_DT_SEND_KEY_COMBOS,
//   IDM_DT_AUTOSCROLL,
//   Scale sub-menu: IDM_DT_SCALE_FIT, IDM_DT_SCALE_100..50
//
// All IDC_DT_* / IDM_DT_* values are defined in resource.h.
//

#ifndef CLIENT_WIN32_DESKTOP_TOOLBAR_H
#define CLIENT_WIN32_DESKTOP_TOOLBAR_H

#include <windows.h>
#include <commctrl.h>

#include <functional>

namespace aspia::client_win32 {

class DesktopToolbar
{
public:
    // Callbacks — the caller (DesktopSessionWindow) wires these up.
    struct Delegate
    {
        std::function<void()>       onSwitchSession;
        std::function<void(int)>    onPowerControl;     // IDM_DT_* value
        std::function<void()>       onSendCad;
        std::function<void()>       onSettings;
        std::function<void()>       onAutoSize;
        std::function<void(bool)>   onFullscreen;       // true = enter
        std::function<void()>       onFileTransfer;
        std::function<void()>       onTextChat;
        std::function<void()>       onTaskManager;
        std::function<void()>       onSystemInfo;
        std::function<void()>       onMinimize;
        std::function<void()>       onClose;
        // Advanced menu
        std::function<void()>       onScreenshot;
        std::function<void()>       onStatistics;
        std::function<void()>       onRecordingSettings;
        std::function<void(bool)>   onStartRecording;   // true = start
        std::function<void(bool)>   onPauseVideo;
        std::function<void(bool)>   onPauseAudio;
        std::function<void()>       onPasteClipboard;
        std::function<void(bool)>   onSendKeyCombos;
        std::function<void(bool)>   onAutoscroll;
        std::function<void(int)>    onScale;            // IDM_DT_SCALE_* value
    };

    explicit DesktopToolbar(HINSTANCE instance);
    ~DesktopToolbar();

    DesktopToolbar(const DesktopToolbar&) = delete;
    DesktopToolbar& operator=(const DesktopToolbar&) = delete;

    // Creates the toolbar as a child of |parent|. The toolbar positions
    // itself at the top-centre of the parent's client area.
    bool create(HWND parent);

    // Show / hide the toolbar (called when the mouse enters/leaves the
    // auto-hide zone at the top of the desktop window).
    void setVisible(bool visible);

    // Force-reposition at the top-centre of the parent.
    void reposition();

    // Update checked / enabled state of individual buttons.
    void setPinned(bool pinned);
    void setFullscreen(bool fullscreen);
    void setRecording(bool recording);
    void enableCad(bool enable);
    void enableSwitchSession(bool enable);
    void enablePowerControl(bool enable);
    void enableFileTransfer(bool enable);
    void enableTextChat(bool enable);
    void enableTaskManager(bool enable);
    void enableSystemInfo(bool enable);

    void setDelegate(Delegate delegate) { delegate_ = std::move(delegate); }

    bool isPinned()     const { return pinned_; }
    bool isFullscreen() const { return fullscreen_; }
    HWND handle()       const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onCommand(int cmdId);
    void onDestroy();

    void createToolbar();
    void addButton(int cmdId, int imageIdx, bool checkable = false);
    void addSeparator();

    void showPowerMenu();
    void showAdvancedMenu();

    HINSTANCE instance_;
    HWND      hwnd_    = nullptr;
    HWND      toolbar_ = nullptr;
    Delegate  delegate_;

    bool pinned_        = false;
    bool fullscreen_    = false;
    bool recording_     = false;
    bool pauseVideo_    = false;
    bool pauseAudio_    = false;
    bool sendKeyCombos_ = false;
    bool autoscroll_    = false;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32DesktopToolbar";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_DESKTOP_TOOLBAR_H
