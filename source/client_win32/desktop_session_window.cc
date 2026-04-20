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

#include "client_win32/desktop_session_window.h"

#include "client_win32/desktop_toolbar.h"
#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

// Initial window size.
constexpr int kDefaultWidth  = 1024;
constexpr int kDefaultHeight = 768;

// Minimum window size.
constexpr int kMinWidth  = 500;
constexpr int kMinHeight = 400;

// Auto-hide strip height at the top of the window (pixels) that triggers
// showing the toolbar when the mouse enters it.
constexpr int kAutoHideStripHeight = 4;

// Scroll increment per SB_LINEUP/SB_LINEDOWN.
constexpr int kScrollLine = 20;

// ---------------------------------------------------------------------------
// Canvas background colour (matches the Qt viewport stylesheet:
// "background-color: rgb(25, 25, 25)").
// ---------------------------------------------------------------------------
constexpr COLORREF kCanvasBackground = RGB(25, 25, 25);

// ---------------------------------------------------------------------------
// Helper: clamp a scroll position.
// ---------------------------------------------------------------------------
int clampScroll(int pos, int pageSize, int maxRange)
{
    const int limit = maxRange - pageSize;
    if (limit <= 0) return 0;
    if (pos < 0)     return 0;
    if (pos > limit) return limit;
    return pos;
}

}  // namespace

// ===========================================================================
// DesktopSessionWindow
// ===========================================================================

DesktopSessionWindow::DesktopSessionWindow(HINSTANCE instance)
    : instance_(instance),
      toolbar_(std::make_unique<DesktopToolbar>(instance))
{
}

DesktopSessionWindow::~DesktopSessionWindow() = default;

