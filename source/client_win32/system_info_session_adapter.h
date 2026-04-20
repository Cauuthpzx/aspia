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
// Phase-2 adapter: bridges client::ClientSystemInfo (Qt signals/slots) to the
// Win32 SystemInfoSessionWindow and the 20 sys_info sub-widgets (delegate
// callbacks).
//

#ifndef CLIENT_WIN32_SYSTEM_INFO_SESSION_ADAPTER_H
#define CLIENT_WIN32_SYSTEM_INFO_SESSION_ADAPTER_H

#include <QObject>

#include <memory>
#include <windows.h>

namespace client { class ClientSystemInfo; }
namespace proto::system_info { class SystemInfo; }

namespace aspia::client_win32 {

class SystemInfoSessionWindow;
class SysInfoApplications;
class SysInfoConnections;
class SysInfoDevices;
class SysInfoDrivers;
class SysInfoEnvVars;
class SysInfoEventLogs;
class SysInfoLicenses;
class SysInfoLocalUserGroups;
class SysInfoLocalUsers;
class SysInfoMonitors;
class SysInfoNetAdapters;
class SysInfoNetShares;
class SysInfoOpenFiles;
class SysInfoPowerOptions;
class SysInfoPrinters;
class SysInfoProcesses;
class SysInfoRoutes;
class SysInfoServices;
class SysInfoSummary;
class SysInfoVideoAdapters;

class SystemInfoSessionAdapter : public QObject
{
    Q_OBJECT

public:
    SystemInfoSessionAdapter(client::ClientSystemInfo* client,
                             SystemInfoSessionWindow* window,
                             HINSTANCE instance,
                             QObject* parent = nullptr);
    ~SystemInfoSessionAdapter() override;

private slots:
    void onSystemInfo(const proto::system_info::SystemInfo& si);

private:
    void createWidgets(HWND parent);
    void populateCategoryTree();
    void wireWindowDelegate();

    client::ClientSystemInfo* client_;
    SystemInfoSessionWindow* window_;
    HINSTANCE instance_;

    // Owned sys_info sub-widgets.
    std::unique_ptr<SysInfoSummary>         summary_;
    std::unique_ptr<SysInfoApplications>    apps_;
    std::unique_ptr<SysInfoConnections>     connections_;
    std::unique_ptr<SysInfoDevices>         devices_;
    std::unique_ptr<SysInfoDrivers>         drivers_;
    std::unique_ptr<SysInfoEnvVars>         env_vars_;
    std::unique_ptr<SysInfoEventLogs>       event_logs_;
    std::unique_ptr<SysInfoLicenses>        licenses_;
    std::unique_ptr<SysInfoLocalUserGroups> local_user_groups_;
    std::unique_ptr<SysInfoLocalUsers>      local_users_;
    std::unique_ptr<SysInfoMonitors>        monitors_;
    std::unique_ptr<SysInfoNetAdapters>     net_adapters_;
    std::unique_ptr<SysInfoNetShares>       net_shares_;
    std::unique_ptr<SysInfoOpenFiles>       open_files_;
    std::unique_ptr<SysInfoPowerOptions>    power_options_;
    std::unique_ptr<SysInfoPrinters>        printers_;
    std::unique_ptr<SysInfoProcesses>       processes_;
    std::unique_ptr<SysInfoRoutes>          routes_;
    std::unique_ptr<SysInfoServices>        services_;
    std::unique_ptr<SysInfoVideoAdapters>   video_adapters_;

    // Currently visible widget (nullptr = none).
    HWND active_widget_ = nullptr;

    Q_DISABLE_COPY_MOVE(SystemInfoSessionAdapter)
};

} // namespace aspia::client_win32

#endif // CLIENT_WIN32_SYSTEM_INFO_SESSION_ADAPTER_H
