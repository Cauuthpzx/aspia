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

#include "client_win32/chat_session_window.h"

#include "client_win32/resource.h"

#include <richedit.h>

namespace aspia::client_win32 {

namespace {

// Layout constants (in logical pixels at system DPI -- good enough for a
// first-pass Win32 port; DPI-awareness is a TODO).
constexpr int kMargin        = 6;
constexpr int kInputHeight   = 24;
constexpr int kSendWidth     = 80;
constexpr int kDefaultWidth  = 450;
constexpr int kDefaultHeight = 500;

std::wstring readEditText(HWND edit)
{
    const int len = GetWindowTextLengthW(edit);
    if (len <= 0)
        return {};
    std::wstring out(static_cast<size_t>(len), L'\0');
    GetWindowTextW(edit, out.data(), len + 1);
    return out;
}

// Apply a character format (colour, optional bold) to the *next* inserted
// text in the rich edit control, then append |text|.
void appendRichText(HWND rich,
                    const std::wstring& text,
                    COLORREF colour,
                    bool bold)
{
    // Move caret to end of current content.
    const LONG end = GetWindowTextLengthW(rich);
    SendMessageW(rich, EM_SETSEL, static_cast<WPARAM>(end),
                 static_cast<LPARAM>(end));

    CHARFORMAT2W cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR | CFM_BOLD;
    cf.dwEffects = bold ? CFE_BOLD : 0;
    cf.crTextColor = colour;
    SendMessageW(rich, EM_SETCHARFORMAT, SCF_SELECTION,
                 reinterpret_cast<LPARAM>(&cf));

    SendMessageW(rich, EM_REPLACESEL, FALSE,
                 reinterpret_cast<LPARAM>(text.c_str()));
}

}  // namespace

ChatSessionWindow::ChatSessionWindow(HINSTANCE instance, HWND parent)
    : instance_(instance), parent_(parent)
{
}

ChatSessionWindow::~ChatSessionWindow()
{
    if (font_)
        DeleteObject(font_);
    if (msftedit_)
        FreeLibrary(msftedit_);
}

bool ChatSessionWindow::create()
{
    // Msftedit.dll hosts the MSFTEDIT_CLASS (RICHEDIT50W) window class.  It
    // must be loaded before CreateWindowEx is called with that class name.
    // LoadLibraryW is safe to call repeatedly -- the OS ref-counts modules --
    // and we pair it with FreeLibrary in the destructor.  If loading fails we
    // bail out; there's no sensible fallback without pulling in a plain
    // multi-line edit (which would break the formatting API we rely on).
    if (!msftedit_)
    {
        msftedit_ = LoadLibraryW(L"Msftedit.dll");
        if (!msftedit_)
            return false;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &ChatSessionWindow::windowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return false;

    hwnd_ = CreateWindowExW(
        0, kClassName, L"Chat",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, kDefaultWidth, kDefaultHeight,
        parent_, nullptr, instance_, this);

    return hwnd_ != nullptr;
}

void ChatSessionWindow::show(int showCmd)
{
    ShowWindow(hwnd_, showCmd);
    UpdateWindow(hwnd_);
}

bool ChatSessionWindow::preTranslateMessage(MSG* msg)
{
    // Treat Enter in the input edit as "send".  This avoids the need for an
    // accelerator table (and therefore an .rc edit) while still giving the
    // user the expected chat UX.  Shift+Enter is reserved for a future
    // multi-line input mode; for now the input is single-line so Shift+Enter
    // falls through to the default handler.
    //
    // TODO(phase2): if we ever switch to a multi-line RichEdit for input,
    // replace this with a real IDR_CHAT_ACCEL accelerator table.
    if (msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN &&
        msg->hwnd == input_ &&
        (GetKeyState(VK_SHIFT) & 0x8000) == 0)
    {
        sendCurrentInput();
        return true;
    }
    return false;
}

void ChatSessionWindow::appendMessage(const std::wstring& sender,
                                      const std::wstring& text,
                                      bool incoming)
{
    if (!history_)
        return;

    // Colour scheme: incoming messages use a cool blue, outgoing a warm green;
    // both render the sender name in bold.  Plain body uses the system window
    // text colour so it stays readable under custom themes / high contrast.
    const COLORREF name_colour = incoming ? RGB(0x1F, 0x4E, 0x9A)
                                          : RGB(0x1B, 0x6A, 0x3B);
    const COLORREF body_colour = GetSysColor(COLOR_WINDOWTEXT);

    appendRichText(history_, sender + L": ", name_colour, /*bold=*/true);
    appendRichText(history_, text + L"\r\n", body_colour, /*bold=*/false);

    // Auto-scroll to the newest line.
    SendMessageW(history_, WM_VSCROLL, SB_BOTTOM, 0);
}

// static
LRESULT CALLBACK ChatSessionWindow::windowProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ChatSessionWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<ChatSessionWindow*>(cs->lpCreateParams);
        self->hwnd_ = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<ChatSessionWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(msg, wp, lp);

    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT ChatSessionWindow::handleMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
            onCreate();
            return 0;

        case WM_SIZE:
            onSize(LOWORD(lp), HIWORD(lp));
            return 0;

        case WM_COMMAND:
            onCommand(LOWORD(wp), HIWORD(wp));
            return 0;

        case WM_SETFOCUS:
            if (input_)
                SetFocus(input_);
            return 0;

        case WM_DESTROY:
            onDestroy();
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wp, lp);
    }
}