bool DesktopSessionWindow::create()
{
    // --- Register main window class ---
    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &DesktopSessionWindow::windowProc;
    wc.hInstance     = instance_;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(kCanvasBackground);
    wc.lpszClassName = kClassName;
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm       = LoadIconW(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    // --- Register canvas class ---
    WNDCLASSEXW cwc  = {};
    cwc.cbSize        = sizeof(cwc);
    cwc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    cwc.lpfnWndProc   = &DesktopSessionWindow::canvasProc;
    cwc.hInstance     = instance_;
    cwc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    cwc.hbrBackground = CreateSolidBrush(kCanvasBackground);
    cwc.lpszClassName = kCanvasClassName;

    if (!RegisterClassExW(&cwc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    // --- Create main window ---
    hwnd_ = CreateWindowExW(
        WS_EX_APPWINDOW,
        kClassName, L"Aspia Remote Desktop",
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL |
            WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT,
        kDefaultWidth, kDefaultHeight,
        nullptr, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void DesktopSessionWindow::show(int showCmd)
{
    if (hwnd_)
    {
        ShowWindow(hwnd_, showCmd);
        UpdateWindow(hwnd_);
    }
}

void DesktopSessionWindow::close()
{
    if (hwnd_)
        PostMessageW(hwnd_, WM_CLOSE, 0, 0);
}

void DesktopSessionWindow::setTitle(const wchar_t* title)
{
    if (hwnd_)
        SetWindowTextW(hwnd_, title);
}

void DesktopSessionWindow::setFrame(
    const void* bits, int width, int height)
{
    frameBits_   = bits;
    frameWidth_  = width;
    frameHeight_ = height;

    if (canvas_)
        InvalidateRect(canvas_, nullptr, FALSE);
}

void DesktopSessionWindow::setRemoteScreenSize(int width, int height)
{
    frameWidth_  = width;
    frameHeight_ = height;
    scrollX_     = 0;
    scrollY_     = 0;

    if (hwnd_)
    {
        RECT rc = {};
        GetClientRect(hwnd_, &rc);
        updateScrollBars(rc.right, rc.bottom);
        layoutCanvas(rc.right, rc.bottom);
    }
}

void DesktopSessionWindow::enableCad(bool enable)
{
    if (toolbar_) toolbar_->enableCad(enable);
}
void DesktopSessionWindow::enableSwitchSession(bool enable)
{
    if (toolbar_) toolbar_->enableSwitchSession(enable);
}
void DesktopSessionWindow::enablePowerControl(bool enable)
{
    if (toolbar_) toolbar_->enablePowerControl(enable);
}
void DesktopSessionWindow::enableFileTransfer(bool enable)
{
    if (toolbar_) toolbar_->enableFileTransfer(enable);
}
void DesktopSessionWindow::enableTextChat(bool enable)
{
    if (toolbar_) toolbar_->enableTextChat(enable);
}
void DesktopSessionWindow::enableTaskManager(bool enable)
{
    if (toolbar_) toolbar_->enableTaskManager(enable);
}
void DesktopSessionWindow::enableSystemInfo(bool enable)
{
    if (toolbar_) toolbar_->enableSystemInfo(enable);
}

bool DesktopSessionWindow::preTranslateMessage(MSG* msg)
{
    // Forward keyboard messages directed at the canvas to the session layer.
    if (msg && msg->hwnd == canvas_ &&
        (msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
         msg->message == WM_SYSKEYDOWN || msg->message == WM_SYSKEYUP))
    {
        if (delegate_.onKeyEvent)
            delegate_.onKeyEvent(msg->wParam, msg->lParam);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Static window proc (main window)
// ---------------------------------------------------------------------------
// static
LRESULT CALLBACK DesktopSessionWindow::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    DesktopSessionWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<DesktopSessionWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<DesktopSessionWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT DesktopSessionWindow::handleMessage(
    UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_SIZE:
            if (wp != SIZE_MINIMIZED)
                onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        case WM_GETMINMAXINFO:
        {
            auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
            mmi->ptMinTrackSize.x = kMinWidth;
            mmi->ptMinTrackSize.y = kMinHeight;
            return 0;
        }

        case WM_HSCROLL:
            onHScroll(wp);
            return 0;

        case WM_VSCROLL:
            onVScroll(wp);
            return 0;

        case WM_MOUSEWHEEL:
            onMouseWheel(wp, lp);
            return 0;

        case WM_SYSCOMMAND:
            // Intercept SC_MINIMIZE to pause video if requested.
            if ((wp & 0xFFF0) == SC_MINIMIZE)
            {
                if (delegate_.onMinimized) delegate_.onMinimized(true);
            }
            else if ((wp & 0xFFF0) == SC_RESTORE)
            {
                if (delegate_.onMinimized) delegate_.onMinimized(false);
            }
            return DefWindowProcW(hwnd_, msg, wp, lp);

        case WM_CLOSE:
            if (delegate_.onClose) delegate_.onClose();
            DestroyWindow(hwnd_);
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

// ---------------------------------------------------------------------------
// Static window proc (canvas)
// ---------------------------------------------------------------------------
// static
LRESULT CALLBACK DesktopSessionWindow::canvasProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    DesktopSessionWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        // The canvas is created with lpCreateParams pointing to the parent
        // DesktopSessionWindow instance.
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<DesktopSessionWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<DesktopSessionWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleCanvasMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT DesktopSessionWindow::handleCanvasMessage(
    UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps = {};
            HDC dc = BeginPaint(canvas_, &ps);

            if (frameBits_ && frameWidth_ > 0 && frameHeight_ > 0)
            {
                BITMAPINFO bmi    = {};
                bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth       = frameWidth_;
                // Negative height = top-down DIB (matches aspia's frame format).
                bmi.bmiHeader.biHeight      = -frameHeight_;
                bmi.bmiHeader.biPlanes      = 1;
                bmi.bmiHeader.biBitCount    = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                StretchDIBits(dc,
                              -scrollX_, -scrollY_,
                              frameWidth_, frameHeight_,
                              0, 0, frameWidth_, frameHeight_,
                              frameBits_, &bmi,
                              DIB_RGB_COLORS, SRCCOPY);
            }
            else
            {
                // No frame yet: fill with the background colour.
                RECT rc = {};
                GetClientRect(canvas_, &rc);
                HBRUSH bg = CreateSolidBrush(kCanvasBackground);
                FillRect(dc, &rc, bg);
                DeleteObject(bg);
            }

            EndPaint(canvas_, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEMOVE:
            if (delegate_.onMouseEvent)
                delegate_.onMouseEvent(msg, wp, lp);
            return 0;

        default:
            return DefWindowProcW(canvas_, msg, wp, lp);
    }
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void DesktopSessionWindow::onCreate()
{
    createCanvas();
    createToolbar();
}

void DesktopSessionWindow::createCanvas()
{
    canvas_ = CreateWindowExW(
        0, kCanvasClassName, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_DESKTOP_WIN_CANVAS)),
        instance_, this);
}

void DesktopSessionWindow::createToolbar()
{
    if (!toolbar_)
        return;

    DesktopToolbar::Delegate tbDelegate;
    tbDelegate.onClose          = [this]() { onToolbarClose(); };
    tbDelegate.onMinimize       = [this]() { onToolbarMinimize(); };
    tbDelegate.onFullscreen     = [this](bool on) { onToolbarFullscreen(on); };
    tbDelegate.onAutoSize       = [this]() { onToolbarAutoSize(); };
    tbDelegate.onSettings       = [this]()
    {
        if (delegate_.onSettingsRequested) delegate_.onSettingsRequested();
    };
    tbDelegate.onPowerControl   = [this](int id)
    {
        if (delegate_.onPowerControl) delegate_.onPowerControl(id);
    };
    tbDelegate.onFileTransfer   = [this]()
    {
        if (delegate_.onFileTransferRequested) delegate_.onFileTransferRequested();
    };
    tbDelegate.onTextChat       = [this]()
    {
        if (delegate_.onTextChatRequested) delegate_.onTextChatRequested();
    };
    tbDelegate.onTaskManager    = [this]()
    {
        if (delegate_.onTaskManagerRequested) delegate_.onTaskManagerRequested();
    };
    tbDelegate.onSystemInfo     = [this]()
    {
        if (delegate_.onSystemInfoRequested) delegate_.onSystemInfoRequested();
    };
    tbDelegate.onSwitchSession  = [this]()
    {
        if (delegate_.onSwitchSessionRequested) delegate_.onSwitchSessionRequested();
    };
    tbDelegate.onStatistics     = [this]()
    {
        if (delegate_.onStatisticsRequested) delegate_.onStatisticsRequested();
    };

    toolbar_->setDelegate(std::move(tbDelegate));
    toolbar_->create(hwnd_);
    // Start pinned so the toolbar is visible by default.
    toolbar_->setPinned(true);
    toolbar_->setVisible(true);
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

void DesktopSessionWindow::onSize(int width, int height)
{
    clientW_ = width;
    clientH_ = height;

    updateScrollBars(width, height);
    layoutCanvas(width, height);

    if (toolbar_)
        toolbar_->reposition();
}

void DesktopSessionWindow::layoutCanvas(int winW, int winH)
{
    if (!canvas_)
        return;

    // If the remote frame fits inside the window, centre it; otherwise the
    // canvas takes the full client area and scroll bars handle the offset.
    const int cw = (frameWidth_  > 0 && frameWidth_  < winW)
                       ? frameWidth_  : winW;
    const int ch = (frameHeight_ > 0 && frameHeight_ < winH)
                       ? frameHeight_ : winH;

    const int cx = (frameWidth_  > 0 && frameWidth_  < winW)
                       ? (winW - frameWidth_)  / 2 : 0;
    const int cy = (frameHeight_ > 0 && frameHeight_ < winH)
                       ? (winH - frameHeight_) / 2 : 0;

    SetWindowPos(canvas_, nullptr, cx, cy, cw, ch, SWP_NOZORDER | SWP_NOACTIVATE);
}

void DesktopSessionWindow::updateScrollBars(int winW, int winH)
{
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;

    // Horizontal.
    si.nMin  = 0;
    si.nMax  = (frameWidth_ > winW) ? frameWidth_  - 1 : 0;
    si.nPage = static_cast<UINT>(winW);
    si.nPos  = scrollX_;
    SetScrollInfo(hwnd_, SB_HORZ, &si, TRUE);

    // Vertical.
    si.nMax  = (frameHeight_ > winH) ? frameHeight_ - 1 : 0;
    si.nPage = static_cast<UINT>(winH);
    si.nPos  = scrollY_;
    SetScrollInfo(hwnd_, SB_VERT, &si, TRUE);
}

// ---------------------------------------------------------------------------
// Scrolling
// ---------------------------------------------------------------------------

void DesktopSessionWindow::onHScroll(WPARAM wp)
{
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask  = SIF_ALL;
    GetScrollInfo(hwnd_, SB_HORZ, &si);

    const int pageSize = static_cast<int>(si.nPage);
    int newPos = si.nPos;

    switch (LOWORD(wp))
    {
        case SB_LINELEFT:       newPos -= kScrollLine;           break;
        case SB_LINERIGHT:      newPos += kScrollLine;           break;
        case SB_PAGELEFT:       newPos -= pageSize;              break;
        case SB_PAGERIGHT:      newPos += pageSize;              break;
        case SB_THUMBTRACK:     newPos = si.nTrackPos;           break;
        case SB_LEFT:           newPos = si.nMin;                break;
        case SB_RIGHT:          newPos = si.nMax;                break;
        default: return;
    }

    newPos = clampScroll(newPos, pageSize, si.nMax + 1);
    if (newPos == scrollX_) return;

    scrollX_ = newPos;
    SetScrollPos(hwnd_, SB_HORZ, scrollX_, TRUE);
    if (canvas_) InvalidateRect(canvas_, nullptr, FALSE);
}

void DesktopSessionWindow::onVScroll(WPARAM wp)
{
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask  = SIF_ALL;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    const int pageSize = static_cast<int>(si.nPage);
    int newPos = si.nPos;

    switch (LOWORD(wp))
    {
        case SB_LINEUP:         newPos -= kScrollLine;           break;
        case SB_LINEDOWN:       newPos += kScrollLine;           break;
        case SB_PAGEUP:         newPos -= pageSize;              break;
        case SB_PAGEDOWN:       newPos += pageSize;              break;
        case SB_THUMBTRACK:     newPos = si.nTrackPos;           break;
        case SB_TOP:            newPos = si.nMin;                break;
        case SB_BOTTOM:         newPos = si.nMax;                break;
        default: return;
    }

    newPos = clampScroll(newPos, pageSize, si.nMax + 1);
    if (newPos == scrollY_) return;

    scrollY_ = newPos;
    SetScrollPos(hwnd_, SB_VERT, scrollY_, TRUE);
    if (canvas_) InvalidateRect(canvas_, nullptr, FALSE);
}

void DesktopSessionWindow::onMouseWheel(WPARAM wp, LPARAM /*lp*/)
{
    const int delta = GET_WHEEL_DELTA_WPARAM(wp);

    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask  = SIF_ALL;
    GetScrollInfo(hwnd_, SB_VERT, &si);

    const int pageSize = static_cast<int>(si.nPage);
    // Three lines per notch (matches Qt default).
    int newPos = si.nPos - (delta / WHEEL_DELTA) * kScrollLine * 3;
    newPos = clampScroll(newPos, pageSize, si.nMax + 1);
    if (newPos == scrollY_) return;

    scrollY_ = newPos;
    SetScrollPos(hwnd_, SB_VERT, scrollY_, TRUE);
    if (canvas_) InvalidateRect(canvas_, nullptr, FALSE);
}

// ---------------------------------------------------------------------------
// Toolbar callbacks
// ---------------------------------------------------------------------------

void DesktopSessionWindow::onToolbarClose()
{
    if (delegate_.onClose) delegate_.onClose();
    DestroyWindow(hwnd_);
}

void DesktopSessionWindow::onToolbarMinimize()
{
    if (hwnd_)
    {
        ShowWindow(hwnd_, SW_MINIMIZE);
        if (delegate_.onMinimized) delegate_.onMinimized(true);
    }
}

void DesktopSessionWindow::onToolbarFullscreen(bool enter)
{
    fullscreen_ = enter;

    if (enter)
    {
        // Save current window placement for restore.
        GetWindowRect(hwnd_, &savedWindowRect_);

        const HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(monitor, &mi);

        SetWindowLongW(hwnd_, GWL_STYLE,
                       WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowPos(hwnd_, HWND_TOP,
                     mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right  - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED);
    }
    else
    {
        SetWindowLongW(hwnd_, GWL_STYLE,
                       WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL |
                           WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowPos(hwnd_, nullptr,
                     savedWindowRect_.left, savedWindowRect_.top,
                     savedWindowRect_.right  - savedWindowRect_.left,
                     savedWindowRect_.bottom - savedWindowRect_.top,
                     SWP_FRAMECHANGED | SWP_NOZORDER);
    }

    if (toolbar_) toolbar_->setFullscreen(enter);
}

void DesktopSessionWindow::onToolbarAutoSize()
{
    if (frameWidth_ <= 0 || frameHeight_ <= 0)
        return;

    // Resize the window so that the canvas exactly fits the remote frame.
    RECT rc = { 0, 0, frameWidth_, frameHeight_ };
    AdjustWindowRectEx(&rc, GetWindowLongW(hwnd_, GWL_STYLE), FALSE,
                       GetWindowLongW(hwnd_, GWL_EXSTYLE));

    SetWindowPos(hwnd_, nullptr,
                 0, 0,
                 rc.right  - rc.left,
                 rc.bottom - rc.top,
                 SWP_NOZORDER | SWP_NOMOVE);
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

void DesktopSessionWindow::onDestroy()
{
    PostQuitMessage(0);
}

}  // namespace aspia::client_win32
