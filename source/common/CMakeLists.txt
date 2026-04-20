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

list(APPEND SOURCE_COMMON
    clipboard.cc
    clipboard.h
    clipboard_file_transfer.cc
    clipboard_file_transfer.h
    clipboard_monitor.cc
    clipboard_monitor.h
    clipboard_win.cc
    clipboard_win.h
    desktop_session_constants.cc
    desktop_session_constants.h
    file_depacketizer.cc
    file_depacketizer.h
    file_packet.h
    file_packetizer.cc
    file_packetizer.h
    file_platform_util.h
    file_platform_util_win.cc
    file_task.cc
    file_task.h
    file_task_factory.cc
    file_task_factory.h
    file_worker.cc
    file_worker.h
    http_file_downloader.cc
    http_file_downloader.h
    keycode_converter.cc
    keycode_converter.h
    system_info_constants.cc
    system_info_constants.h
    update_checker.cc
    update_checker.h
    update_info.cc
    update_info.h)

list(APPEND SOURCE_COMMON_UI
    ui/about_dialog.cc
    ui/about_dialog.h
    ui/about_dialog.ui
    ui/chat_status_message.cc
    ui/chat_status_message.h
    ui/chat_status_message.ui
    ui/chat_incoming_message.cc
    ui/chat_incoming_message.h
    ui/chat_incoming_message.ui
    ui/chat_message.h
    ui/chat_outgoing_message.cc
    ui/chat_outgoing_message.h
    ui/chat_outgoing_message.ui
    ui/chat_widget.cc
    ui/chat_widget.h
    ui/chat_widget.ui
    ui/dialog_button_box.cc
    ui/dialog_button_box.h
    ui/download_dialog.cc
    ui/download_dialog.h
    ui/download_dialog.ui
    ui/language_action.h
    ui/msg_box.cc
    ui/msg_box.h
    ui/session_type.cc
    ui/session_type.h
    ui/status_dialog.cc
    ui/status_dialog.h
    ui/status_dialog.ui
    ui/taskbar_button.cc
    ui/taskbar_button.h
    ui/taskbar_progress.cc
    ui/taskbar_progress.h
    ui/update_dialog.cc
    ui/update_dialog.h
    ui/update_dialog.ui)

list(APPEND SOURCE_COMMON_WIN
    win/file_object.cc
    win/file_object.h
    win/file_stream.cc
    win/file_stream.h
    win/format_enumerator.cc
    win/format_enumerator.h)

list(APPEND SOURCE_COMMON_RESOURCES resources/common.qrc)

source_group("" FILES ${SOURCE_COMMON})
source_group(ui FILES ${SOURCE_COMMON_UI})
source_group(resources FILES ${SOURCE_COMMON_RESOURCES})
source_group(win FILES ${SOURCE_COMMON_WIN})

add_library(aspia_common STATIC
    ${SOURCE_COMMON}
    ${SOURCE_COMMON_UI}
    ${SOURCE_COMMON_WIN}
    ${SOURCE_COMMON_RESOURCES})
target_link_libraries(aspia_common PRIVATE
    aspia_base
    aspia_proto
    ${QT_COMMON_LIBS}
    ${QT_PLATFORM_LIBS}
    ${THIRD_PARTY_LIBS})
set_property(TARGET aspia_common PROPERTY AUTOMOC ON)
set_property(TARGET aspia_common PROPERTY AUTOUIC ON)
set_property(TARGET aspia_common PROPERTY AUTORCC ON)
