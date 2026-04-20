#
# Aspia Project
# Copyright (C) 2016-2026 Dmitry Chapyshev <dmitry@aspia.ru>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# Aspia Client (Win32, Qt-free).
#
# This target is the forward-looking replacement for the legacy Qt-based
# aspia_client. It links only against the Win32 API and (eventually)
# aspia_proto. It is the landing zone for the Phase 2 Qt removal.
#

list(APPEND SOURCE_CLIENT_WIN32
    app.cc
    app.h
    about_dialog.cc
    about_dialog.h
    auth_dialog.cc
    auth_dialog.h
    chat_session_adapter.cc
    chat_session_adapter.h
    chat_session_window.cc
    chat_session_window.h
    console_address_book_dialog.cc
    console_address_book_dialog.h
    console_computer_dialog.cc
    console_computer_dialog.h
    console_computer_group_dialog.cc
    console_computer_group_dialog.h
    console_fast_connect_dialog.cc
    console_fast_connect_dialog.h
    console_main_window.cc
    console_main_window.h
    console_open_address_book_dialog.cc
    console_open_address_book_dialog.h
    console_update_settings_dialog.cc
    console_update_settings_dialog.h
    desktop_config_dialog.cc
    desktop_config_dialog.h
    desktop_session_adapter.cc
    desktop_session_adapter.h
    desktop_session_window.cc
    desktop_session_window.h
    desktop_toolbar.cc
    desktop_toolbar.h
    download_dialog.cc
    download_dialog.h
    entry_point.cc
    file_panel.cc
    file_panel.h
    file_remove_dialog.cc
    file_remove_dialog.h
    file_transfer_dialog.cc
    file_transfer_dialog.h
    file_transfer_session_adapter.cc
    file_transfer_session_adapter.h
    file_transfer_session_window.cc
    file_transfer_session_window.h
    host_change_password_dialog.cc
    host_change_password_dialog.h
    host_check_password_dialog.cc
    host_check_password_dialog.h
    host_config_dialog.cc
    host_config_dialog.h
    host_connect_confirm_dialog.cc
    host_connect_confirm_dialog.h
    host_main_window.cc
    host_main_window.h
    host_notifier_window.cc
    host_notifier_window.h
    host_user_dialog.cc
    host_user_dialog.h
    hosts_tab.cc
    hosts_tab.h
    local_computer_dialog.cc
    local_computer_dialog.h
    local_group_dialog.cc
    local_group_dialog.h
    local_group_widget.cc
    local_group_widget.h
    main_window.cc
    main_window.h
    record_settings_dialog.cc
    record_settings_dialog.h
    resource.h
    router_dialog.cc
    router_dialog.h
    router_group_widget.cc
    router_group_widget.h
    router_manager_window.cc
    router_manager_window.h
    router_user_dialog.cc
    router_user_dialog.h
    router_widget.cc
    router_widget.h
    settings_dialog.cc
    settings_dialog.h
    statistics_dialog.cc
    statistics_dialog.h
    status_dialog.cc
    status_dialog.h
    sys_info_applications.cc
    sys_info_applications.h
    sys_info_connections.cc
    sys_info_connections.h
    sys_info_devices.cc
    sys_info_devices.h
    sys_info_drivers.cc
    sys_info_drivers.h
    sys_info_env_vars.cc
    sys_info_env_vars.h
    sys_info_event_logs.cc
    sys_info_event_logs.h
    sys_info_licenses.cc
    sys_info_licenses.h
    sys_info_local_user_groups.cc
    sys_info_local_user_groups.h
    sys_info_local_users.cc
    sys_info_local_users.h
    sys_info_monitors.cc
    sys_info_monitors.h
    sys_info_net_adapters.cc
    sys_info_net_adapters.h
    sys_info_net_shares.cc
    sys_info_net_shares.h
    sys_info_open_files.cc
    sys_info_open_files.h
    sys_info_power_options.cc
    sys_info_power_options.h
    sys_info_printers.cc
    sys_info_printers.h
    sys_info_processes.cc
    sys_info_processes.h
    sys_info_routes.cc
    sys_info_routes.h
    sys_info_services.cc
    sys_info_services.h
    sys_info_summary.cc
    sys_info_summary.h
    sys_info_video_adapters.cc
    sys_info_video_adapters.h
    system_info_session_adapter.cc
    system_info_session_adapter.h
    system_info_session_window.cc
    system_info_session_window.h
    task_manager_window.cc
    task_manager_window.h
    client.rc
    client.manifest)

source_group("" FILES ${SOURCE_CLIENT_WIN32})

add_executable(aspia_client_win32 ${SOURCE_CLIENT_WIN32})

set_target_properties(aspia_client_win32 PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/MANIFEST:NO"
    AUTOMOC ON)

target_link_libraries(aspia_client_win32 PRIVATE
    aspia_client_core
    aspia_proto
    aspia_base
    Qt6::Core
    comctl32
    dwmapi
    uxtheme
    version
    shlwapi
    shell32
    ole32
    uuid)
