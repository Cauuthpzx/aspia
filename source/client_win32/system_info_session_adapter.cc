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

#include "client_win32/system_info_session_adapter.h"

#include "client/client_system_info.h"
#include "client_win32/system_info_session_window.h"
#include "client_win32/sys_info_applications.h"
#include "client_win32/sys_info_connections.h"
#include "client_win32/sys_info_devices.h"
#include "client_win32/sys_info_drivers.h"
#include "client_win32/sys_info_env_vars.h"
#include "client_win32/sys_info_event_logs.h"
#include "client_win32/sys_info_licenses.h"
#include "client_win32/sys_info_local_user_groups.h"
#include "client_win32/sys_info_local_users.h"
#include "client_win32/sys_info_monitors.h"
#include "client_win32/sys_info_net_adapters.h"
#include "client_win32/sys_info_net_shares.h"
#include "client_win32/sys_info_open_files.h"
#include "client_win32/sys_info_power_options.h"
#include "client_win32/sys_info_printers.h"
#include "client_win32/sys_info_processes.h"
#include "client_win32/sys_info_routes.h"
#include "client_win32/sys_info_services.h"
#include "client_win32/sys_info_summary.h"
#include "client_win32/sys_info_video_adapters.h"
#include "proto/system_info.h"

namespace aspia::client_win32 {

namespace {

// Category IDs — must be unique; used to route onCategorySelected events.
enum CategoryId
{
    kCatSummary        = 0,
    kCatApplications   = 1,
    kCatConnections    = 2,
    kCatDevices        = 3,
    kCatDrivers        = 4,
    kCatEnvVars        = 5,
    kCatEventLogs      = 6,
    kCatLicenses       = 7,
    kCatLocalUserGroups = 8,
    kCatLocalUsers     = 9,
    kCatMonitors       = 10,
    kCatNetAdapters    = 11,
    kCatNetShares      = 12,
    kCatOpenFiles      = 13,
    kCatPowerOptions   = 14,
    kCatPrinters       = 15,
    kCatProcesses      = 16,
    kCatRoutes         = 17,
    kCatServices       = 18,
    kCatVideoAdapters  = 19,
};

} // namespace

// ---------------------------------------------------------------------------

SystemInfoSessionAdapter::SystemInfoSessionAdapter(client::ClientSystemInfo* client,
                                                   SystemInfoSessionWindow* window,
                                                   HINSTANCE instance,
                                                   QObject* parent)
    : QObject(parent),
      client_(client),
      window_(window),
      instance_(instance)
{
    Q_ASSERT(client_);
    Q_ASSERT(window_);

    // Connect sig_systemInfo (may arrive from IO thread → queued delivery).
    connect(client_, &client::ClientSystemInfo::sig_systemInfo,
            this, &SystemInfoSessionAdapter::onSystemInfo,
            Qt::QueuedConnection);

    // Create all sub-widgets parented to the content pane of the session window.
    createWidgets(window_->contentPane());

    // Populate the category tree.
    populateCategoryTree();

    // Wire the window delegate.
    wireWindowDelegate();

    // Show the summary by default.
    if (summary_)
    {
        window_->setContentWindow(summary_->handle());
        active_widget_ = summary_->handle();
    }
}

SystemInfoSessionAdapter::~SystemInfoSessionAdapter() = default;

// ---------------------------------------------------------------------------
// Qt signal handler
// ---------------------------------------------------------------------------

void SystemInfoSessionAdapter::onSystemInfo(const proto::system_info::SystemInfo& si)
{
    // Dispatch to all sub-widgets that have a proto-aware setter.  Each
    // widget checks internally whether its sub-message is present.
    if (summary_)
        summary_->clear();       // summary uses addSection(); rebuild on each refresh

    if (apps_)
        apps_->setFromProto(si);
    if (connections_)
        connections_->setFromProto(si);
    if (devices_)
        devices_->setFromProto(si);
    if (drivers_)
        drivers_->setFromProto(si);
    if (env_vars_)
        env_vars_->setFromProto(si);
    if (event_logs_)
        event_logs_->setFromProto(si);
    if (licenses_)
        licenses_->setFromProto(si);
    if (local_user_groups_)
        local_user_groups_->setFromProto(si);
    if (local_users_)
        local_users_->setFromProto(si);
    if (monitors_)
        monitors_->setFromProto(si);
    if (net_adapters_)
        net_adapters_->setFromProto(si);
    if (net_shares_)
        net_shares_->setFromProto(si);
    if (open_files_)
        open_files_->setFromProto(si);
    if (power_options_)
        power_options_->setFromProto(si);
    if (printers_)
        printers_->setFromProto(si);
    if (processes_)
        processes_->setFromProto(si);
    if (routes_)
        routes_->setFromProto(si);
    if (services_)
        services_->setFromProto(si);
    if (video_adapters_)
        video_adapters_->setFromProto(si);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void SystemInfoSessionAdapter::createWidgets(HWND parent)
{
    summary_           = std::make_unique<SysInfoSummary>();
    apps_              = std::make_unique<SysInfoApplications>(instance_);
    connections_       = std::make_unique<SysInfoConnections>(instance_);
    devices_           = std::make_unique<SysInfoDevices>(instance_);
    drivers_           = std::make_unique<SysInfoDrivers>(instance_);
    env_vars_          = std::make_unique<SysInfoEnvVars>(instance_);
    event_logs_        = std::make_unique<SysInfoEventLogs>(instance_);
    licenses_          = std::make_unique<SysInfoLicenses>(instance_);
    local_user_groups_ = std::make_unique<SysInfoLocalUserGroups>(instance_);
    local_users_       = std::make_unique<SysInfoLocalUsers>(instance_);
    monitors_          = std::make_unique<SysInfoMonitors>(instance_);
    net_adapters_      = std::make_unique<SysInfoNetAdapters>(instance_);
    net_shares_        = std::make_unique<SysInfoNetShares>(instance_);
    open_files_        = std::make_unique<SysInfoOpenFiles>(instance_);
    power_options_     = std::make_unique<SysInfoPowerOptions>();
    printers_          = std::make_unique<SysInfoPrinters>(instance_);
    processes_         = std::make_unique<SysInfoProcesses>();
    routes_            = std::make_unique<SysInfoRoutes>(instance_);
    services_          = std::make_unique<SysInfoServices>(instance_);
    video_adapters_    = std::make_unique<SysInfoVideoAdapters>(instance_);

    summary_->create(parent);
    apps_->create(parent);
    connections_->create(parent);
    devices_->create(parent);
    drivers_->create(parent);
    env_vars_->create(parent);
    event_logs_->create(parent);
    licenses_->create(parent);
    local_user_groups_->create(parent);
    local_users_->create(parent);
    monitors_->create(parent);
    net_adapters_->create(parent);
    net_shares_->create(parent);
    open_files_->create(parent);
    power_options_->create(parent);
    printers_->create(parent);
    processes_->create(parent);
    routes_->create(parent);
    services_->create(parent);
    video_adapters_->create(parent);
}

void SystemInfoSessionAdapter::populateCategoryTree()
{
    window_->addCategory(TVI_ROOT, L"Summary",          kCatSummary);
    window_->addCategory(TVI_ROOT, L"Applications",     kCatApplications);
    window_->addCategory(TVI_ROOT, L"Connections",      kCatConnections);
    window_->addCategory(TVI_ROOT, L"Devices",          kCatDevices);
    window_->addCategory(TVI_ROOT, L"Drivers",          kCatDrivers);
    window_->addCategory(TVI_ROOT, L"Environment Vars", kCatEnvVars);
    window_->addCategory(TVI_ROOT, L"Event Logs",       kCatEventLogs);
    window_->addCategory(TVI_ROOT, L"Licenses",         kCatLicenses);
    window_->addCategory(TVI_ROOT, L"User Groups",      kCatLocalUserGroups);
    window_->addCategory(TVI_ROOT, L"Local Users",      kCatLocalUsers);
    window_->addCategory(TVI_ROOT, L"Monitors",         kCatMonitors);
    window_->addCategory(TVI_ROOT, L"Network Adapters", kCatNetAdapters);
    window_->addCategory(TVI_ROOT, L"Network Shares",   kCatNetShares);
    window_->addCategory(TVI_ROOT, L"Open Files",       kCatOpenFiles);
    window_->addCategory(TVI_ROOT, L"Power Options",    kCatPowerOptions);
    window_->addCategory(TVI_ROOT, L"Printers",         kCatPrinters);
    window_->addCategory(TVI_ROOT, L"Processes",        kCatProcesses);
    window_->addCategory(TVI_ROOT, L"Routes",           kCatRoutes);
    window_->addCategory(TVI_ROOT, L"Services",         kCatServices);
    window_->addCategory(TVI_ROOT, L"Video Adapters",   kCatVideoAdapters);
}

void SystemInfoSessionAdapter::wireWindowDelegate()
{
    SystemInfoSessionWindow::Delegate d;

    // Refresh: send a full system-info request to the host.
    // An empty category string requests all categories.
    d.onRefresh = [this]()
    {
        proto::system_info::SystemInfoRequest request;
        // Empty category = request everything.

        QMetaObject::invokeMethod(client_, "onSystemInfoRequest",
                                  Qt::QueuedConnection,
                                  Q_ARG(proto::system_info::SystemInfoRequest, request));
    };

    // Save / Print — Phase 3.
    d.onSave  = []() {};
    d.onPrint = []() {};

    // Context-menu copy operations — Phase 3.
    d.onCopyRow   = []() {};
    d.onCopyName  = []() {};
    d.onCopyValue = []() {};

    // Category selected: switch the visible child window.
    d.onCategorySelected = [this](int categoryId)
    {
        HWND next = nullptr;

        switch (categoryId)
        {
            case kCatSummary:        next = summary_          ? summary_->handle()          : nullptr; break;
            case kCatApplications:   next = apps_             ? apps_->handle()             : nullptr; break;
            case kCatConnections:    next = connections_       ? connections_->handle()      : nullptr; break;
            case kCatDevices:        next = devices_           ? devices_->handle()          : nullptr; break;
            case kCatDrivers:        next = drivers_           ? drivers_->handle()          : nullptr; break;
            case kCatEnvVars:        next = env_vars_          ? env_vars_->handle()         : nullptr; break;
            case kCatEventLogs:      next = event_logs_        ? event_logs_->handle()       : nullptr; break;
            case kCatLicenses:       next = licenses_          ? licenses_->handle()         : nullptr; break;
            case kCatLocalUserGroups:next = local_user_groups_ ? local_user_groups_->handle(): nullptr; break;
            case kCatLocalUsers:     next = local_users_       ? local_users_->handle()      : nullptr; break;
            case kCatMonitors:       next = monitors_          ? monitors_->handle()         : nullptr; break;
            case kCatNetAdapters:    next = net_adapters_      ? net_adapters_->handle()     : nullptr; break;
            case kCatNetShares:      next = net_shares_        ? net_shares_->handle()       : nullptr; break;
            case kCatOpenFiles:      next = open_files_        ? open_files_->handle()       : nullptr; break;
            case kCatPowerOptions:   next = power_options_     ? power_options_->handle()    : nullptr; break;
            case kCatPrinters:       next = printers_          ? printers_->handle()         : nullptr; break;
            case kCatProcesses:      next = processes_         ? processes_->handle()        : nullptr; break;
            case kCatRoutes:         next = routes_            ? routes_->handle()           : nullptr; break;
            case kCatServices:       next = services_          ? services_->handle()         : nullptr; break;
            case kCatVideoAdapters:  next = video_adapters_    ? video_adapters_->handle()   : nullptr; break;
            default:                 break;
        }

        if (next && next != active_widget_)
        {
            window_->setContentWindow(next);
            active_widget_ = next;
        }
    };

    window_->setDelegate(std::move(d));
}

} // namespace aspia::client_win32