void ChatSessionWindow::onCreate()
{
    font_ = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    history_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdHistory)),
        instance_, nullptr);

    input_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdInput)),
        instance_, nullptr);

    send_ = CreateWindowExW(
        0, L"BUTTON", L"Send",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        0, 0, 0, 0, hwnd_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kIdSend)),
        instance_, nullptr);

    if (font_)
    {
        SendMessageW(history_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
        SendMessageW(input_,   WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
        SendMessageW(send_,    WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    }

    // Let the history control raise EN_LINK etc. if we ever add link support.
    SendMessageW(history_, EM_SETEVENTMASK, 0,
                 ENM_LINK | ENM_SELCHANGE);
    SendMessageW(history_, EM_AUTOURLDETECT, TRUE, 0);

    SetFocus(input_);
}

void ChatSessionWindow::onSize(int width, int height)
{
    const int inputTop = height - kMargin - kInputHeight;
    const int historyHeight = inputTop - (2 * kMargin);
    const int inputWidth = width - (3 * kMargin) - kSendWidth;

    if (history_)
    {
        SetWindowPos(history_, nullptr,
                     kMargin, kMargin,
                     width - (2 * kMargin),
                     historyHeight > 0 ? historyHeight : 0,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (input_)
    {
        SetWindowPos(input_, nullptr,
                     kMargin, inputTop,
                     inputWidth > 0 ? inputWidth : 0,
                     kInputHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (send_)
    {
        SetWindowPos(send_, nullptr,
                     width - kMargin - kSendWidth, inputTop,
                     kSendWidth, kInputHeight,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void ChatSessionWindow::onCommand(int id, int notifyCode)
{
    switch (id)
    {
        case kIdSend:
            if (notifyCode == BN_CLICKED)
                sendCurrentInput();
            break;

        default:
            break;
    }
}

void ChatSessionWindow::onDestroy()
{
}

void ChatSessionWindow::sendCurrentInput()
{
    if (!input_)
        return;

    std::wstring text = readEditText(input_);
    if (text.empty())
        return;

    // Notify the parent (session owner) so it can push the message onto the
    // wire.  The parent is expected to call appendMessage() for the local
    // echo once delivery is confirmed.
    //
    // TODO(phase2): replace this parent-posted WM_APP with a direct call into
    // the Win32-ported text_chat session controller.
    if (parent_)
    {
        SendMessageW(parent_, kWmChatSend,
                     static_cast<WPARAM>(kIdmSend),
                     reinterpret_cast<LPARAM>(&text));
    }

    SetWindowTextW(input_, L"");
    SetFocus(input_);
}

}  // namespace aspia::client_win32
