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

#include "client_win32/desktop_toolbar.h"

#include "client_win32/resource.h"

namespace aspia::client_win32 {

namespace {

// Toolbar button size in pixels.
constexpr int kBtnSize = 24;

// Tooltip strings indexed by button command ID.
struct BtnDef
{
    int          cmdId;
    const wchar_t* tooltip;
    bool         checkable;
    bool         initiallyEnabled;
};

const BtnDef kButtons[] = {
    { IDC_DT_PIN,            L"Lock toolbar",            true,  true  },
    // separator
    { IDC_DT_SWITCH_SESSION, L"Switch Session",           false, false },
    { IDC_DT_POWER_CONTROL,  L"Power control",            false, false },
    { IDC_DT_CAD,            L"Send Ctrl+Alt+Delete",     false, false },
    { IDC_DT_SETTINGS,       L"Session configuration",    false, true  },
    // separator
    { IDC_DT_AUTOSIZE,       L"Automatic window size",    false, true  },
    { IDC_DT_FULLSCREEN,     L"Switch to full screen",    true,  true  },
    // separator
    { IDC_DT_FILE_TRANSFER,  L"File transfer",            false, false },
    { IDC_DT_TEXT_CHAT,      L"Text Chat",                false, false },
    { IDC_DT_TASK_MANAGER,   L"Task Manager",             false, false },
    { IDC_DT_SYSTEM_INFO,    L"System Information",       false, false },
    // separator
    { IDC_DT_MENU,           L"Advanced menu",            false, true  },
    // separator
    { IDC_DT_MINIMIZE,       L"Minimize window",          false, true  },
    { IDC_DT_CLOSE,          L"Close session",            false, true  },
};

// Separator placeholder sentinel (cmdId=-1).
constexpr int kSepSentinel = -1;

// Build the ordered button sequence (including sentinel separators).
struct BtnSeq
{
    int  cmdId;    // -1 = separator
    bool checkable;
    bool initiallyEnabled;
};

const BtnSeq kSeq[] = {
    { IDC_DT_PIN,            true,  true  },
    { kSepSentinel,          false, false },
    { IDC_DT_SWITCH_SESSION, false, false },
    { IDC_DT_POWER_CONTROL,  false, false },
    { IDC_DT_CAD,            false, false },
    { IDC_DT_SETTINGS,       false, true  },
    { kSepSentinel,          false, false },
    { IDC_DT_AUTOSIZE,       false, true  },
    { IDC_DT_FULLSCREEN,     true,  true  },
    { kSepSentinel,          false, false },
    { IDC_DT_FILE_TRANSFER,  false, false },
    { IDC_DT_TEXT_CHAT,      false, false },
    { IDC_DT_TASK_MANAGER,   false, false },
    { IDC_DT_SYSTEM_INFO,    false, false },
    { kSepSentinel,          false, false },
    { IDC_DT_MENU,           false, true  },
    { kSepSentinel,          false, false },
    { IDC_DT_MINIMIZE,       false, true  },
    { IDC_DT_CLOSE,          false, true  },
};

constexpr int kSeqCount = static_cast<int>(sizeof(kSeq) / sizeof(kSeq[0]));

// Lookup tooltip for a command ID.
const wchar_t* tooltipForCmd(int cmdId)
{
    for (const BtnDef& d : kButtons)
        if (d.cmdId == cmdId) return d.tooltip;
    return L"";
}

}  // namespace

DesktopToolbar::DesktopToolbar(HINSTANCE instance)
    : instance_(instance)
{
}

DesktopToolbar::~DesktopToolbar() = default;

bool DesktopToolbar::create(HWND parent)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &DesktopToolbar::windowProc;
    wc.hInstance     = instance_;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    // The toolbar container is a borderless child window positioned at the
    // top-centre of |parent|.
    hwnd_ = CreateWindowExW(
        0, kClassName, L"",
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0, parent, nullptr, instance_, this);

    if (!hwnd_)
        return false;

    reposition();
    return true;
}

