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
// Win32 replacement for client/ui/desktop/desktop_session_window.ui/.cc.
//
// The Qt original is a QWidget-derived SessionWindow that:
//   - Hosts a DesktopWidget (the raw render surface) inside a QScrollArea.
//   - Hosts a DesktopToolBar (auto-hiding overlay at the top).
//   - Forwards keyboard / mouse events from the render surface to the
//     ClientDesktop session object as proto::input::KeyEvent /
//     proto::input::MouseEvent messages.
//   - Supports fullscreen, scale, video/audio pause, session switching,
//     power-control, file transfer, text chat, task manager, sys-info.
//
// The Win32 skeleton follows the same structure:
//   - A top-level OVERLAPPEDWINDOW.
//   - A canvas child window (IDC_DESKTOP_WIN_CANVAS = 8300) that fills
//     the client area (minus scroll bars when the remote frame is larger
//     than the window). Rendering is done via WM_PAINT on the canvas.
//   - A DesktopToolbar child window floating at the top-centre.
//   - Scroll bars (WS_HSCROLL | WS_VSCROLL) on the main window handle
//     horizontal and vertical scrolling.
//
// Input events (keyboard / mouse) are forwarded through the Delegate
// interface. Proto wiring (ClientDesktop) is left for phase 2.
//

#ifndef CLIENT_WIN32_DESKTOP_SESSION_WINDOW_H
#define CLIENT_WIN32_DESKTOP_SESSION_WINDOW_H

#include <windows.h>

#include <functional>
#include <memory>
#include <string>

namespace aspia::client_win32 {

class DesktopToolbar;

class DesktopSessionWindow
{
public:
    // Callbacks from the window to the session layer (phase 2 wiring).
    struct Delegate
    {
        // Called when the user changes desktop config via the toolbar.
        std::function<void()>       onSettingsRequested;
        // Called when the user closes the session window.
        std::function<void()>       onClose;
        // Called when the window is minimised / restored (for pausing video).
        std::function<void(bool)>   onMinimized;      // true = minimised
        // Power control (IDM_DT_* value forwarded from the toolbar).
        std::function<void(int)>    onPowerControl;
        // Secondary sessions.
        std::function<void()>       onFileTransferRequested;
        std::function<void()>       onTextChatRequested;
        std::function<void()>       onTaskManagerRequested;
        std::function<void()>       onSystemInfoRequested;
        std::function<void()>       onSwitchSessionRequested;
        std::function<void()>       onStatisticsRequested;
        // Input events (raw Win32 WPARAM/LPARAM, translated in phase 2).
        std::function<void(WPARAM, LPARAM)> onKeyEvent;
        std::function<void(UINT, WPARAM, LPARAM)> onMouseEvent;
    };

    explicit DesktopSessionWindow(HINSTANCE instance);
    ~DesktopSessionWindow();

    DesktopSessionWindow(const DesktopSessionWindow&) = delete;
    DesktopSessionWindow& operator=(const DesktopSessionWindow&) = delete;

    bool create();
    void show(int showCmd = SW_SHOW);
    void close();

    // Update the window title (e.g. "host:port — Aspia Remote Desktop").
    void setTitle(const wchar_t* title);

    // Called by the session layer to deliver a rendered frame.
    // |bits| is a pointer to a DIB section (top-down, 32-bpp BGR).
    // The window stores the pointer and invalidates the canvas.
    void setFrame(const void* bits, int width, int height);

    // Resize the canvas to match the remote screen size (may add scroll bars).
    void setRemoteScreenSize(int width, int height);

    // Enable / disable toolbar features based on session capabilities.
    void enableCad(bool enable);
    void enableSwitchSession(bool enable);
    void enablePowerControl(bool enable);
    void enableFileTransfer(bool enable);
    void enableTextChat(bool enable);
    void enableTaskManager(bool enable);
    void enableSystemInfo(bool enable);

    void setDelegate(Delegate delegate) { delegate_ = std::move(delegate); }

    HWND handle()  const { return hwnd_; }
    HWND canvas()  const { return canvas_; }

    bool preTranslateMessage(MSG* msg);

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    // Canvas sub-window (IDC_DESKTOP_WIN_CANVAS).
    static LRESULT CALLBACK canvasProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleCanvasMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onHScroll(WPARAM wp);
    void onVScroll(WPARAM wp);
    void onMouseWheel(WPARAM wp, LPARAM lp);
    void onChangeState(WPARAM wp);
    void onDestroy();

    void createCanvas();
    void createToolbar();
    void layoutCanvas(int winW, int winH);
    void updateScrollBars(int winW, int winH);

    void onToolbarClose();
    void onToolbarMinimize();
    void onToolbarFullscreen(bool enter);
    void onToolbarAutoSize();

    // Scroll helpers.
    int  scrollLine(int barType, int request, int current, int pageSize, int max);

    HINSTANCE instance_;
    HWND      hwnd_    = nullptr;
    HWND      canvas_  = nullptr;

    std::unique_ptr<DesktopToolbar> toolbar_;
    Delegate  delegate_;

    // Remote frame (owned externally; pointer valid until next setFrame call).
    const void* frameBits_   = nullptr;
    int         frameWidth_  = 0;
    int         frameHeight_ = 0;

    // Scroll state.
    int scrollX_ = 0;
    int scrollY_ = 0;

    // Current window client size.
    int clientW_ = 0;
    int clientH_ = 0;

    bool fullscreen_  = false;
    bool autoHide_    = true;   // toolbar auto-hides when not pinned
    RECT savedWindowRect_ = {}; // for fullscreen restore

    static constexpr wchar_t kClassName[]       = L"AspiaClientWin32DesktopSession";
    static constexpr wchar_t kCanvasClassName[] = L"AspiaClientWin32DesktopCanvas";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_DESKTOP_SESSION_WINDOW_H
