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
// Phase-2 adapter: bridges client::ClientDesktop (Qt signals/slots) to the
// Win32 DesktopSessionWindow (delegate callbacks).
//

#include "client_win32/desktop_session_adapter.h"

#include "base/desktop/frame.h"
#include "client/client_desktop.h"
#include "client_win32/desktop_session_window.h"
#include "proto/desktop_control.h"
#include "proto/desktop_cursor.h"
#include "proto/desktop_input.h"
#include "proto/desktop_power.h"
#include "proto/desktop_screen.h"

#include <QObject>

#include <windows.h>

namespace aspia::client_win32 {

namespace {

// ---------------------------------------------------------------------------
// Mouse button translation helpers
// ---------------------------------------------------------------------------

// Translate a Win32 mouse message + wParam into a proto::input::MouseEvent
// button mask.
static uint32_t mouseMaskFromWin32(UINT msg, WPARAM wp)
{
    uint32_t mask = 0;

    // Buttons that are held down (from MK_* flags in wParam).
    if (wp & MK_LBUTTON)
        mask |= proto::input::MouseEvent::LEFT_BUTTON;
    if (wp & MK_RBUTTON)
        mask |= proto::input::MouseEvent::RIGHT_BUTTON;
    if (wp & MK_MBUTTON)
        mask |= proto::input::MouseEvent::MIDDLE_BUTTON;
    if (wp & MK_XBUTTON1)
        mask |= proto::input::MouseEvent::BACK_BUTTON;
    if (wp & MK_XBUTTON2)
        mask |= proto::input::MouseEvent::FORWARD_BUTTON;

    // Wheel events — not expressed as held-down bits.
    if (msg == WM_MOUSEWHEEL)
    {
        short delta = static_cast<short>(HIWORD(wp));
        if (delta > 0)
            mask |= proto::input::MouseEvent::WHEEL_UP;
        else
            mask |= proto::input::MouseEvent::WHEEL_DOWN;
    }

    return mask;
}

} // namespace

// ---------------------------------------------------------------------------
// DesktopSessionAdapter
// ---------------------------------------------------------------------------

DesktopSessionAdapter::DesktopSessionAdapter(client::ClientDesktop* client,
                                             DesktopSessionWindow* window,
                                             QObject* parent)
    : QObject(parent),
      client_(client),
      window_(window)
{
    Q_ASSERT(client_);
    Q_ASSERT(window_);

    // Connect Qt signals from the session client to our local slots.
    connect(client_, &client::ClientDesktop::sig_capabilities,
            this, &DesktopSessionAdapter::onCapabilities,
            Qt::QueuedConnection);

    connect(client_, &client::ClientDesktop::sig_screenListChanged,
            this, &DesktopSessionAdapter::onScreenListChanged,
            Qt::QueuedConnection);

    connect(client_, &client::ClientDesktop::sig_frameChanged,
            this, &DesktopSessionAdapter::onFrameChanged,
            Qt::QueuedConnection);

    connect(client_, &client::ClientDesktop::sig_drawFrame,
            this, &DesktopSessionAdapter::onDrawFrame,
            Qt::QueuedConnection);

    connect(client_, &client::ClientDesktop::sig_metrics,
            this, &DesktopSessionAdapter::onMetrics,
            Qt::QueuedConnection);

    connect(client_, &client::ClientDesktop::sig_sessionListChanged,
            this, &DesktopSessionAdapter::onSessionListChanged,
            Qt::QueuedConnection);

    wireWindowDelegate();
}

DesktopSessionAdapter::~DesktopSessionAdapter() = default;

// ---------------------------------------------------------------------------
// Qt signal handlers (called on the main/Qt thread)
// ---------------------------------------------------------------------------

void DesktopSessionAdapter::onCapabilities(const proto::control::Capabilities& caps)
{
    // Walk the named flags and enable the corresponding toolbar buttons.
    for (int i = 0; i < caps.flag_size(); ++i)
    {
        const auto& flag = caps.flag(i);
        if (flag.name() == "cad")
            window_->enableCad(flag.value());
        else if (flag.name() == "switch_session")
            window_->enableSwitchSession(flag.value());
        else if (flag.name() == "power_control")
            window_->enablePowerControl(flag.value());
        else if (flag.name() == "file_transfer")
            window_->enableFileTransfer(flag.value());
        else if (flag.name() == "text_chat")
            window_->enableTextChat(flag.value());
        else if (flag.name() == "task_manager")
            window_->enableTaskManager(flag.value());
        else if (flag.name() == "system_info")
            window_->enableSystemInfo(flag.value());
    }
}

void DesktopSessionAdapter::onScreenListChanged(const proto::screen::ScreenList& /*list*/)
{
    // Phase 3: populate screen-selector combo in the toolbar.
}

void DesktopSessionAdapter::onFrameChanged(const QSize& size,
                                           std::shared_ptr<base::Frame> frame)
{
    if (!frame)
        return;

    last_frame_ = frame;
    window_->setFrame(frame->frameData(), size.width(), size.height());
    window_->setRemoteScreenSize(size.width(), size.height());
}

void DesktopSessionAdapter::onDrawFrame()
{
    HWND canvas = window_->canvas();
    if (canvas && IsWindow(canvas))
        InvalidateRect(canvas, nullptr, FALSE);
}

void DesktopSessionAdapter::onMetrics(const client::ClientDesktop::Metrics& /*metrics*/)
{
    // Phase 3: update the statistics dialog.
}

void DesktopSessionAdapter::onSessionListChanged(const proto::control::SessionList& /*sessions*/)
{
    // Phase 3: populate session-switch UI.
}

// ---------------------------------------------------------------------------
// Win32 window delegate wiring
// ---------------------------------------------------------------------------

void DesktopSessionAdapter::wireWindowDelegate()
{
    DesktopSessionWindow::Delegate d;

    // Settings (desktop config dialog) — Phase 3.
    d.onSettingsRequested = []() {};

    // Minimise / restore pause — forward to video pause.
    d.onMinimized = [this](bool minimized)
    {
        QMetaObject::invokeMethod(client_, "onVideoPauseChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, minimized));
    };

    // Close — handled at the App level; nothing to do here.
    d.onClose = []() {};

    // Power control: the toolbar forwards an IDM_DT_* menu command id.
    // Map known ids to proto::power::Control::Action values.
    d.onPowerControl = [this](int cmdId)
    {
        proto::power::Control_Action action = proto::power::Control::ACTION_UNKNOWN;

        // IDM values are defined in resource.h; use their numeric range.
        // The Win32 toolbar sends the raw command id; map them as agreed:
        //   8101 = IDM_DT_POWER_SHUTDOWN
        //   8102 = IDM_DT_POWER_REBOOT
        //   8103 = IDM_DT_POWER_LOGOFF
        //   8104 = IDM_DT_POWER_LOCK
        //   8105 = IDM_DT_POWER_REBOOT_SAFE_MODE
        switch (cmdId)
        {
            case 8101: action = proto::power::Control::ACTION_SHUTDOWN;         break;
            case 8102: action = proto::power::Control::ACTION_REBOOT;           break;
            case 8103: action = proto::power::Control::ACTION_LOGOFF;           break;
            case 8104: action = proto::power::Control::ACTION_LOCK;             break;
            case 8105: action = proto::power::Control::ACTION_REBOOT_SAFE_MODE; break;
            default:   break;
        }

        if (action != proto::power::Control::ACTION_UNKNOWN)
        {
            QMetaObject::invokeMethod(client_, "onPowerControl",
                                      Qt::QueuedConnection,
                                      Q_ARG(proto::power::Control_Action, action));
        }
    };

    // Secondary session requests — Phase 3.
    d.onFileTransferRequested  = []() {};
    d.onTextChatRequested      = []() {};
    d.onTaskManagerRequested   = []() {};
    d.onSystemInfoRequested    = []() {};
    d.onSwitchSessionRequested = []() {};
    d.onStatisticsRequested    = []() {};

    // -----------------------------------------------------------------------
    // Key events
    // -----------------------------------------------------------------------
    // The Win32 window passes (WPARAM wVirtualKey, LPARAM lParam) from
    // WM_KEYDOWN / WM_SYSKEYDOWN / WM_KEYUP / WM_SYSKEYUP.
    // proto::input::KeyEvent uses a USB HID keycode.  We store the VK in the
    // usb_keycode field for now (the host side already handles VK→USB mapping
    // on Windows); flags carry PRESSED if it is a key-down event.
    //
    // lParam bit 31: 0 = key pressed, 1 = key released.
    // lParam bit 24: 1 = extended key (right-hand Ctrl/Alt, numpad /, Print Screen).
    d.onKeyEvent = [this](WPARAM wVirtualKey, LPARAM lParam)
    {
        proto::input::KeyEvent event;
        event.set_usb_keycode(static_cast<uint32_t>(wVirtualKey));

        uint32_t flags = 0;
        bool key_down = ((lParam >> 31) & 1) == 0; // bit 31 = 0 means key-down
        if (key_down)
            flags |= proto::input::KeyEvent::PRESSED;

        // Capture Caps/Num lock state.
        if (GetKeyState(VK_CAPITAL) & 1)
            flags |= proto::input::KeyEvent::CAPSLOCK;
        if (GetKeyState(VK_NUMLOCK) & 1)
            flags |= proto::input::KeyEvent::NUMLOCK;

        event.set_flags(flags);

        QMetaObject::invokeMethod(client_, "onKeyEvent",
                                  Qt::QueuedConnection,
                                  Q_ARG(proto::input::KeyEvent, event));
    };

    // -----------------------------------------------------------------------
    // Mouse events
    // -----------------------------------------------------------------------
    // The Win32 window passes (UINT msg, WPARAM wParam, LPARAM lParam) from
    // the canvas WM_LBUTTONDOWN / WM_MOUSEMOVE / WM_MOUSEWHEEL etc.
    d.onMouseEvent = [this](UINT msg, WPARAM wp, LPARAM lp)
    {
        proto::input::MouseEvent event;
        event.set_x(GET_X_LPARAM(lp));
        event.set_y(GET_Y_LPARAM(lp));
        event.set_mask(mouseMaskFromWin32(msg, wp));

        QMetaObject::invokeMethod(client_, "onMouseEvent",
                                  Qt::QueuedConnection,
                                  Q_ARG(proto::input::MouseEvent, event));
    };

    window_->setDelegate(std::move(d));
}

} // namespace aspia::client_win32
