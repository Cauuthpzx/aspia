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

#ifndef CLIENT_WIN32_DESKTOP_SESSION_ADAPTER_H
#define CLIENT_WIN32_DESKTOP_SESSION_ADAPTER_H

#include <QObject>

#include <memory>

namespace base { class Frame; }
namespace client { class ClientDesktop; }
namespace proto::control { class Capabilities; class SessionList; }
namespace proto::screen { class ScreenList; }
namespace proto::cursor { class Position; }

namespace aspia::client_win32 {

class DesktopSessionWindow;

class DesktopSessionAdapter : public QObject
{
    Q_OBJECT

public:
    DesktopSessionAdapter(client::ClientDesktop* client,
                          DesktopSessionWindow* window,
                          QObject* parent = nullptr);
    ~DesktopSessionAdapter() override;

private slots:
    void onCapabilities(const proto::control::Capabilities& caps);
    void onScreenListChanged(const proto::screen::ScreenList& list);
    void onFrameChanged(const QSize& size, std::shared_ptr<base::Frame> frame);
    void onDrawFrame();
    void onMetrics(const client::ClientDesktop::Metrics& metrics);
    void onSessionListChanged(const proto::control::SessionList& sessions);

private:
    void wireWindowDelegate();

    client::ClientDesktop* client_;
    DesktopSessionWindow* window_;
    // Keeps the frame buffer alive while Win32 is painting it.
    std::shared_ptr<base::Frame> last_frame_;

    Q_DISABLE_COPY_MOVE(DesktopSessionAdapter)
};

} // namespace aspia::client_win32

#endif // CLIENT_WIN32_DESKTOP_SESSION_ADAPTER_H
