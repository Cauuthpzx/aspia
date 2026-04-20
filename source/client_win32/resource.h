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

#ifndef CLIENT_WIN32_RESOURCE_H
#define CLIENT_WIN32_RESOURCE_H

// Icons and menu resources
#define IDI_ASPIA              101
#define IDR_MAIN_MENU          102
#define IDR_MAIN_ACCEL         103

// Dialog templates
#define IDD_AUTH               200
#define IDD_ABOUT              210
#define IDD_STATUS             220
#define IDD_DOWNLOAD           230
#define IDD_ROUTER             240
#define IDD_DESKTOP_CONFIG     250
#define IDD_RECORD_SETTINGS    260

// Main window child-control IDs
#define IDC_MAIN_TABS         1001
#define IDC_MAIN_STATUSBAR    1002

// Authorization dialog control IDs
#define IDC_AUTH_USERNAME     2001
#define IDC_AUTH_PASSWORD     2002
#define IDC_AUTH_SHOW_PASSWORD 2003
#define IDC_AUTH_ONE_TIME     2004

// About dialog control IDs
#define IDC_ABOUT_NAME        2100
#define IDC_ABOUT_VERSION     2101
#define IDC_ABOUT_COPYRIGHT   2102
#define IDC_ABOUT_INFO        2103
#define IDC_ABOUT_SITE_LINK   2104
#define IDC_ABOUT_MAIL_LINK   2105
#define IDC_ABOUT_DONATE      2106

// Status dialog control IDs
#define IDC_STATUS_MESSAGE    2200

// Download dialog control IDs
#define IDC_DOWNLOAD_STATUS   2300
#define IDC_DOWNLOAD_PROGRESS 2301

// Router dialog control IDs
#define IDC_ROUTER_ADDRESS         2401
#define IDC_ROUTER_NAME            2402
#define IDC_ROUTER_SESSION_TYPE    2403
#define IDC_ROUTER_USERNAME        2404
#define IDC_ROUTER_PASSWORD        2405
#define IDC_ROUTER_SHOW_PASSWORD   2406

// Desktop config dialog control IDs
#define IDC_DCFG_VIDEO_CODEC             2500
#define IDC_DCFG_PIXEL_FORMAT            2501
#define IDC_DCFG_COMPRESS_RATIO          2502
#define IDC_DCFG_COMPRESS_RATIO_LABEL    2503
#define IDC_DCFG_FPS                     2504
#define IDC_DCFG_FPS_LABEL               2505
#define IDC_DCFG_AUDIO                   2510
#define IDC_DCFG_CLIPBOARD               2511
#define IDC_DCFG_CURSOR_SHAPE            2512
#define IDC_DCFG_CURSOR_POSITION         2513
#define IDC_DCFG_DISABLE_EFFECTS         2514
#define IDC_DCFG_DISABLE_WALLPAPER       2515
#define IDC_DCFG_LOCK_AT_DISCONNECT      2516
#define IDC_DCFG_BLOCK_REMOTE_INPUT      2517

// Record settings dialog control IDs
#define IDC_RECORD_AUTOSTART       2600
#define IDC_RECORD_PATH_LABEL      2601
#define IDC_RECORD_PATH            2602
#define IDC_RECORD_BROWSE          2603
#define IDC_RECORD_FORMAT_LABEL    2604
#define IDC_RECORD_FORMAT          2605
#define IDC_RECORD_CODEC_LABEL     2606
#define IDC_RECORD_CODEC           2607

// Menu command IDs
#define ID_FILE_EXIT         40001
#define ID_FILE_CONNECT      40002
#define ID_HELP_ABOUT        40003

#endif  // CLIENT_WIN32_RESOURCE_H
