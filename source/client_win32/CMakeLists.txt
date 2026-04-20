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
    chat_session_window.cc
    chat_session_window.h
    desktop_config_dialog.cc
    desktop_config_dialog.h
    download_dialog.cc
    download_dialog.h
    entry_point.cc
    file_remove_dialog.cc
    file_remove_dialog.h
    file_transfer_dialog.cc
    file_transfer_dialog.h
    local_computer_dialog.cc
    local_computer_dialog.h
    main_window.cc
    main_window.h
    record_settings_dialog.cc
    record_settings_dialog.h
    resource.h
    router_dialog.cc
    router_dialog.h
    router_user_dialog.cc
    router_user_dialog.h
    settings_dialog.cc
    settings_dialog.h
    statistics_dialog.cc
    statistics_dialog.h
    status_dialog.cc
    status_dialog.h
    task_manager_window.cc
    task_manager_window.h
    client.rc
    client.manifest)

source_group("" FILES ${SOURCE_CLIENT_WIN32})

add_executable(aspia_client_win32 ${SOURCE_CLIENT_WIN32})

set_target_properties(aspia_client_win32 PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/MANIFEST:NO")

target_link_libraries(aspia_client_win32 PRIVATE
    comctl32
    dwmapi
    uxtheme
    version
    shlwapi
    shell32
    ole32
    uuid)
