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
#define IDD_SETTINGS           270
#define IDD_STATISTICS         280
#define IDD_FILE_TRANSFER      300
#define IDD_FILE_REMOVE        310
#define IDD_LOCAL_COMPUTER     330
#define IDD_ROUTER_USER        340
#define IDD_LOCAL_GROUP        350

// Accelerator / message IDs for top-level windows (no dialog template)
#define IDR_TASKMGR_ACCEL      290
#define IDM_CHAT_SEND          320
#define IDR_ROUTER_MGR_ACCEL   400

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

// Settings dialog control IDs
#define IDC_SETTINGS_LANGUAGE              2700
#define IDC_SETTINGS_THEME                 2701
#define IDC_SETTINGS_DISPLAY_NAME          2702
#define IDC_SETTINGS_AUDIO                 2710
#define IDC_SETTINGS_CLIPBOARD             2711
#define IDC_SETTINGS_CURSOR_SHAPE          2712
#define IDC_SETTINGS_CURSOR_POSITION       2713
#define IDC_SETTINGS_DISABLE_EFFECTS       2720
#define IDC_SETTINGS_DISABLE_WALLPAPER     2721
#define IDC_SETTINGS_LOCK_AT_DISCONNECT    2730
#define IDC_SETTINGS_BLOCK_REMOTE_INPUT    2731
#define IDC_SETTINGS_CHECK_UPDATES         2740
#define IDC_SETTINGS_CUSTOM_UPDATE_SERVER  2741
#define IDC_SETTINGS_UPDATE_SERVER         2742
#define IDC_SETTINGS_CHECK_FOR_UPDATES_NOW 2743
#define IDC_SETTINGS_LOG_ENABLED           2750
#define IDC_SETTINGS_LOG_LEVEL             2751
#define IDC_SETTINGS_LOG_PATH              2752
#define IDC_SETTINGS_LOG_BROWSE            2753

// Statistics dialog control IDs
#define IDC_STATS_DURATION_VALUE            2801
#define IDC_STATS_TCP_TOTAL_VALUE           2802
#define IDC_STATS_TCP_SPEED_VALUE           2803
#define IDC_STATS_UDP_TOTAL_VALUE           2804
#define IDC_STATS_UDP_SPEED_VALUE           2805
#define IDC_STATS_VIDEO_PACKETS_VALUE       2806
#define IDC_STATS_VIDEO_PAUSE_RESUME_VALUE  2807
#define IDC_STATS_VIDEO_PACKET_MIN_MAX_AVG  2808
#define IDC_STATS_AUDIO_PACKETS_VALUE       2809
#define IDC_STATS_AUDIO_PAUSE_RESUME_VALUE  2810
#define IDC_STATS_AUDIO_PACKET_MIN_MAX_AVG  2811
#define IDC_STATS_VIDEO_CAPTURER_ENCODER    2812
#define IDC_STATS_FPS_VALUE                 2813
#define IDC_STATS_MOUSE_SEND_DROP_VALUE     2814
#define IDC_STATS_KEY_SEND_VALUE            2815
#define IDC_STATS_TEXT_SEND_VALUE           2816
#define IDC_STATS_CLIPBOARD_READ_SEND_VALUE 2817
#define IDC_STATS_CURSOR_SHAPE_COUNT_CACHE  2818
#define IDC_STATS_CURSOR_SHAPE_CACHE_SIZE   2819
#define IDC_STATS_CURSOR_POS_COUNT_VALUE    2820

// Task manager window control IDs
#define IDC_TASKMGR_TABS          2900
#define IDC_TASKMGR_LIST_PROCESS  2901
#define IDC_TASKMGR_LIST_SERVICE  2902
#define IDC_TASKMGR_LIST_USERS    2903
#define IDC_TASKMGR_LIST_PERF     2904
#define IDC_TASKMGR_STATUSBAR     2905
#define IDC_TASKMGR_END_TASK      2910
#define IDC_TASKMGR_DISCONNECT    2911
#define IDC_TASKMGR_LOGOFF        2912

// File transfer dialog control IDs
#define IDC_FILE_TRANSFER_CURRENT_ITEM      3000
#define IDC_FILE_TRANSFER_SPEED             3001
#define IDC_FILE_TRANSFER_OVERALL_LABEL     3002
#define IDC_FILE_TRANSFER_ITEM_LABEL        3003
#define IDC_FILE_TRANSFER_PROGRESS_OVERALL  3004
#define IDC_FILE_TRANSFER_PROGRESS_ITEM     3005
#define IDC_FILE_TRANSFER_STATUS            3006

// File remove dialog control IDs
#define IDC_FILE_REMOVE_CURRENT_ITEM  3100
#define IDC_FILE_REMOVE_PROGRESS      3101

// Chat session window control IDs
#define IDC_CHAT_HISTORY   3201
#define IDC_CHAT_INPUT     3202
#define IDC_CHAT_SEND      3203

// Local computer dialog control IDs
#define IDC_LOCAL_COMPUTER_GROUP          3301
#define IDC_LOCAL_COMPUTER_NAME           3302
#define IDC_LOCAL_COMPUTER_ADDRESS        3303
#define IDC_LOCAL_COMPUTER_USERNAME       3304
#define IDC_LOCAL_COMPUTER_PASSWORD       3305
#define IDC_LOCAL_COMPUTER_SHOW_PASSWORD  3306
#define IDC_LOCAL_COMPUTER_COMMENT        3307

