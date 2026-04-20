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
// Win32 replacement for client/ui/chat/chat_session_window.ui.
//
// Control IDs owned by this window (to be added to resource.h later, in range
// 3200..3299, reserved for the chat session window):
//
//     #define IDC_CHAT_HISTORY   3201  // read-only RICHEDIT50W, message log
//     #define IDC_CHAT_INPUT     3202  // single-line edit, outgoing message
//     #define IDC_CHAT_SEND      3203  // "Send" push button
//
// Menu / command IDs (owned; also listed here for resource.h):
//
//     #define IDM_CHAT_SEND      320   // WM_COMMAND id posted to parent when
//                                      // user clicks Send or presses Enter
//

#ifndef CLIENT_WIN32_CHAT_SESSION_WINDOW_H
#define CLIENT_WIN32_CHAT_SESSION_WINDOW_H

#include <windows.h>

#include <string>

namespace aspia::client_win32 {

class ChatSessionWindow
{
public:
    // Control IDs (range 3200..3299 reserved; mirrored in comment at top).
    static constexpr int kIdHistory = 3201;
    static constexpr int kIdInput   = 3202;
    static constexpr int kIdSend    = 3203;

    // WM_COMMAND id for Send, also posted up to parent as WM_APP notification
    // payload.
    static constexpr int kIdmSend   = 320;

    // Notification message posted to the parent window when the user triggers
    // "send".  wParam = 0, lParam = pointer to a std::wstring owned by this
    // window (valid only during the synchronous handling of the message).
    // The chat session window is NOT responsible for delivering the text over
    // the wire -- that is the parent/session layer's job.
    //
    // TODO(phase2): wire this up to the text-chat proto session once the
    // client session pipeline is ported to Win32.
    static constexpr UINT kWmChatSend = WM_APP + 1;

    ChatSessionWindow(HINSTANCE instance, HWND parent);
    ~ChatSessionWindow();

    ChatSessionWindow(const ChatSessionWindow&) = delete;
    ChatSessionWindow& operator=(const ChatSessionWindow&) = delete;

    bool create();
    void show(int showCmd);
    bool preTranslateMessage(MSG* msg);

    // Append a chat line to the history control.  |incoming| controls the
    // colour used for the sender / body.  The sender name is rendered bold.
    void appendMessage(const std::wstring& sender,
                       const std::wstring& text,
                       bool incoming);

    HWND handle() const { return hwnd_; }

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT handleMessage(UINT msg, WPARAM wp, LPARAM lp);

    void onCreate();
    void onSize(int width, int height);
    void onCommand(int id, int notifyCode);
    void onDestroy();

    // Reads the input edit, fires kWmChatSend at parent, clears input.
    void sendCurrentInput();

    HINSTANCE instance_;
    HWND parent_;
    HWND hwnd_ = nullptr;
    HWND history_ = nullptr;
    HWND input_ = nullptr;
    HWND send_ = nullptr;
    HFONT font_ = nullptr;

    // Msftedit.dll must be resident for the lifetime of the window; we load it
    // on first create() and free it in the destructor.  The module is
    // reference-counted by the OS, so multiple chat windows co-exist safely.
    HMODULE msftedit_ = nullptr;

    static constexpr wchar_t kClassName[] = L"AspiaClientWin32ChatWindow";
};

}  // namespace aspia::client_win32

#endif  // CLIENT_WIN32_CHAT_SESSION_WINDOW_H
