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
// Phase-2 adapter: bridges client::ClientTextChat (Qt signals/slots) to the
// Win32 ChatSessionWindow (kWmChatSend notification + appendMessage).
//

#ifndef CLIENT_WIN32_CHAT_SESSION_ADAPTER_H
#define CLIENT_WIN32_CHAT_SESSION_ADAPTER_H

#include <QObject>

namespace proto::chat { class Chat; }
namespace client { class ClientTextChat; }

namespace aspia::client_win32 {

class ChatSessionWindow;

class ChatSessionAdapter : public QObject
{
    Q_OBJECT

public:
    ChatSessionAdapter(client::ClientTextChat* client,
                       ChatSessionWindow* window,
                       QObject* parent = nullptr);
    ~ChatSessionAdapter() override;

private slots:
    void onChatMessage(const proto::chat::Chat& chat);

private:
    void wireChatWindowMessages();

    client::ClientTextChat* client_;
    ChatSessionWindow* window_;

    Q_DISABLE_COPY_MOVE(ChatSessionAdapter)
};

} // namespace aspia::client_win32

#endif // CLIENT_WIN32_CHAT_SESSION_ADAPTER_H