// Local group dialog control IDs
#define IDC_LOCAL_GROUP_PARENT    3501
#define IDC_LOCAL_GROUP_NAME      3502
#define IDC_LOCAL_GROUP_COMMENT   3503

// Router user dialog control IDs
#define IDC_ROUTER_USER_USERNAME                 3400
#define IDC_ROUTER_USER_PASSWORD                 3401
#define IDC_ROUTER_USER_PASSWORD_RETRY           3402
#define IDC_ROUTER_USER_SHOW_PASSWORD            3403
#define IDC_ROUTER_USER_DISABLE                  3404
#define IDC_ROUTER_USER_SESSIONS_GROUP           3405
#define IDC_ROUTER_USER_SESSION_DESKTOP_MANAGE   3410
#define IDC_ROUTER_USER_SESSION_DESKTOP_VIEW     3411
#define IDC_ROUTER_USER_SESSION_FILE_TRANSFER    3412
#define IDC_ROUTER_USER_SESSION_SYSTEM_INFO      3413
#define IDC_ROUTER_USER_SESSION_TEXT_CHAT        3414
#define IDC_ROUTER_USER_SESSION_PORT_FORWARDING  3415

// Router manager window control IDs (top-level window)
#define IDC_ROUTER_MGR_TABS          4000
#define IDC_ROUTER_MGR_LIST_USERS    4001
#define IDC_ROUTER_MGR_LIST_HOSTS    4002
#define IDC_ROUTER_MGR_LIST_ACTIVE   4003
#define IDC_ROUTER_MGR_STATUSBAR     4004
#define IDC_ROUTER_MGR_TOOLBAR       4005
#define IDC_ROUTER_MGR_ADD           4010
#define IDC_ROUTER_MGR_EDIT          4011
#define IDC_ROUTER_MGR_REMOVE        4012
#define IDC_ROUTER_MGR_REFRESH       4013
#define IDC_ROUTER_MGR_DISCONNECT    4014
#define IDC_ROUTER_MGR_REFRESH_ACT   4015
#define IDC_ROUTER_MGR_CLOSE         4016

// File panel (dual-pane file manager) control IDs
#define IDC_FILE_PANEL_HEADER       4100
#define IDC_FILE_PANEL_DRIVE        4101
#define IDC_FILE_PANEL_ADDRESS      4102
#define IDC_FILE_PANEL_TOOLBAR      4103
#define IDC_FILE_PANEL_LIST         4104
#define IDC_FILE_PANEL_STATUS       4105
#define IDC_FILE_PANEL_TB_UP        4110
#define IDC_FILE_PANEL_TB_REFRESH   4111
#define IDC_FILE_PANEL_TB_HOME      4112
#define IDC_FILE_PANEL_TB_NEWFOLDER 4113
#define IDC_FILE_PANEL_TB_DELETE    4114
#define IDC_FILE_PANEL_TB_SEND      4115
#define IDC_FILE_PANEL_TB_UPLOAD    4116
#define IDC_FILE_PANEL_TB_DOWNLOAD  4117

// File transfer session window control IDs
#define IDC_FT_SESSION_LOCAL_PANEL   4200
#define IDC_FT_SESSION_REMOTE_PANEL  4201
#define IDC_FT_SESSION_STATUSBAR     4202

// Hosts tab control IDs
#define IDC_HOSTS_SEARCH    4300
#define IDC_HOSTS_TREE      4301
#define IDC_HOSTS_LIST      4302

// Sys-info child widgets list control IDs
#define IDC_SYSINFO_PROCESSES_LIST          4400
#define IDC_SYSINFO_SUMMARY_TREE            4500
#define IDC_SYSINFO_APPS_LIST               4600
#define IDC_SYSINFO_SERVICES_LIST           4700
#define IDC_SYSINFO_CONNECTIONS_LIST        4800
#define IDC_SYSINFO_DEVICES_LIST            4900
#define IDC_SYSINFO_DRIVERS_LIST            5000
#define IDC_SYSINFO_EVENT_LOGS_LIST         5100
#define IDC_SYSINFO_EVENT_LOGS_COMBO        5101
#define IDC_SYSINFO_LICENSES_LIST           5200
#define IDC_SYSINFO_LOCAL_USERS_LIST        5300
#define IDC_SYSINFO_LOCAL_USER_GROUPS_LIST  5400
#define IDC_SYSINFO_MONITORS_LIST           5500
#define IDC_SYSINFO_NET_ADAPTERS_LIST       5600
#define IDC_SYSINFO_NET_SHARES_LIST         5700
#define IDC_SYSINFO_OPEN_FILES_LIST         5800
#define IDC_SYSINFO_PRINTERS_LIST           5900
#define IDC_SYSINFO_ROUTES_LIST             6000
#define IDC_SYSINFO_VIDEO_ADAPTERS_LIST     6100
#define IDC_SYSINFO_POWER_OPTIONS_TREE      6200

// Menu command IDs
#define ID_FILE_EXIT         40001
#define ID_FILE_CONNECT      40002
#define ID_HELP_ABOUT        40003

#endif  // CLIENT_WIN32_RESOURCE_H