void DesktopToolbar::setVisible(bool visible)
{
    if (hwnd_)
        ShowWindow(hwnd_, visible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void DesktopToolbar::reposition()
{
    if (!hwnd_ || !toolbar_)
        return;

    // Measure the toolbar's desired size.
    RECT tbrc = {};
    GetWindowRect(toolbar_, &tbrc);
    const int tbW = tbrc.right  - tbrc.left;
    const int tbH = tbrc.bottom - tbrc.top;

    // Centre at the top of the parent's client area.
    HWND parent = GetParent(hwnd_);
    RECT prc    = {};
    GetClientRect(parent, &prc);

    const int x = (prc.right - tbW) / 2;
    SetWindowPos(hwnd_, HWND_TOP,
                 x, 0, tbW, tbH,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SetWindowPos(toolbar_, nullptr,
                 0, 0, tbW, tbH,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

void DesktopToolbar::setPinned(bool pinned)
{
    pinned_ = pinned;
    if (toolbar_)
        SendMessageW(toolbar_, TB_CHECKBUTTON,
                     static_cast<WPARAM>(IDC_DT_PIN),
                     MAKELPARAM(pinned ? TRUE : FALSE, 0));
}

void DesktopToolbar::setFullscreen(bool fullscreen)
{
    fullscreen_ = fullscreen;
    if (toolbar_)
        SendMessageW(toolbar_, TB_CHECKBUTTON,
                     static_cast<WPARAM>(IDC_DT_FULLSCREEN),
                     MAKELPARAM(fullscreen ? TRUE : FALSE, 0));
}

void DesktopToolbar::setRecording(bool recording)
{
    recording_ = recording;
}

void DesktopToolbar::enableCad(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_CAD),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

void DesktopToolbar::enableSwitchSession(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_SWITCH_SESSION),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

void DesktopToolbar::enablePowerControl(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_POWER_CONTROL),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

void DesktopToolbar::enableFileTransfer(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_FILE_TRANSFER),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

void DesktopToolbar::enableTextChat(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_TEXT_CHAT),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

void DesktopToolbar::enableTaskManager(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_TASK_MANAGER),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

void DesktopToolbar::enableSystemInfo(bool enable)
{
    if (toolbar_)
        SendMessageW(toolbar_, TB_ENABLEBUTTON,
                     static_cast<WPARAM>(IDC_DT_SYSTEM_INFO),
                     MAKELPARAM(enable ? TRUE : FALSE, 0));
}

// static
LRESULT CALLBACK DesktopToolbar::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    DesktopToolbar* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<DesktopToolbar*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<DesktopToolbar*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT DesktopToolbar::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_COMMAND:
            onCommand(LOWORD(wp));
            return 0;

        case WM_NOTIFY:
        {
            // Forward tooltip requests from the toolbar.
            auto* hdr = reinterpret_cast<NMHDR*>(lp);
            if (hdr && hdr->code == TTN_GETDISPINFOW)
            {
                auto* disp = reinterpret_cast<NMTTDISPINFOW*>(lp);
                disp->lpszText = const_cast<wchar_t*>(
                    tooltipForCmd(static_cast<int>(disp->hdr.idFrom)));
                return 0;
            }
            return DefWindowProcW(hwnd_, msg, wp, lp);
        }

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void DesktopToolbar::onCreate()
{
    createToolbar();
}

void DesktopToolbar::createToolbar()
{
    toolbar_ = CreateWindowExW(
        0, TOOLBARCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
            TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER | CCS_NOPARENTALIGN,
        0, 0, 0, 0, hwnd_,
        nullptr, instance_, nullptr);

    SendMessageW(toolbar_, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessageW(toolbar_, TB_SETBITMAPSIZE, 0, MAKELPARAM(kBtnSize, kBtnSize));

    // Create an empty image list (icons will be added in phase 2).
    HIMAGELIST hil = ImageList_Create(kBtnSize, kBtnSize, ILC_COLOR32 | ILC_MASK, 20, 0);
    SendMessageW(toolbar_, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(hil));

    for (int i = 0; i < kSeqCount; ++i)
    {
        const BtnSeq& s = kSeq[i];
        TBBUTTON btn    = {};

        if (s.cmdId == kSepSentinel)
        {
            btn.fsStyle = BTNS_SEP;
            btn.iBitmap = 8; // separator width hint
        }
        else
        {
            btn.iBitmap   = I_IMAGENONE;
            btn.idCommand = s.cmdId;
            btn.fsState   = s.initiallyEnabled ? TBSTATE_ENABLED : 0;
            btn.fsStyle   = BTNS_BUTTON | BTNS_AUTOSIZE;
            if (s.checkable)
                btn.fsStyle |= BTNS_CHECK;
        }

        SendMessageW(toolbar_, TB_ADDBUTTONS, 1, reinterpret_cast<LPARAM>(&btn));
    }

    SendMessageW(toolbar_, TB_AUTOSIZE, 0, 0);
}

void DesktopToolbar::onCommand(int cmdId)
{
    switch (cmdId)
    {
        case IDC_DT_PIN:
            pinned_ = !pinned_;
            setPinned(pinned_);
            break;

        case IDC_DT_SWITCH_SESSION:
            if (delegate_.onSwitchSession) delegate_.onSwitchSession();
            break;

        case IDC_DT_POWER_CONTROL:
            showPowerMenu();
            break;

        case IDC_DT_CAD:
            if (delegate_.onSendCad) delegate_.onSendCad();
            break;

        case IDC_DT_SETTINGS:
            if (delegate_.onSettings) delegate_.onSettings();
            break;

        case IDC_DT_AUTOSIZE:
            if (delegate_.onAutoSize) delegate_.onAutoSize();
            break;

        case IDC_DT_FULLSCREEN:
            fullscreen_ = !fullscreen_;
            setFullscreen(fullscreen_);
            if (delegate_.onFullscreen) delegate_.onFullscreen(fullscreen_);
            break;

        case IDC_DT_FILE_TRANSFER:
            if (delegate_.onFileTransfer) delegate_.onFileTransfer();
            break;

        case IDC_DT_TEXT_CHAT:
            if (delegate_.onTextChat) delegate_.onTextChat();
            break;

        case IDC_DT_TASK_MANAGER:
            if (delegate_.onTaskManager) delegate_.onTaskManager();
            break;

        case IDC_DT_SYSTEM_INFO:
            if (delegate_.onSystemInfo) delegate_.onSystemInfo();
            break;

        case IDC_DT_MENU:
            showAdvancedMenu();
            break;

        case IDC_DT_MINIMIZE:
            if (delegate_.onMinimize) delegate_.onMinimize();
            break;

        case IDC_DT_CLOSE:
            if (delegate_.onClose) delegate_.onClose();
            break;

        // Power sub-menu items (forwarded from TrackPopupMenu).
        case IDM_DT_SHUTDOWN:
        case IDM_DT_REBOOT:
        case IDM_DT_REBOOT_SAFE_MODE:
        case IDM_DT_LOGOFF:
        case IDM_DT_LOCK:
            if (delegate_.onPowerControl) delegate_.onPowerControl(cmdId);
            break;

        // Advanced menu items.
        case IDM_DT_SCREENSHOT:
            if (delegate_.onScreenshot) delegate_.onScreenshot();
            break;

        case IDM_DT_STATISTICS:
            if (delegate_.onStatistics) delegate_.onStatistics();
            break;

        case IDM_DT_RECORDING_SETTINGS:
            if (delegate_.onRecordingSettings) delegate_.onRecordingSettings();
            break;

        case IDM_DT_START_RECORDING:
            recording_ = !recording_;
            if (delegate_.onStartRecording) delegate_.onStartRecording(recording_);
            break;

        case IDM_DT_PAUSE_VIDEO:
            pauseVideo_ = !pauseVideo_;
            if (delegate_.onPauseVideo) delegate_.onPauseVideo(pauseVideo_);
            break;

        case IDM_DT_PAUSE_AUDIO:
            pauseAudio_ = !pauseAudio_;
            if (delegate_.onPauseAudio) delegate_.onPauseAudio(pauseAudio_);
            break;

        case IDM_DT_PASTE_CLIPBOARD:
            if (delegate_.onPasteClipboard) delegate_.onPasteClipboard();
            break;

        case IDM_DT_SEND_KEY_COMBOS:
            sendKeyCombos_ = !sendKeyCombos_;
            if (delegate_.onSendKeyCombos) delegate_.onSendKeyCombos(sendKeyCombos_);
            break;

        case IDM_DT_AUTOSCROLL:
            autoscroll_ = !autoscroll_;
            if (delegate_.onAutoscroll) delegate_.onAutoscroll(autoscroll_);
            break;

        case IDM_DT_SCALE_FIT:
        case IDM_DT_SCALE_100:
        case IDM_DT_SCALE_90:
        case IDM_DT_SCALE_80:
        case IDM_DT_SCALE_70:
        case IDM_DT_SCALE_60:
        case IDM_DT_SCALE_50:
            if (delegate_.onScale) delegate_.onScale(cmdId);
            break;

        default:
            break;
    }
}

void DesktopToolbar::showPowerMenu()
{
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    AppendMenuW(menu, MF_STRING, IDM_DT_SHUTDOWN,         L"Shutdown");
    AppendMenuW(menu, MF_STRING, IDM_DT_REBOOT,           L"Reboot");
    AppendMenuW(menu, MF_STRING, IDM_DT_REBOOT_SAFE_MODE, L"Reboot (Safe mode)");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, IDM_DT_LOGOFF,           L"Logoff");
    AppendMenuW(menu, MF_STRING, IDM_DT_LOCK,             L"Lock");

    // Position below the Power Control button.
    RECT btnRect = {};
    SendMessageW(toolbar_, TB_GETRECT,
                 static_cast<WPARAM>(IDC_DT_POWER_CONTROL),
                 reinterpret_cast<LPARAM>(&btnRect));
    POINT pt = { btnRect.left, btnRect.bottom };
    ClientToScreen(toolbar_, &pt);

    TrackPopupMenu(menu,
                   TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
                   pt.x, pt.y, 0, hwnd_, nullptr);
    DestroyMenu(menu);
}

void DesktopToolbar::showAdvancedMenu()
{
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    // Scale sub-menu.
    HMENU scaleMenu = CreatePopupMenu();
    if (scaleMenu)
    {
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_FIT, L"Fit window");
        AppendMenuW(scaleMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_100, L"100%");
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_90,  L"90%");
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_80,  L"80%");
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_70,  L"70%");
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_60,  L"60%");
        AppendMenuW(scaleMenu, MF_STRING, IDM_DT_SCALE_50,  L"50%");
    }

    AppendMenuW(menu, MF_STRING,  IDM_DT_SCREENSHOT,         L"Save screenshot...");
    AppendMenuW(menu, MF_STRING,  IDM_DT_STATISTICS,         L"Statistics");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING,  IDM_DT_RECORDING_SETTINGS, L"Recording settings...");
    AppendMenuW(menu, MF_STRING,  IDM_DT_START_RECORDING,
                recording_ ? L"Stop recording" : L"Start recording");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING,  IDM_DT_PAUSE_VIDEO,        L"Pause video when minimizing");
    AppendMenuW(menu, MF_STRING,  IDM_DT_PAUSE_AUDIO,        L"Pause audio when minimizing");
    AppendMenuW(menu, MF_STRING,  IDM_DT_SEND_KEY_COMBOS,    L"Send key combinations");
    AppendMenuW(menu, MF_STRING,  IDM_DT_AUTOSCROLL,         L"Automatic scrolling");
    AppendMenuW(menu, MF_STRING,  IDM_DT_PASTE_CLIPBOARD,    L"Paste clipboard as keystrokes");
    if (scaleMenu)
        AppendMenuW(menu, MF_POPUP,
                    reinterpret_cast<UINT_PTR>(scaleMenu), L"Scale");

    RECT btnRect = {};
    SendMessageW(toolbar_, TB_GETRECT,
                 static_cast<WPARAM>(IDC_DT_MENU),
                 reinterpret_cast<LPARAM>(&btnRect));
    POINT pt = { btnRect.left, btnRect.bottom };
    ClientToScreen(toolbar_, &pt);

    TrackPopupMenu(menu,
                   TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
                   pt.x, pt.y, 0, hwnd_, nullptr);
    DestroyMenu(menu);
}

void DesktopToolbar::onDestroy()
{
}

}  // namespace aspia::client_win32
