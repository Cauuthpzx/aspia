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

#include "client_win32/chat_session_adapter.h"

#include "client/client_text_chat.h"
#include "client_win32/chat_session_window.h"
#include "proto/chat.h"

#include <QString>
#include <windows.h>

namespace aspia::client_win32 {

namespace {

// Convert a std::string (UTF-8 from protobuf) to std::wstring.
static std::wstring utf8ToWide(const std::string& s)
{
    if (s.empty())
        return {};

    int needed = MultiByteToWideChar(CP_UTF8, 0,
                                     s.data(), static_cast<int>(s.size()),
                                     nullptr, 0);
    if (needed <= 0)
        return {};

    std::wstring out(static_cast<size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0,
                        s.data(), static_cast<int>(s.size()),
                        out.data(), needed);
    return out;
}

} // namespace

// ---------------------------------------------------------------------------

ChatSessionAdapter::ChatSessionAdapter(client::ClientChat* client,
                                       ChatSessionWindow* window,
                                       QObject* parent)
    : QObject(parent),
      client_(client),
      window_(window)
{
    Q_ASSERT(client_);
    Q_ASSERT(window_);

    // Incoming chat messages from the host (may arrive on IO thread).
    connect(client_, &client::ClientChat::sig_chatMessage,
            this, &ChatSessionAdapter::onChatMessage,
            Qt::QueuedConnection);

    // Wire the Win32 window's "send" notification back to the client.
    wireChatWindowMessages();
}

ChatSessionAdapter::~ChatSessionAdapter() = default;

// ---------------------------------------------------------------------------
// Qt signal handler
// ---------------------------------------------------------------------------

void ChatSessionAdapter::onChatMessage(const proto::chat::Chat& chat)
{
    if (!chat.has_chat_message())
        return;

    const auto& msg = chat.chat_message();
    std::wstring sender = utf8ToWide(msg.source());
    std::wstring text   = utf8ToWide(msg.text());

    // A non-empty source means the message came from the remote side.
    bool incoming = !msg.source().empty();

    window_->appendMessage(sender, text, incoming);
}

// ---------------------------------------------------------------------------
// Win32 window wiring
// ---------------------------------------------------------------------------

void ChatSessionAdapter::wireChatWindowMessages()
{
    // The ChatSessionWindow posts kWmChatSend to its parent HWND when the user
    // clicks "Send" or presses Enter. The wParam is 0 and lParam is a pointer
    // to a std::wstring (owned by the window, valid for this call only).
    //
    // Since the ChatSessionWindow's parent is a top-level HWND managed outside
    // this adapter, we cannot subclass the parent here. Instead we hook into the
    // window via a Windows message subproc on the ChatSessionWindow itself to
    // intercept WM_APP+1 before it propagates, or rely on the App-level message
    // loop to call us back.  For Phase 2 we use a lightweight approach: we
    // install a WndProc subclass on the ChatSessionWindow HWND that intercepts
    // kWmChatSend and forwards the text to the Qt client via invokeMethod.
    //
    // This requires the window to be created before the adapter is constructed;
    // the caller must ensure that order.

    HWND hwnd = window_->handle();
    if (!hwnd || !IsWindow(hwnd))
        return;

    // Store 'this' as a window property so the subclassed WndProc can find the
    // adapter.
    SetProp(hwnd, L"AspiaChatAdapter", reinterpret_cast<HANDLE>(this));

    // Subclass the window to intercept kWmChatSend.
    WNDPROC originalProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(hwnd, GWLP_WNDPROC,
                         reinterpret_cast<LONG_PTR>(
                             [](HWND h, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT
                             {
                                 if (msg == ChatSessionWindow::kWmChatSend)
                                 {
                                     auto* adapter = reinterpret_cast<ChatSessionAdapter*>(
                                         GetProp(h, L"AspiaChatAdapter"));
                                     if (adapter && lp)
                                     {
                                         const std::wstring& wtext =
                                             *reinterpret_cast<const std::wstring*>(lp);

                                         // Convert wstring → QString → proto UTF-8.
                                         QString qtext = QString::fromStdWString(wtext);
                                         std::string utf8 = qtext.toStdString();

                                         proto::chat::Chat chatMsg;
                                         chatMsg.mutable_chat_message()->set_text(utf8);

                                         QMetaObject::invokeMethod(
                                             adapter->client_, "onChatMessage",
                                             Qt::QueuedConnection,
                                             Q_ARG(proto::chat::Chat, chatMsg));
                                     }
                                     return 0;
                                 }

                                 // For all other messages call the original proc.
                                 WNDPROC prev = reinterpret_cast<WNDPROC>(
                                     GetProp(h, L"AspiaChatAdapterPrevProc"));
                                 if (prev)
                                     return CallWindowProc(prev, h, msg, wp, lp);
                                 return DefWindowProc(h, msg, wp, lp);
                             })));

    // Store the original WndProc so the subclassed proc can chain calls.
    SetProp(hwnd, L"AspiaChatAdapterPrevProc",
            reinterpret_cast<HANDLE>(originalProc));
}

} // namespace aspia::client_win32
